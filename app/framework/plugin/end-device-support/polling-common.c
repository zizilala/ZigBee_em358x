// Copyright 2013 Silicon Laboratories, Inc.

#include "app/framework/include/af.h"
#include "app/framework/plugin/end-device-support/end-device-support.h"

// The polling task is not used on the host.
#ifdef EZSP_HOST
  #define ADD_POLLING_TASK()
  #define REMOVE_POLLING_TASK()
#else
  #define ADD_POLLING_TASK()    emberAfAddToCurrentAppTasks(EMBER_AF_LAST_POLL_GOT_DATA)
  #define REMOVE_POLLING_TASK() emberAfRemoveFromCurrentAppTasks(EMBER_AF_LAST_POLL_GOT_DATA)
#endif

EmAfPollingState emAfPollingStates[EMBER_SUPPORTED_NETWORKS];

// This function is called when a poll completes and explains what happend with
// the poll.  If the number of sequential data polls not ACKed by the parent
// exceeds the threshold, we will try to find a new parent.
void emAfPollCompleteHandler(EmberStatus status, int8u limit)
{
  EmAfPollingState *state;
  int8u networkIndex;

  emberAfPushCallbackNetworkIndex();
  networkIndex = emberGetCurrentNetwork();
  state = &emAfPollingStates[networkIndex];

  if (emAfEnablePollCompletedCallback) {
    emberAfPluginEndDeviceSupportPollCompletedCallback(status);
  }

  switch (status) {
  case EMBER_SUCCESS:
    ADD_POLLING_TASK();
    emberAfDebugPrintln("poll nwk %d: got data", networkIndex);
    state->numPollsFailing = 0;
    break;
  case EMBER_MAC_NO_DATA:
    REMOVE_POLLING_TASK();
    emberAfDebugPrintln("poll nwk %d: no data", networkIndex);
    state->numPollsFailing = 0;
    break;
  case EMBER_DELIVERY_FAILED:
    // This means the air was busy, which we don't count as a failure.
    REMOVE_POLLING_TASK();
    emberAfDebugPrintln("poll nwk %d: delivery failed", networkIndex);
    break;
  case EMBER_MAC_NO_ACK_RECEIVED:
    // If we are performing key establishment, we can ignore this since the
    // parent could go away for long periods of time while doing ECC processes.
    if (emberAfPerformingKeyEstablishment()) {
      break;
    }
    // Count failures until we hit the limit, then we try a rejoin. If rejoin
    // fails, it will trigger a move.
    state->numPollsFailing++;
    if (limit <= state->numPollsFailing) {
      emberAfStartMoveCallback();
    }
    REMOVE_POLLING_TASK();
    break;
  default:
    emberAfDebugPrintln("poll nwk %d: 0x%x", networkIndex, status);
  }

  emberAfPopNetworkIndex();
}
