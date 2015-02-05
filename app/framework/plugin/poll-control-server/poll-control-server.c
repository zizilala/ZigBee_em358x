// *******************************************************************
// * poll-control-server.c
// *
// *
// * Copyright 2013 Silicon Laboratories, Inc.                              *80*
// *******************************************************************

#include "app/framework/include/af.h"

// This plugin does not synchronize attributes between endpoints and does not
// handle multi-network issues with regard to polling.  Because of this, it is
// limited to exactly one endpoint that implements the Poll Control cluster
// server.
#if EMBER_AF_POLL_CONTROL_CLUSTER_SERVER_ENDPOINT_COUNT != 1
  #error "The Poll Control Server plugin only supports one endpoint."
#endif

// The built-in cluster tick has hooks into the polling code and is therefore
// used to control both the temporary fast poll mode while waiting for
// CheckInResponse commands and the actual fast poll mode that follows one or
// more positive CheckInResponse commands.  The endpoint event is used to send
// the periodic CheckIn commands.
//
// When it is time to check in, a new fast poll period begins with the
// selection of clients.  The clients are determined by scanning the binding
// table for nodes bound to the local endpoint for the Poll Control server.
// A CheckIn command is sent to each client, up to the limit set in the plugin
// options.  After the CheckIn commands are sent, the plugin enters a temporary
// fast poll mode until either all clients send a CheckInResponse command or
// the check-in timeout expires.  If one or more clients requests fast polling,
// the plugin continues fast polling for the maximum requested duration.  If
// FastPollStop commands are received from any clients, the fast poll duration
// is adjusted so that it reflects the maximum duration requested by all active
// clients.  Once the requested duration for all clients is satisfied, fast
// polling ends.
//
// Note that if a required check in happens to coincide with an existing fast
// poll period, the current fast poll period is terminated, all existing
// clients are forgetten, and a new fast poll period begins with the selection
// of new clients and the sending of new CheckIn commands.

extern EmberEventControl emberAfPluginPollControlServerCheckInEndpointEventControls[];

typedef struct {
  int8u bindingIndex;
  int16u fastPollTimeoutQs;
} Client;
static Client clients[EMBER_AF_PLUGIN_POLL_CONTROL_SERVER_MAX_CLIENTS];

enum {
  INITIAL = 0,
  WAITING = 1,
  POLLING = 2,
};
static int8u state = INITIAL;
static int32u fastPollStartTimeMs;

// The timeout option is in quarter seconds, but we use it in milliseconds.
#define CHECK_IN_TIMEOUT_DURATION_MS                             \
  (EMBER_AF_PLUGIN_POLL_CONTROL_SERVER_CHECK_IN_RESPONSE_TIMEOUT \
   * MILLISECOND_TICKS_PER_QUARTERSECOND)
#define NULL_INDEX 0xFF

static EmberAfStatus readServerAttribute(int8u endpoint,
                                         EmberAfAttributeId attributeId,
                                         PGM_P name,
                                         int8u *data,
                                         int8u size)
{
  EmberAfStatus status = emberAfReadServerAttribute(endpoint,
                                                    ZCL_POLL_CONTROL_CLUSTER_ID,
                                                    attributeId,
                                                    data,
                                                    size);
  if (status != EMBER_ZCL_STATUS_SUCCESS) {
    emberAfPollControlClusterPrintln("ERR: %ping %p 0x%x", "read", name, status);
  }
  return status;
}

static EmberAfStatus writeServerAttribute(int8u endpoint,
                                          EmberAfAttributeId attributeId,
                                          PGM_P name,
                                          int8u *data,
                                          EmberAfAttributeType type)
{
  EmberAfStatus status = emberAfWriteServerAttribute(endpoint,
                                                     ZCL_POLL_CONTROL_CLUSTER_ID,
                                                     attributeId,
                                                     data,
                                                     type);
  if (status != EMBER_ZCL_STATUS_SUCCESS) {
    emberAfPollControlClusterPrintln("ERR: %ping %p 0x%x", "writ", name, status);
  }
  return status;
}

static EmberStatus scheduleServerTick(int8u endpoint, int32u delayMs)
{
  return emberAfScheduleServerTickExtended(endpoint,
                                           ZCL_POLL_CONTROL_CLUSTER_ID,
                                           delayMs,
                                           EMBER_AF_SHORT_POLL,
                                           EMBER_AF_OK_TO_SLEEP);
}

static EmberStatus deactivateServerTick(int8u endpoint)
{
  return emberAfDeactivateServerTick(endpoint, ZCL_POLL_CONTROL_CLUSTER_ID);
}

static void scheduleCheckIn(int8u endpoint)
{
  EmberAfStatus status;
  int32u checkInIntervalQs;
  status = readServerAttribute(endpoint,
                               ZCL_CHECK_IN_INTERVAL_ATTRIBUTE_ID,
                               "check in interval",
                               (int8u *)&checkInIntervalQs,
                               sizeof(checkInIntervalQs));
  if (status == EMBER_ZCL_STATUS_SUCCESS && checkInIntervalQs != 0) {
    emberAfEndpointEventControlSetDelay(emberAfPluginPollControlServerCheckInEndpointEventControls,
                                        endpoint,
                                        (checkInIntervalQs
                                         * MILLISECOND_TICKS_PER_QUARTERSECOND));
  } else {
    emberAfEndpointEventControlSetInactive(emberAfPluginPollControlServerCheckInEndpointEventControls,
                                           endpoint);
  }
}

int8u findClientIndex(void)
{
  EmberBindingTableEntry incomingBinding;
  int8u incomingBindingIndex = emberAfGetBindingIndex();
  if (emberGetBinding(incomingBindingIndex, &incomingBinding)
      == EMBER_SUCCESS) {
    int8u clientIndex;
    for (clientIndex = 0;
         clientIndex < EMBER_AF_PLUGIN_POLL_CONTROL_SERVER_MAX_CLIENTS;
         clientIndex++) {
      EmberBindingTableEntry clientBinding;
      if ((emberGetBinding(clients[clientIndex].bindingIndex, &clientBinding)
           == EMBER_SUCCESS)
          && incomingBinding.type == clientBinding.type
          && incomingBinding.local == clientBinding.local
          && incomingBinding.remote == clientBinding.remote
          && (MEMCOMPARE(incomingBinding.identifier,
                         clientBinding.identifier,
                         EUI64_SIZE)
              == 0)) {
        return clientIndex;
      }
    }
  }
  return NULL_INDEX;
}

static boolean pendingCheckInResponses(void)
{
  int8u i;
  for (i = 0; i < EMBER_AF_PLUGIN_POLL_CONTROL_SERVER_MAX_CLIENTS; i++) {
    if (clients[i].bindingIndex != NULL_INDEX
        && clients[i].fastPollTimeoutQs == 0) {
      return TRUE;
    }
  }
  return FALSE;
}

static boolean outstandingFastPollRequests(int8u endpoint)
{
  int32u currentTimeMs = halCommonGetInt32uMillisecondTick();
  int32u elapsedFastPollTimeMs = elapsedTimeInt32u(fastPollStartTimeMs,
                                                   currentTimeMs);
  int16u fastPollTimeoutQs = 0;
  int8u i;
  for (i = 0; i < EMBER_AF_PLUGIN_POLL_CONTROL_SERVER_MAX_CLIENTS; i++) {
    if (clients[i].bindingIndex != NULL_INDEX) {
      if (clients[i].fastPollTimeoutQs * MILLISECOND_TICKS_PER_QUARTERSECOND
          < elapsedFastPollTimeMs) {
        clients[i].bindingIndex = NULL_INDEX;
      } else if (fastPollTimeoutQs < clients[i].fastPollTimeoutQs) {
        fastPollTimeoutQs = clients[i].fastPollTimeoutQs;
      }
    }
  }

  if (fastPollTimeoutQs == 0) {
    return FALSE;
  } else {
    int32u newFastPollEndTimeMs = (fastPollStartTimeMs
                                   + (fastPollTimeoutQs
                                      * MILLISECOND_TICKS_PER_QUARTERSECOND));
    int32u remainingFastPollTimeMs = elapsedTimeInt32u(currentTimeMs,
                                                       newFastPollEndTimeMs);
    scheduleServerTick(endpoint, remainingFastPollTimeMs);
    return TRUE;
  }
}

static EmberAfStatus validateCheckInInterval(int8u endpoint,
                                             int32u newCheckInIntervalQs)
{
  EmberAfStatus status;
  int32u longPollIntervalQs;

  if (newCheckInIntervalQs == 0) {
    return EMBER_ZCL_STATUS_SUCCESS;
  }

  status = readServerAttribute(endpoint,
                               ZCL_LONG_POLL_INTERVAL_ATTRIBUTE_ID,
                               "long poll interval",
                               (int8u *)&longPollIntervalQs,
                               sizeof(longPollIntervalQs));
  if (status != EMBER_ZCL_STATUS_SUCCESS) {
    return status;
  } else if (newCheckInIntervalQs < longPollIntervalQs) {
    return EMBER_ZCL_STATUS_INVALID_VALUE;
  }

#ifdef ZCL_USING_POLL_CONTROL_CLUSTER_CHECK_IN_INTERVAL_MIN_ATTRIBUTE
  {
    int32u checkInIntervalMinQs;
    status = readServerAttribute(endpoint,
                                 ZCL_CHECK_IN_INTERVAL_MIN_ATTRIBUTE_ID,
                                 "check in interval min",
                                 (int8u *)&checkInIntervalMinQs,
                                 sizeof(checkInIntervalMinQs));
    if (status != EMBER_ZCL_STATUS_SUCCESS) {
      return status;
    } else if (newCheckInIntervalQs < checkInIntervalMinQs) {
      return EMBER_ZCL_STATUS_INVALID_VALUE;
    }
  }
#endif

  return EMBER_ZCL_STATUS_SUCCESS;
}

static EmberAfStatus validateLongPollInterval(int8u endpoint,
                                              int32u newLongPollIntervalQs)
{
  EmberAfStatus status;
  int32u checkInIntervalQs;
  int16u shortPollIntervalQs;

  status = readServerAttribute(endpoint,
                               ZCL_CHECK_IN_INTERVAL_ATTRIBUTE_ID,
                               "check in interval",
                               (int8u *)&checkInIntervalQs,
                               sizeof(checkInIntervalQs));
  if (status != EMBER_ZCL_STATUS_SUCCESS) {
    return status;
  } else if (checkInIntervalQs < newLongPollIntervalQs) {
    return EMBER_ZCL_STATUS_INVALID_VALUE;
  }

  status = readServerAttribute(endpoint,
                               ZCL_SHORT_POLL_INTERVAL_ATTRIBUTE_ID,
                               "short poll interval",
                               (int8u *)&shortPollIntervalQs,
                               sizeof(shortPollIntervalQs));
  if (status != EMBER_ZCL_STATUS_SUCCESS) {
    return status;
  } else if (newLongPollIntervalQs < shortPollIntervalQs) {
    return EMBER_ZCL_STATUS_INVALID_VALUE;
  }

#ifdef ZCL_USING_POLL_CONTROL_CLUSTER_LONG_POLL_INTERVAL_MIN_ATTRIBUTE
  {
    int32u longPollIntervalMinQs;
    status = readServerAttribute(endpoint,
                                 ZCL_LONG_POLL_INTERVAL_MIN_ATTRIBUTE_ID,
                                 "long poll interval min",
                                 (int8u *)&longPollIntervalMinQs,
                                 sizeof(longPollIntervalMinQs));
    if (status != EMBER_ZCL_STATUS_SUCCESS) {
      return status;
    } else if (newLongPollIntervalQs < longPollIntervalMinQs) {
      return EMBER_ZCL_STATUS_INVALID_VALUE;
    }
  }
#endif

  return EMBER_ZCL_STATUS_SUCCESS;
}

static EmberAfStatus validateShortPollInterval(int8u endpoint,
                                               int16u newShortPollIntervalQs)
{
  EmberAfStatus status;
  int32u longPollIntervalQs;
  status = readServerAttribute(endpoint,
                               ZCL_LONG_POLL_INTERVAL_ATTRIBUTE_ID,
                               "long poll interval",
                               (int8u *)&longPollIntervalQs,
                               sizeof(longPollIntervalQs));
  if (status != EMBER_ZCL_STATUS_SUCCESS) {
    return status;
  } else if (longPollIntervalQs < newShortPollIntervalQs) {
    return EMBER_ZCL_STATUS_INVALID_VALUE;
  }
  return EMBER_ZCL_STATUS_SUCCESS;
}

static EmberAfStatus validateFastPollInterval(int8u endpoint,
                                              int16u newFastPollIntervalQs)
{
#ifdef ZCL_USING_POLL_CONTROL_CLUSTER_FAST_POLL_TIMEOUT_MAX_ATTRIBUTE
  EmberAfStatus status;
  int16u fastPollTimeoutMaxQs;
  status = readServerAttribute(endpoint,
                               ZCL_FAST_POLL_TIMEOUT_MAX_ATTRIBUTE_ID,
                               "fast poll timeout max",
                               (int8u *)&fastPollTimeoutMaxQs,
                               sizeof(fastPollTimeoutMaxQs));
  if (status != EMBER_ZCL_STATUS_SUCCESS) {
    return status;
  } else if (fastPollTimeoutMaxQs < newFastPollIntervalQs) {
    return EMBER_ZCL_STATUS_INVALID_VALUE;
  }
#endif

  return EMBER_ZCL_STATUS_SUCCESS;
}

static EmberAfStatus validateCheckInIntervalMin(int8u endpoint,
                                                int32u newCheckInIntervalMinQs)
{
  EmberAfStatus status;
  int32u checkInIntervalQs;
  status = readServerAttribute(endpoint,
                               ZCL_CHECK_IN_INTERVAL_ATTRIBUTE_ID,
                               "check in interval",
                               (int8u *)&checkInIntervalQs,
                               sizeof(checkInIntervalQs));
  if (status != EMBER_ZCL_STATUS_SUCCESS) {
    return status;
  } else if (checkInIntervalQs < newCheckInIntervalMinQs) {
    return EMBER_ZCL_STATUS_INVALID_VALUE;
  }
  return EMBER_ZCL_STATUS_SUCCESS;
}

static EmberAfStatus validateLongPollIntervalMin(int8u endpoint,
                                                 int32u newLongPollIntervalMinQs)
{
  EmberAfStatus status;
  int32u longPollIntervalQs;
  status = readServerAttribute(endpoint,
                               ZCL_LONG_POLL_INTERVAL_ATTRIBUTE_ID,
                               "long poll interval",
                               (int8u *)&longPollIntervalQs,
                               sizeof(longPollIntervalQs));
  if (status != EMBER_ZCL_STATUS_SUCCESS) {
    return status;
  } else if (longPollIntervalQs < newLongPollIntervalMinQs) {
    return EMBER_ZCL_STATUS_INVALID_VALUE;
  }
  return EMBER_ZCL_STATUS_SUCCESS;
}

static EmberAfStatus validateFastPollTimeoutMax(int8u endpoint,
                                                int16u newFastPollTimeoutMaxQs)
{
  EmberAfStatus status;
  int16u fastPollTimeoutQs;
  status = readServerAttribute(endpoint,
                               ZCL_FAST_POLL_TIMEOUT_ATTRIBUTE_ID,
                               "fast poll timeout",
                               (int8u *)&fastPollTimeoutQs,
                               sizeof(fastPollTimeoutQs));
  if (status != EMBER_ZCL_STATUS_SUCCESS) {
    return status;
  } else if (newFastPollTimeoutMaxQs < fastPollTimeoutQs) {
    return EMBER_ZCL_STATUS_INVALID_VALUE;
  }
  return EMBER_ZCL_STATUS_SUCCESS;
}

void emberAfPollControlClusterServerInitCallback(int8u endpoint)
{
  int32u longPollIntervalQs;
  int16u shortPollIntervalQs;

  if (readServerAttribute(endpoint,
                          ZCL_LONG_POLL_INTERVAL_ATTRIBUTE_ID,
                          "long poll interval",
                          (int8u *)&longPollIntervalQs,
                          sizeof(longPollIntervalQs))
      == EMBER_ZCL_STATUS_SUCCESS) {
    emberAfSetLongPollIntervalQsCallback(longPollIntervalQs);
  }

  if (readServerAttribute(endpoint,
                          ZCL_SHORT_POLL_INTERVAL_ATTRIBUTE_ID,
                          "short poll interval",
                          (int8u *)&shortPollIntervalQs,
                          sizeof(shortPollIntervalQs))
      == EMBER_ZCL_STATUS_SUCCESS) {
    emberAfSetShortPollIntervalQsCallback(shortPollIntervalQs);
  }

  // TODO: Begin checking in after the network comes up instead of at startup.
  scheduleCheckIn(endpoint);
}

void emberAfPollControlClusterServerTickCallback(int8u endpoint)
{
  if (state == WAITING) {
    int16u fastPollTimeoutQs = 0;
    int8u i;
    for (i = 0; i < EMBER_AF_PLUGIN_POLL_CONTROL_SERVER_MAX_CLIENTS; i++) {
      if (clients[i].bindingIndex != NULL_INDEX
          && fastPollTimeoutQs < clients[i].fastPollTimeoutQs) {
        fastPollTimeoutQs = clients[i].fastPollTimeoutQs;
      }
    }
    if (fastPollTimeoutQs != 0) {
      state = POLLING;
      fastPollStartTimeMs = halCommonGetInt32uMillisecondTick();
      scheduleServerTick(endpoint,
                         (fastPollTimeoutQs
                          * MILLISECOND_TICKS_PER_QUARTERSECOND));
      return;
    }
  }

  state = INITIAL;
  deactivateServerTick(endpoint);
}

void emberAfPluginPollControlServerCheckInEndpointEventHandler(int8u endpoint)
{
  int8u bindingIndex, clientIndex;

  for (clientIndex = 0;
       clientIndex < EMBER_AF_PLUGIN_POLL_CONTROL_SERVER_MAX_CLIENTS;
       clientIndex++) {
    clients[clientIndex].bindingIndex = EMBER_NULL_BINDING;
  }

  for (bindingIndex = 0, clientIndex = 0;
       (bindingIndex < EMBER_BINDING_TABLE_SIZE
        && clientIndex < EMBER_AF_PLUGIN_POLL_CONTROL_SERVER_MAX_CLIENTS);
       bindingIndex++) {
    EmberBindingTableEntry binding;
    if (emberGetBinding(bindingIndex, &binding) == EMBER_SUCCESS
        && binding.type == EMBER_UNICAST_BINDING
        && binding.local == endpoint
        && binding.clusterId == ZCL_POLL_CONTROL_CLUSTER_ID) {
      emberAfFillCommandPollControlClusterCheckIn();
      if (emberAfSendCommandUnicast(EMBER_OUTGOING_VIA_BINDING, bindingIndex)
          == EMBER_SUCCESS) {
        clients[clientIndex].bindingIndex = bindingIndex;
        clients[clientIndex].fastPollTimeoutQs = 0;
        clientIndex++;
      }
    }
  }

  if (clientIndex == 0) {
    state = INITIAL;
    deactivateServerTick(endpoint);
  } else {
    state = WAITING;
    scheduleServerTick(endpoint, CHECK_IN_TIMEOUT_DURATION_MS);
  }

  scheduleCheckIn(endpoint);
}

EmberAfStatus emberAfPollControlClusterServerPreAttributeChangedCallback(int8u endpoint,
                                                                         EmberAfAttributeId attributeId,
                                                                         EmberAfAttributeType attributeType,
                                                                         int8u size,
                                                                         int8u *value)
{
  switch (attributeId) {
  case ZCL_CHECK_IN_INTERVAL_ATTRIBUTE_ID:
    {
      int32u newCheckInIntervalQs;
      MEMCOPY(&newCheckInIntervalQs, value, size);
      return validateCheckInInterval(endpoint, newCheckInIntervalQs);
    }
  case ZCL_LONG_POLL_INTERVAL_ATTRIBUTE_ID:
    {
      int32u newLongPollIntervalQs;
      MEMCOPY(&newLongPollIntervalQs, value, size);
      return validateLongPollInterval(endpoint, newLongPollIntervalQs);
    }
  case ZCL_SHORT_POLL_INTERVAL_ATTRIBUTE_ID:
    {
      int16u newShortPollIntervalQs;
      MEMCOPY(&newShortPollIntervalQs, value, size);
      return validateShortPollInterval(endpoint, newShortPollIntervalQs);
    }
  case ZCL_FAST_POLL_TIMEOUT_ATTRIBUTE_ID:
    {
      int16u newFastPollIntervalQs;
      MEMCOPY(&newFastPollIntervalQs, value, size);
      return validateFastPollInterval(endpoint, newFastPollIntervalQs);
    }
  case ZCL_CHECK_IN_INTERVAL_MIN_ATTRIBUTE_ID:
    {
      int32u newCheckInIntervalMinQs;
      MEMCOPY(&newCheckInIntervalMinQs, value, size);
      return validateCheckInIntervalMin(endpoint, newCheckInIntervalMinQs);
    }
  case ZCL_LONG_POLL_INTERVAL_MIN_ATTRIBUTE_ID:
    {
      int32u newLongPollIntervalMinQs;
      MEMCOPY(&newLongPollIntervalMinQs, value, size);
      return validateLongPollIntervalMin(endpoint, newLongPollIntervalMinQs);
    }
  case ZCL_FAST_POLL_TIMEOUT_MAX_ATTRIBUTE_ID:
    {
      int32u newFastPollTimeoutMaxQs;
      MEMCOPY(&newFastPollTimeoutMaxQs, value, size);
      return validateFastPollTimeoutMax(endpoint, newFastPollTimeoutMaxQs);
    }
  default:
    return EMBER_ZCL_STATUS_SUCCESS;
  }
}

void emberAfPollControlClusterServerAttributeChangedCallback(int8u endpoint,
                                                             EmberAfAttributeId attributeId)
{
  switch (attributeId) {
  case ZCL_CHECK_IN_INTERVAL_ATTRIBUTE_ID:
    scheduleCheckIn(endpoint);
    break;
  case ZCL_LONG_POLL_INTERVAL_ATTRIBUTE_ID:
    {
      EmberAfStatus status;
      int32u longPollIntervalQs;
      status = readServerAttribute(endpoint,
                                   ZCL_LONG_POLL_INTERVAL_ATTRIBUTE_ID,
                                   "long poll interval",
                                   (int8u *)&longPollIntervalQs,
                                   sizeof(longPollIntervalQs));
      if (status == EMBER_ZCL_STATUS_SUCCESS) {
        emberAfSetLongPollIntervalQsCallback(longPollIntervalQs);
      }
      break;
    }
  case ZCL_SHORT_POLL_INTERVAL_ATTRIBUTE_ID:
    {
      EmberAfStatus status;
      int16u shortPollIntervalQs;
      status = readServerAttribute(endpoint,
                                   ZCL_SHORT_POLL_INTERVAL_ATTRIBUTE_ID,
                                   "short poll interval",
                                   (int8u *)&shortPollIntervalQs,
                                   sizeof(shortPollIntervalQs));
      if (status == EMBER_ZCL_STATUS_SUCCESS) {
        emberAfSetShortPollIntervalQsCallback(shortPollIntervalQs);
      }
      break;
    }
  case ZCL_FAST_POLL_TIMEOUT_ATTRIBUTE_ID:
  case ZCL_CHECK_IN_INTERVAL_MIN_ATTRIBUTE_ID:
  case ZCL_LONG_POLL_INTERVAL_MIN_ATTRIBUTE_ID:
  case ZCL_FAST_POLL_TIMEOUT_MAX_ATTRIBUTE_ID:
  default:
    break;
  }
}

boolean emberAfPollControlClusterCheckInResponseCallback(int8u startFastPolling,
                                                         int16u fastPollTimeoutQs)
{
  EmberAfStatus status = EMBER_ZCL_STATUS_ACTION_DENIED;
  int8u clientIndex = findClientIndex();

  emberAfPollControlClusterPrintln("RX: CheckInResponse 0x%x, 0x%2x",
                                   startFastPolling,
                                   fastPollTimeoutQs);

  if (clientIndex != NULL_INDEX) {
    if (state == WAITING) {
      int8u endpoint = emberAfCurrentEndpoint();
      if (startFastPolling) {
        if (fastPollTimeoutQs == 0) {
          status = readServerAttribute(endpoint,
                                       ZCL_FAST_POLL_TIMEOUT_ATTRIBUTE_ID,
                                       "fast poll timeout",
                                       (int8u *)&fastPollTimeoutQs,
                                       sizeof(fastPollTimeoutQs));
        } else {
          status = validateFastPollInterval(endpoint, fastPollTimeoutQs);
        }
        if (status == EMBER_ZCL_STATUS_SUCCESS) {
          clients[clientIndex].fastPollTimeoutQs = fastPollTimeoutQs;
        } else {
          clients[clientIndex].bindingIndex = NULL_INDEX;
        }
      } else {
        status = EMBER_ZCL_STATUS_SUCCESS;
        clients[clientIndex].bindingIndex = NULL_INDEX;
      }

      // Calling the tick directly when in the waiting state will cause the
      // temporarily fast poll mode to stop and will begin the actual fast poll
      // mode if applicable.
      if (!pendingCheckInResponses()) {
        emberAfPollControlClusterServerTickCallback(endpoint);
      }
    } else {
      status = EMBER_ZCL_STATUS_TIMEOUT;
    }
  }

  emberAfSendImmediateDefaultResponse(status);
  return TRUE;
}

boolean emberAfPollControlClusterFastPollStopCallback(void)
{
  EmberAfStatus status = EMBER_ZCL_STATUS_ACTION_DENIED;
  int8u clientIndex = findClientIndex();

  emberAfPollControlClusterPrintln("RX: FastPollStop");

  if (clientIndex != NULL_INDEX) {
    if (state == POLLING) {
      int8u endpoint = emberAfCurrentEndpoint();
      status = EMBER_ZCL_STATUS_SUCCESS;
      clients[clientIndex].bindingIndex = NULL_INDEX;

      // Calling the tick directly in the polling state will cause the fast
      // poll mode to stop.
      if (!outstandingFastPollRequests(endpoint)) {
        emberAfPollControlClusterServerTickCallback(endpoint);
      }
    } else {
      status = EMBER_ZCL_STATUS_TIMEOUT;
    }
  }

  emberAfSendImmediateDefaultResponse(status);
  return TRUE;
}

boolean emberAfPollControlClusterSetLongPollIntervalCallback(int32u newLongPollIntervalQs)
{
#ifdef EMBER_AF_PLUGIN_POLL_CONTROL_SERVER_ACCEPT_SET_LONG_POLL_INTERVAL_COMMAND
  EmberAfStatus status;
  int8u endpoint = emberAfCurrentEndpoint();

  emberAfPollControlClusterPrintln("RX: SetLongPollInterval 0x%4x",
                                   newLongPollIntervalQs);

  // Trying to write the attribute will trigger the PreAttributeChanged
  // callback, which will handle validation.  If the write is successful, the
  // AttributeChanged callback will fire, which will handle setting the new
  // long poll interval.
  status = writeServerAttribute(endpoint,
                                ZCL_LONG_POLL_INTERVAL_ATTRIBUTE_ID,
                                "long poll interval",
                                (int8u *)&newLongPollIntervalQs,
                                ZCL_INT32U_ATTRIBUTE_TYPE);

  emberAfSendImmediateDefaultResponse(status);
  return TRUE;
#else
  return FALSE;
#endif
}

boolean emberAfPollControlClusterSetShortPollIntervalCallback(int16u newShortPollIntervalQs)
{
#ifdef EMBER_AF_PLUGIN_POLL_CONTROL_SERVER_ACCEPT_SET_SHORT_POLL_INTERVAL_COMMAND
  EmberAfStatus status;
  int8u endpoint = emberAfCurrentEndpoint();

  emberAfPollControlClusterPrintln("RX: SetShortPollInterval 0x%2x",
                                   newShortPollIntervalQs);

  // Trying to write the attribute will trigger the PreAttributeChanged
  // callback, which will handle validation.  If the write is successful, the
  // AttributeChanged callback will fire, which will handle setting the new
  // short poll interval.
  status = writeServerAttribute(endpoint,
                                ZCL_SHORT_POLL_INTERVAL_ATTRIBUTE_ID,
                                "short poll interval",
                                (int8u *)&newShortPollIntervalQs,
                                ZCL_INT16U_ATTRIBUTE_TYPE);

  emberAfSendImmediateDefaultResponse(status);
  return TRUE;
#else
  return FALSE;
#endif
}
