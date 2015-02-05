// *******************************************************************
// * af-main-common.c
// *
// * Code common to both the Host and SOC (system on a chip) versions
// * of the Application Framework.
// *
// * Copyright 2009 by Ember Corporation. All rights reserved.              *80*
// *******************************************************************

#include PLATFORM_HEADER     // Micro and compiler specific typedefs and macros

#if defined EZSP_HOST
  #include "stack/include/ember-types.h"
  #include "stack/include/error.h"
  #include "stack/include/library.h"
#else
  // Ember stack and related utilities
  #include "stack/include/ember.h"         // Main stack definitions
  #include "stack/include/cbke-crypto-engine.h"
#endif

// HAL - hardware abstraction layer
#include "hal/hal.h"
#include "app/util/serial/serial.h"  // Serial utility APIs

// CLI - command line interface
#include "app/util/serial/command-interpreter2.h"

#if defined EZSP_HOST
  // EZSP - ember serial host protocol
  #include "app/util/ezsp/ezsp-protocol.h"
  #include "app/util/ezsp/ezsp.h"
  #include "app/util/ezsp/serial-interface.h"
  #include "app/util/ezsp/ezsp-utils.h"
#endif

// Fragmentation.
#ifdef EMBER_AF_PLUGIN_FRAGMENTATION
#include "app/framework/plugin/fragmentation/fragmentation.h"
#endif

// Service discovery library
#include "service-discovery.h"

// determines the number of in-clusters and out-clusters based on defines
// in config.h
#include "af-main.h"

#include "attribute-storage.h"
#include "attribute-table.h"
#include "util.h"
#include "callback.h"
#include "print.h"
#include "config.h"
#include "app/framework/security/af-security.h"
#include "app/framework/security/crypto-state.h"

// Querying the Ember Stack for what libraries are present.
#include "app/util/common/library.h"

// ZDO - ZigBee Device Object
#include "app/util/zigbee-framework/zigbee-device-common.h"

#include "app/framework/plugin/partner-link-key-exchange/partner-link-key-exchange.h"
#include "app/framework/plugin/ota-storage-common/ota-storage.h"

//------------------------------------------------------------------------------

#define INVALID_MESSAGE_TAG 0xFF

#if defined(EMBER_AF_HAS_COORDINATOR_NETWORK)
  #if !defined(EMBER_AF_PLUGIN_CONCENTRATOR)
    #error "A Coordinator device (Trust Center) MUST enable the concentrator plugin to function correctly."
  #endif
#endif

#ifdef EMBER_AF_GENERATED_PLUGIN_STACK_STATUS_FUNCTION_DECLARATIONS
  EMBER_AF_GENERATED_PLUGIN_STACK_STATUS_FUNCTION_DECLARATIONS
#endif

#ifdef EMBER_AF_GENERATED_PLUGIN_MESSAGE_SENT_FUNCTION_DECLARATIONS
  EMBER_AF_GENERATED_PLUGIN_MESSAGE_SENT_FUNCTION_DECLARATIONS
#endif

// flags the user can turn on or off to make the printing behave differently
boolean emberAfPrintReceivedMessages = TRUE;

PGM EmberAfOtaImageId emberAfInvalidImageId = INVALID_OTA_IMAGE_ID;

static CallbackTableEntry messageSentCallbacks[EMBER_AF_MESSAGE_SENT_CALLBACK_TABLE_SIZE];


// We declare this variable 'const' but NOT PGM.  Those functions that we may use
// this variable would also have to declare it PGM in order to function
// correctly, which is not the case (e.g. emberFindKeyTableEntry()).
const EmberEUI64 emberAfNullEui64 = {0,0,0,0,0,0,0,0};

//------------------------------------------------------------------------------
// Forward declarations
static int8u getMessageSentCallbackIndex(void);
static void invalidateMessageSentCallbackEntry(int8u messageTag);
static EmberAfMessageSentFunction getMessageSentCallback(int8u tag);

static int8u getMessageSentCallbackIndex(void) 
{
  int8u i;
  for (i = 0; i < EMBER_AF_MESSAGE_SENT_CALLBACK_TABLE_SIZE; i++) {
    if (messageSentCallbacks[i].tag == INVALID_MESSAGE_TAG) {
      return i;
    }
  }

  return INVALID_MESSAGE_TAG;
}

static void invalidateMessageSentCallbackEntry(int8u tag)
{
  int8u i;
  for (i = 0; i < EMBER_AF_MESSAGE_SENT_CALLBACK_TABLE_SIZE; i++) {
    if (messageSentCallbacks[i].tag == tag) {
      messageSentCallbacks[i].tag = INVALID_MESSAGE_TAG;
      messageSentCallbacks[i].callback = NULL;
      return;
    }
  }
}

static EmberAfMessageSentFunction getMessageSentCallback(int8u tag) 
{
  int8u i;
  for (i = 0; i < EMBER_AF_MESSAGE_SENT_CALLBACK_TABLE_SIZE; i++) {
    if (messageSentCallbacks[i].tag == tag) {
      return messageSentCallbacks[i].callback;
    }
  }

  return NULL;
}

void emAfInitializeMessageSentCallbackArray(void)
{
  int8u i;
  for (i = 0; i < EMBER_AF_MESSAGE_SENT_CALLBACK_TABLE_SIZE; i++) {
    messageSentCallbacks[i].tag = INVALID_MESSAGE_TAG;
    messageSentCallbacks[i].callback = NULL;
  }
}

EmberAfCbkeKeyEstablishmentSuite emberAfIsFullSmartEnergySecurityPresent(void)
{
  EmberAfCbkeKeyEstablishmentSuite cbkeKeyEstablishmentSuite = EMBER_AF_INVALID_KEY_ESTABLISHMENT_SUITE;

#if defined EMBER_AF_HAS_SECURITY_PROFILE_SE
  EmberCertificateData cert;
  if ((emberGetLibraryStatus(EMBER_ECC_LIBRARY_ID)
       & EMBER_LIBRARY_PRESENT_MASK)
      && (EMBER_SUCCESS == emberGetCertificate(&cert)) ) {
    cbkeKeyEstablishmentSuite |= EMBER_AF_CBKE_KEY_ESTABLISHMENT_SUITE_163K1;
  }
  
  EmberCertificate283k1Data cert283k1;
  if((emberGetLibraryStatus(EMBER_ECC_LIBRARY_283K1_ID)
       & EMBER_LIBRARY_PRESENT_MASK)
       &&  (EMBER_SUCCESS == emberGetCertificate283k1(&cert283k1))) {
    cbkeKeyEstablishmentSuite |= EMBER_AF_CBKE_KEY_ESTABLISHMENT_SUITE_283K1;
  }
#endif

  return cbkeKeyEstablishmentSuite;
}

static EmberStatus send(EmberOutgoingMessageType type,
                        int16u indexOrDestination,
                        EmberApsFrame *apsFrame,
                        int16u messageLength,
                        int8u *message,
                        boolean broadcast,
                        EmberAfMessageSentFunction callback)
{
  EmberStatus status;
  int8u commandId, index, messageSentIndex;

  // The send APIs only deal with ZCL messages, so they must at least contain
  // the ZCL header.
  if (messageLength < EMBER_AF_ZCL_OVERHEAD) {
    return EMBER_ERR_FATAL;
  } else if (message[0] & ZCL_MANUFACTURER_SPECIFIC_MASK) {
    if (messageLength < EMBER_AF_ZCL_MANUFACTURER_SPECIFIC_OVERHEAD) {
      return EMBER_ERR_FATAL;
    }
    commandId = message[4];
  } else {
    commandId = message[2];
  }

  messageSentIndex = getMessageSentCallbackIndex();
  if (callback != NULL && messageSentIndex == INVALID_MESSAGE_TAG) {
    return EMBER_TABLE_FULL;
  }

  // The source endpoint in the APS frame MUST be valid at this point.  We use
  // it to set the appropriate outgoing network as well as the profile id in
  // the APS frame.
  index = emberAfIndexFromEndpoint(apsFrame->sourceEndpoint);
  if (index == 0xFF) {
    return EMBER_INVALID_ENDPOINT;
  }
  status = emberAfPushEndpointNetworkIndex(apsFrame->sourceEndpoint);
  if (status != EMBER_SUCCESS) {
    return status;
  }
  apsFrame->profileId = emberAfProfileIdFromIndex(index);

  // Encryption is turned on if it is required, but not turned off if it isn't.
  // This allows the application to send encrypted messages in special cases
  // that aren't covered by the specs by manually setting the encryption bit
  // prior to calling the send APIs.
  if (emberAfDetermineIfLinkSecurityIsRequired(commandId,
                                               FALSE, // incoming?
                                               broadcast,
                                               apsFrame->profileId,
                                               apsFrame->clusterId)) {
    apsFrame->options |= EMBER_APS_OPTION_ENCRYPTION;
  }

  if (messageLength
      <= emberAfMaximumApsPayloadLength(type, indexOrDestination, apsFrame)) {
    int8u messageTag = INVALID_MESSAGE_TAG;
    status = emAfSend(type,
                      indexOrDestination,
                      apsFrame,
                      (int8u)messageLength,
                      message,
                      &messageTag);
    if (callback != NULL && status == EMBER_SUCCESS && messageTag != INVALID_MESSAGE_TAG) {
      messageSentCallbacks[messageSentIndex].tag = messageTag;
      messageSentCallbacks[messageSentIndex].callback = callback;
    }
#ifdef EMBER_AF_PLUGIN_FRAGMENTATION
  } else if (!broadcast) {
    status = emAfFragmentationSendUnicast(type,
                                          indexOrDestination,
                                          apsFrame,
                                          message,
                                          messageLength);
    emberAfDebugPrintln("%pstart:len=%d.", "Fragmentation:", messageLength);
#endif
  } else {
    status = EMBER_MESSAGE_TOO_LONG;
  }

  if (status == EMBER_OPERATION_IN_PROGRESS
      && apsFrame->options & EMBER_APS_OPTION_DSA_SIGN) {
    // We consider "in progress" signed messages as being sent successfully.
    // The stack will send the message after signing.
    status = EMBER_SUCCESS;
    emAfSetCryptoOperationInProgress();
  }

  if (status == EMBER_SUCCESS) {
    emberAfAddToCurrentAppTasks(EMBER_AF_WAITING_FOR_DATA_ACK
                                | EMBER_AF_WAITING_FOR_ZCL_RESPONSE);
  }

  emberAfPopNetworkIndex();
  return status;
}

EmberStatus emberAfSendMulticastWithCallback(EmberMulticastId multicastId,
                                             EmberApsFrame *apsFrame,
                                             int16u messageLength,
                                             int8u *message,
                                             EmberAfMessageSentFunction callback)
{
  apsFrame->groupId = multicastId;
  return send(EMBER_OUTGOING_MULTICAST,
              multicastId,
              apsFrame,
              messageLength,
              message,
              TRUE, // broadcast?
              callback);
}

EmberStatus emberAfSendMulticast(EmberMulticastId multicastId,
                                 EmberApsFrame *apsFrame,
                                 int16u messageLength,
                                 int8u *message)
{
  return emberAfSendMulticastWithCallback(multicastId,
                                          apsFrame,
                                          messageLength,
                                          message,
                                          NULL);
}

EmberStatus emberAfSendBroadcastWithCallback(EmberNodeId destination,
                                             EmberApsFrame *apsFrame,
                                             int16u messageLength,
                                             int8u *message,
                                             EmberAfMessageSentFunction callback)
{
  return send(EMBER_OUTGOING_BROADCAST,
              destination,
              apsFrame,
              messageLength,
              message,
              TRUE, // broadcast?
              callback);
}

EmberStatus emberAfSendBroadcast(EmberNodeId destination,
                                 EmberApsFrame *apsFrame,
                                 int16u messageLength,
                                 int8u *message)
{
  return emberAfSendBroadcastWithCallback(destination,
                                          apsFrame,
                                          messageLength,
                                          message,
                                          NULL);
}

EmberStatus emberAfSendUnicastWithCallback(EmberOutgoingMessageType type,
                                           int16u indexOrDestination,
                                           EmberApsFrame *apsFrame,
                                           int16u messageLength,
                                           int8u *message,
                                           EmberAfMessageSentFunction callback)
{
  // The source endpoint in the APS frame MAY NOT be valid at this point if the
  // outgoing type is "via binding."
  if (type == EMBER_OUTGOING_VIA_BINDING) {
    // If using binding, set the endpoints based on those in the binding.  The
    // cluster in the binding is not used because bindings can be used to send
    // messages with any cluster id, not just the one set in the binding.
    EmberBindingTableEntry binding;
    EmberStatus status = emberGetBinding(indexOrDestination, &binding);
    if (status != EMBER_SUCCESS) {
      return status;
    }
    apsFrame->sourceEndpoint = binding.local;
    apsFrame->destinationEndpoint = binding.remote;
  }
  return send(type,
              indexOrDestination,
              apsFrame,
              messageLength,
              message,
              FALSE, // broadcast?
              callback);
}

EmberStatus emberAfSendUnicast(EmberOutgoingMessageType type,
                               int16u indexOrDestination,
                               EmberApsFrame *apsFrame,
                               int16u messageLength,
                               int8u *message)
{
  return emberAfSendUnicastWithCallback(type,
                                        indexOrDestination,
                                        apsFrame,
                                        messageLength,
                                        message,
                                        NULL);
}

EmberStatus emberAfSendUnicastToBindingsWithCallback(EmberApsFrame *apsFrame,
                                                     int16u messageLength,
                                                     int8u* message,
                                                     EmberAfMessageSentFunction callback)
{
  EmberStatus status = EMBER_INVALID_BINDING_INDEX;
  int8u i;

  for (i = 0; i < EMBER_BINDING_TABLE_SIZE; i++) {
    EmberBindingTableEntry binding;
    status = emberGetBinding(i, &binding);
    if (status != EMBER_SUCCESS) {
      return status;
    }
    if (binding.type == EMBER_UNICAST_BINDING
        && binding.local == apsFrame->sourceEndpoint
        && binding.clusterId == apsFrame->clusterId) {
      apsFrame->destinationEndpoint = binding.remote;
      status = send(EMBER_OUTGOING_VIA_BINDING,
                    i,
                    apsFrame,
                    messageLength,
                    message,
                    FALSE, // broadcast?
                    callback);
      if (status != EMBER_SUCCESS) {
        return status;
      }
    }
  }

  return status;
}

EmberStatus emberAfSendUnicastToBindings(EmberApsFrame *apsFrame,
                                         int16u messageLength,
                                         int8u* message)
{
  return emberAfSendUnicastToBindingsWithCallback(apsFrame,
                                                  messageLength,
                                                  message,
                                                  NULL);
}

EmberStatus emberAfSendInterPan(EmberPanId panId,
                                const EmberEUI64 eui64,
                                EmberNodeId nodeId,
                                EmberMulticastId multicastId,
                                EmberAfClusterId clusterId,
                                EmberAfProfileId profileId,
                                int16u messageLength,
                                int8u* messageBytes)
{
  EmberAfInterpanHeader header;
  MEMSET(&header, 0, sizeof(EmberAfInterpanHeader));
  header.panId = panId;
  header.shortAddress = nodeId;
  if (eui64 != NULL) {
    MEMCOPY(header.longAddress, eui64, EUI64_SIZE);
    header.options |= EMBER_AF_INTERPAN_OPTION_MAC_HAS_LONG_ADDRESS;
    header.messageType = EMBER_AF_INTER_PAN_UNICAST;
  } else if (multicastId != 0) {
    header.groupId = multicastId;
    header.messageType = EMBER_AF_INTER_PAN_MULTICAST;
  } else {
    header.messageType = (nodeId < EMBER_BROADCAST_ADDRESS
                          ? EMBER_AF_INTER_PAN_UNICAST
                          : EMBER_AF_INTER_PAN_BROADCAST);
  }
  header.profileId = profileId;
  header.clusterId = clusterId;
  return emberAfInterpanSendMessageCallback(&header,
                                            messageLength,
                                            messageBytes);
}

void emberAfPrintMessageData(int8u* data, int16u length)
{
#if defined EMBER_AF_PRINT_APP
  emberAfAppPrint(" payload (len %2x) [", length); 
  emberAfAppPrintBuffer(data, length, TRUE);
  emberAfAppPrintln("]"); 
#endif
}

void emAfPrintStatus(PGM_P task,
                     EmberStatus status)
{
  if (status == EMBER_SUCCESS) {
    emberAfPrint(emberAfPrintActiveArea,
                 "%p: %p", 
                 "Success",
                 task );
  } else {
    emberAfPrint(emberAfPrintActiveArea,
                 "%p: %p: 0x%x", 
                 "Error",
                 task, 
                 status);
  }
}

static EmberStatus broadcastPermitJoin(int8u duration)
{
  EmberStatus status;
  int8u data[3] = { 0,   // sequence number (filled in later)
                    0,   // duration (filled in below)
                    0 }; // TC significance (not used)

  data[1] = duration;
  status = emberSendZigDevRequest(EMBER_BROADCAST_ADDRESS,
                                  PERMIT_JOINING_REQUEST,
                                  0,   // APS options
                                  data,
                                  3);  // length
  return status;
}

// Public API
EmberStatus emberAfPermitJoin(int8u duration,
                              boolean broadcastMgmtPermitJoin)
{
  // Permit joining forever is bad behavior, so we want to limit
  // this.  If 254 is not enough a re-broadcast should be done later.
  if (duration == 255) {
    emberAfAppPrintln("Limiting duration of permit join from forever (255) to 254");
    duration = 254;
  }
  return emAfPermitJoin(duration,
                        broadcastMgmtPermitJoin);
}

// Old API that doesn't restrict prevent permit joining forever (255)
EmberStatus emAfPermitJoin(int8u duration, 
                           boolean broadcastMgmtPermitJoin) 
{
  EmberStatus status = emberPermitJoining(duration);
  emberAfAppPrintln("pJoin for %d sec: 0x%x", duration, status); 
  if (status == EMBER_SUCCESS && broadcastMgmtPermitJoin) {
    status = broadcastPermitJoin(duration);
  }
  return status;
}


// ******************************************************************
// Functions called by the Serial Command Line Interface (CLI)
// ******************************************************************

boolean emAfProcessZdo(EmberNodeId sender,
                       EmberApsFrame* apsFrame,
                       int8u* message, 
                       int16u length)
{
  if (apsFrame->profileId != EMBER_ZDO_PROFILE_ID) {
    return FALSE;
  }
  
  // To make the printing simpler, we assume all 'request' messages
  // have a status of 0x00.  Request messages have no status value in them
  // but saying 'success' (0x00) seems appropriate.
  // Response messages will have their status value printed appropriately.
  emberAfZdoPrintln("RX: ZDO, command 0x%2x, status: 0x%X", 
                    apsFrame->clusterId,
                    (apsFrame->clusterId >= CLUSTER_ID_RESPONSE_MINIMUM
                     ? message[1]
                     : 0));
    
  if (apsFrame->clusterId == SIMPLE_DESCRIPTOR_RESPONSE) {
    emberAfZdoPrintln("RX: %p Desc Resp", "Simple");
  } else if (apsFrame->clusterId == MATCH_DESCRIPTORS_RESPONSE) {
    emberAfZdoPrint("RX: %p Desc Resp", "Match");
    emberAfZdoPrintln(", Matches: %d", message[4]);
  } else if (apsFrame->clusterId == END_DEVICE_BIND_RESPONSE) {
    emberAfZdoPrintln("RX: End dev bind response, status=%x", message[1]);
  } else if (apsFrame->clusterId == END_DEVICE_ANNOUNCE) {
    emberAfZdoPrintln("Device Announce: 0x%2x",
                      (int16u)(message[1])
                      + (int16u)(message[2] << 8));
  } else if (apsFrame->clusterId == IEEE_ADDRESS_RESPONSE) {
    emberAfZdoPrintln("RX: IEEE Address Response");
  } else if (apsFrame->clusterId == ACTIVE_ENDPOINTS_RESPONSE) {
    emberAfZdoPrintln("RX: Active EP Response, Count: %d", message[4]);
  }

#if defined(EMBER_AF_PLUGIN_IAS_ZONE_CLIENT)
  emberAfPluginIasZoneClientZdoCallback(sender, apsFrame, message, length);
#endif

  if (emberAfPreZDOMessageReceivedCallback(sender, apsFrame, message, length)) {
    goto zdoProcessingDone;
  }

  if (apsFrame->clusterId == BIND_RESPONSE) {
    emberAfPartnerLinkKeyExchangeResponseCallback(sender, message[1]);
  }

 zdoProcessingDone:
  // if it is a zdo response we can remove the zdo waiting task
  // and let a sleepy go back into hibernation
  if (apsFrame->clusterId > CLUSTER_ID_RESPONSE_MINIMUM) {
    emberAfRemoveFromCurrentAppTasks(EMBER_AF_WAITING_FOR_ZDO_RESPONSE);
  }

  return TRUE;
}

void emAfIncomingMessageHandler(EmberIncomingMessageType type,
                                EmberApsFrame *apsFrame,
                                int8u lastHopLqi,
                                int8s lastHopRssi,
                                int16u messageLength,
                                int8u *messageContents)
{
  EmberNodeId sender = emberGetSender();
  EmberAfIncomingMessage im;

#ifdef EMBER_AF_PLUGIN_FRAGMENTATION
  if (emAfFragmentationIncomingMessage(apsFrame,
                                       sender,
                                       &messageContents,
                                       &messageLength)) {
    emberAfDebugPrintln("%pfragment processed.", "Fragmentation:");
    return;
  }
#endif //EMBER_AF_PLUGIN_FRAGMENTATION

  emberAfDebugPrintln("Processing message: len=%d profile=%2x cluster=%2x",
                      messageLength,
                      apsFrame->profileId,
                      apsFrame->clusterId);
  emberAfDebugFlush();

  // Populate the incoming message struct to pass to the incoming message
  // callback.
  im.type              = type;
  im.apsFrame          = apsFrame;
  im.message           = messageContents;
  im.msgLen            = messageLength;
  im.source            = sender;
  im.lastHopLqi        = lastHopLqi;
  im.lastHopRssi       = lastHopRssi;
  im.bindingTableIndex = emberAfGetBindingIndex();
  im.addressTableIndex = emberAfGetAddressIndex();
  im.networkIndex      = emberGetCurrentNetwork();
  if (emberAfPreMessageReceivedCallback(&im)) {
    return;
  }

  // Handle service discovery responses.
  if (emAfServiceDiscoveryIncoming(sender,
                                   apsFrame,
                                   messageContents,
                                   messageLength)) {
    return;
  }

  // Handle ZDO messages.
  if (emAfProcessZdo(sender, apsFrame, messageContents, messageLength)) {
    return;
  }

  // Handle ZCL messages.
  if (emberAfProcessMessage(apsFrame,
                            type,
                            messageContents,
                            messageLength,
                            sender,
                            NULL)) { // no inter-pan header
    return;
  }
}

static void printMessage(EmberIncomingMessageType tyep,
                         EmberApsFrame* apsFrame,
                         int16u messageLength,
                         int8u* messageContents)
{
  emberAfAppPrint("Profile: %p (0x%2X), Cluster: 0x%2X, %d bytes,",
                     (apsFrame->profileId == EMBER_ZDO_PROFILE_ID
                      ? "ZDO"
                      : (apsFrame->profileId == SE_PROFILE_ID 
                         ? "SE"
                         : (apsFrame->profileId == 0x0104
                            ? "HA"
                            : "??"))),
                     apsFrame->profileId,
                     apsFrame->clusterId,
                     messageLength);
  if (apsFrame->profileId != EMBER_ZDO_PROFILE_ID
      && messageLength >= 3) {
    emberAfAppPrint(" ZCL %p Cmd ID: %d", 
                    (messageContents[0] & ZCL_CLUSTER_SPECIFIC_COMMAND
                     ? "Cluster"
                     : "Global"),
                    messageContents[2]);
  } 
  emberAfAppPrintln("");
}

void emAfMessageSentHandler(EmberOutgoingMessageType type,
                            int16u indexOrDestination,
                            EmberApsFrame *apsFrame,
                            EmberStatus status,
                            int16u messageLength,
                            int8u *messageContents,
                            int8u messageTag)
{
  EmberAfMessageSentFunction callback;
  if (status != EMBER_SUCCESS) {
    emberAfAppPrint("%ptx %x, ", "ERROR: ", status);
    printMessage(type, apsFrame, messageLength, messageContents);
  }

  callback = getMessageSentCallback(messageTag);
  invalidateMessageSentCallbackEntry(messageTag);
  emberAfDeliveryStatusCallback(type, status);

  if (status == EMBER_SUCCESS
      && apsFrame->profileId == EMBER_ZDO_PROFILE_ID
      && apsFrame->clusterId < CLUSTER_ID_RESPONSE_MINIMUM) {
    emberAfAddToCurrentAppTasks(EMBER_AF_WAITING_FOR_ZDO_RESPONSE);
  }

  emberAfRemoveFromCurrentAppTasks(EMBER_AF_WAITING_FOR_DATA_ACK);

  if (messageContents != NULL && messageContents[0] & ZCL_CLUSTER_SPECIFIC_COMMAND) {
    emberAfClusterMessageSentCallback(type,
                                      indexOrDestination,
                                      apsFrame,
                                      messageLength,
                                      messageContents,
                                      status);
  }

  if (callback != NULL) {
    (*callback)(type, indexOrDestination, apsFrame, messageLength, messageContents, status);
  }

#ifdef EMBER_AF_GENERATED_PLUGIN_MESSAGE_SENT_FUNCTION_CALLS
  EMBER_AF_GENERATED_PLUGIN_MESSAGE_SENT_FUNCTION_CALLS
#endif

    emberAfMessageSentCallback(type,
                               indexOrDestination,
                               apsFrame,
                               messageLength,
                               messageContents,
                               status);

}

#ifdef EMBER_AF_PLUGIN_FRAGMENTATION
void emAfFragmentationMessageSentHandler(EmberOutgoingMessageType type,
                                         int16u indexOrDestination,
                                         EmberApsFrame *apsFrame,
                                         int8u *buffer,
                                         int16u bufLen,
                                         EmberStatus status)
{
  // the fragmented message is no longer in process
  emberAfDebugPrintln("%pend.", "Fragmentation:");
  emAfMessageSentHandler(type,
                         indexOrDestination,
                         apsFrame,
                         status,
                         bufLen,
                         buffer,
                         INVALID_MESSAGE_TAG);
}
#endif //EMBER_AF_PLUGIN_FRAGMENTATION

void emAfStackStatusHandler(EmberStatus status)
{
  emberAfAppFlush();

  // To be extra careful, we clear the network cache whenever a new status is
  // received.
  emAfClearNetworkCache(emberGetCurrentNetwork());

  switch (status) {
    case EMBER_NETWORK_UP:
    case EMBER_TRUST_CENTER_EUI_HAS_CHANGED:  // also means NETWORK_UP
      {
        // Set the runtime security settings as soon as the stack goes up.
        EmberExtendedSecurityBitmask oldExtended;
        EmberExtendedSecurityBitmask newExtended;
        const EmberAfSecurityProfileData *data = emAfGetCurrentSecurityProfileData();
        boolean trustCenter = (emberAfGetNodeId() == EMBER_TRUST_CENTER_NODE_ID);
        if (data != NULL) {
          newExtended = (trustCenter
                      ? data->tcExtendedBitmask
                      : data->nodeExtendedBitmask);
        }

        emberGetExtendedSecurityBitmask(&oldExtended);
        if (oldExtended & EMBER_EXT_NO_FRAME_COUNTER_RESET){
          newExtended |= EMBER_EXT_NO_FRAME_COUNTER_RESET;
        }

        emberAfSecurityInitCallback(NULL, &newExtended, trustCenter);
        emberSetExtendedSecurityBitmask(newExtended);

        emberAfAppPrintln("%p%pUP 0x%2X", "EMBER_", "NETWORK_", emberAfGetNodeId());
        emberAfAppFlush();
#if defined(EMBER_TEST)
        simulatedTimePasses();
#endif        

        if (status == EMBER_TRUST_CENTER_EUI_HAS_CHANGED) {
          emberAfAppPrintln("Trust Center EUI has changed.");
          // We abort registration because we want to clear out any previous
          // state and force it to start anew.  One of two results will occur after
          // we restart registration later.
          // (1) It succeeds and we are on a new network with a new TC, in which
          //     case we need to kick off key establishment to re-authenticate it 
          //     and also re-discover other ESIs.
          // (2) It will fail, in which case we have to reboot to forget the untrusted
          //     network and its settings.
          emberAfRegistrationAbortCallback();
        } else {
          emberStartWritingStackTokens();
        }

        // This kicks off registration for newly joined devices.  If registration
        // already occurred, nothing will happen here.
        emberAfRegistrationStartCallback();
        break;
      }

    case EMBER_RECEIVED_KEY_IN_THE_CLEAR:
    case EMBER_NO_NETWORK_KEY_RECEIVED:
    case EMBER_NO_LINK_KEY_RECEIVED:
    case EMBER_PRECONFIGURED_KEY_REQUIRED:
    case EMBER_MOVE_FAILED:
    case EMBER_JOIN_FAILED:
    case EMBER_NO_BEACONS:
    case EMBER_CANNOT_JOIN_AS_ROUTER:
    case EMBER_NETWORK_DOWN:
      if (status == EMBER_NETWORK_DOWN) {
        emberAfAppPrintln("%p%pDOWN", "EMBER_", "NETWORK_");
      } else {
      emberAfAppPrintln("%pJOIN%p", "EMBER_", "_FAILED");
    }
    emberAfAppFlush();
    emberAfStackDown();
    break;

  default:
    emberAfDebugPrintln("EVENT: stackStatus 0x%x", status);
  }

  emberAfAppFlush();

#ifdef EMBER_AF_GENERATED_PLUGIN_STACK_STATUS_FUNCTION_CALLS
  EMBER_AF_GENERATED_PLUGIN_STACK_STATUS_FUNCTION_CALLS
#endif

  if (emberAfStackStatusCallback(status)) {
    // Bug 13690: Even if the callback handled the status, we still want to do
    // our things regarding the stack status. Therefore we no longer return
    // here. For now we just print a warning message to inform the customer that
    // a TRUE value returned by emberAfStackStatusCallback() no longer avoid
    // the Stack Status to be handled by our code.
    emberAfAppPrintln("The app framework is handling the stack status.");
  }
}

#ifdef EMBER_AF_USE_STANDARD_NETWORK_INIT
  #define networkInit emberNetworkInit
#else
static EmberStatus networkInitExtended(void)
  {
    EmberNetworkInitStruct networkInitStruct = { 
      EMBER_AF_CUSTOM_NETWORK_INIT_OPTIONS   // EmberNetworkInitBitmask value
    };
    return emberNetworkInitExtended(&networkInitStruct);
  }
  #define networkInit networkInitExtended
#endif

// If possible, initialize each network.  For ZigBee PRO networks, the node
// type of the device must match the one used previously, but note that
// coordinator-capable devices are allowed to initialize as routers.
void emAfNetworkInit(void)
{
  int8u i;
  for (i = 0; i < EMBER_SUPPORTED_NETWORKS; i++) {
    boolean initialize = TRUE;
    emberAfPushNetworkIndex(i);
    emAfClearNetworkCache(i);
    if (emAfNetworks[i].type == EM_AF_NETWORK_TYPE_ZIGBEE_PRO) {
      EmberNodeType nodeType;
      if (emAfNetworks[i].variant.pro.nodeType == EMBER_COORDINATOR) {
        zaTrustCenterSecurityPolicyInit();
      }
      if (emberAfGetNodeType(&nodeType) != EMBER_SUCCESS
          || (nodeType != emAfNetworks[i].variant.pro.nodeType
              && (emAfNetworks[i].variant.pro.nodeType != EMBER_COORDINATOR
                  || nodeType != EMBER_ROUTER))) {
        initialize = FALSE;
      }
    }
    if (initialize) {
      networkInit();
    }
    emberAfPopNetworkIndex();
  }
}

int8u emberAfCopyBigEndianEui64Argument(int8s index, EmberEUI64 destination)
{
  EmberEUI64 tmp;
  int8u length = emberCopyEui64Argument(index, tmp);
  emberReverseMemCopy(destination, tmp, EUI64_SIZE);
  return length;
}

// form-and-join library callbacks.
void emberUnusedPanIdFoundHandler(EmberPanId panId, int8u channel)
{
  emberAfPushCallbackNetworkIndex();
  emberAfUnusedPanIdFoundCallback(panId, channel);
  emberAfPopNetworkIndex();
}

void emberJoinableNetworkFoundHandler(EmberZigbeeNetwork *networkFound,
                                      int8u lqi,
                                      int8s rssi)
{
  emberAfPushCallbackNetworkIndex();
  emberAfJoinableNetworkFoundCallback(networkFound, lqi, rssi);
  emberAfPopNetworkIndex();
}

void emberScanErrorHandler(EmberStatus status)
{
  emberAfPushCallbackNetworkIndex();
  emberAfScanErrorCallback(status);
  emberAfPopNetworkIndex();
}

EmberStatus emberAfFormNetwork(EmberNetworkParameters *parameters)
{
  EmberStatus status = EMBER_INVALID_CALL;
#ifdef EMBER_AF_HAS_COORDINATOR_NETWORK
  if (emAfCurrentNetwork->type == EM_AF_NETWORK_TYPE_ZIGBEE_PRO
      && emAfCurrentNetwork->variant.pro.nodeType == EMBER_COORDINATOR) {
    zaTrustCenterSecurityInit();
    emberAfCorePrintln("%ping on ch %d, panId 0x%2X",
                       "Form", 
                       parameters->radioChannel,
                       parameters->panId);
    emberAfCoreFlush();
    status = emberFormNetwork(parameters);
  }
#endif
  return status;
}

EmberStatus emberAfJoinNetwork(EmberNetworkParameters *parameters)
{
  EmberStatus status = EMBER_INVALID_CALL;
  if (emAfCurrentNetwork->type == EM_AF_NETWORK_TYPE_ZIGBEE_PRO) {
    EmberNodeType nodeType = emAfCurrentNetwork->variant.pro.nodeType;
    if (nodeType == EMBER_COORDINATOR) {
      nodeType = EMBER_ROUTER;
    }
    zaNodeSecurityInit();
    emberAfCorePrintln("%ping on ch %d, panId 0x%2X",
                       "Join", 
                       parameters->radioChannel,
                       parameters->panId);
    status = emberJoinNetwork(nodeType, parameters);
  }
  return status;
}

// mfgString is expected to be +1 of MFG_STRING_MAX_LENGTH
void emberAfFormatMfgString(int8u* mfgString)
{
  int8u i;
  emberAfGetMfgString(mfgString);

  for (i = 0; i < MFG_STRING_MAX_LENGTH; i++) {
    // The MFG string is not necessarily NULL terminated.
    // Uninitialized bytes are left at 0xFF so we make sure
    // it is NULL terminated.
    if (mfgString[i] == 0xFF) {
      mfgString[i] = '\0';
    }
  }
  mfgString[MFG_STRING_MAX_LENGTH] = '\0';
}

static PGM EmberReleaseTypeStruct releaseTypes[] = {
  EMBER_RELEASE_TYPE_TO_STRING_STRUCT_DATA
};

void emAfParseAndPrintVersion(EmberVersion versionStruct)
{
  int8u i = 0;
  PGM_P typeText = NULL;
  while (releaseTypes[i].typeString != NULL) {
    if (releaseTypes[i].typeNum == versionStruct.type) {
      typeText = releaseTypes[i].typeString;
    }
    i++;
  }
  emberAfAppPrint("stack ver. [%d.%d.%d",
                  versionStruct.major,
                  versionStruct.minor,
                  versionStruct.patch);
  if (versionStruct.special != 0) {
    emberAfAppPrint(".%d",
                    versionStruct.special);
  }
  emberAfAppPrintln(" %p build %d]", 
                    (typeText == NULL
                     ? "???"
                     : typeText),
                    versionStruct.build);
  emberAfAppFlush();
}

//hal button isr
void halButtonIsr(int8u button, int8u state) {
  #ifdef EMBER_CALLBACK_HAL_BUTTON_ISR
  emberAfHalButtonIsrCallback(button, state);
  #endif
}


