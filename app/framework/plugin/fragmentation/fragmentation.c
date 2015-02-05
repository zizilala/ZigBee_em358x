// *****************************************************************************
// * fragmentation.c
// *
// * Splits long messages into smaller fragments for transmission and
// * reassembles received fragments into full messages.
// *
// * Copyright 2010 by Ember Corporation. All rights reserved.              *80*
// *****************************************************************************

#include "app/framework/include/af.h"
#include "fragmentation.h"

//------------------------------------------------------------------------------
// Globals

EmberEventControl emAfFragmentationEvents[10];

#if defined(EMBER_AF_PLUGIN_FRAGMENTATION_FREE_OUTGOING_MESSAGE_PRIOR_TO_FINAL_ACK)
#define FREE_OUTGOING_MESSAGE_PRIOR_TO_FINAL_ACK TRUE
#else
#define FREE_OUTGOING_MESSAGE_PRIOR_TO_FINAL_ACK FALSE
#endif

static const boolean freeOutgoingMessagePriorToFinalAck = FREE_OUTGOING_MESSAGE_PRIOR_TO_FINAL_ACK;

#define UNUSED_TX_PACKET_ENTRY 0xFF

//------------------------------------------------------------------------------
// Forward Declarations

static EmberStatus sendNextFragments(txFragmentedPacket* txPacket);
static void abortTransmission(txFragmentedPacket *txPacket, EmberStatus status);
static txFragmentedPacket* getFreeTxPacketEntry(void);
static txFragmentedPacket* txPacketLookUp(EmberApsFrame *apsFrame);

// We separate the outgoing buffer from the txPacket entry to allow us to keep around
// data about previous fragmented messages that have sent their last packet
// but not yet been acknowledged.  This saves space by not replicating the entire
// buffer required to store the outgoing message.  However, in that case we do not
// pass the complete message to the message sent handler.
static txFragmentedPacket txPackets[EMBER_AF_PLUGIN_FRAGMENTATION_MAX_OUTGOING_PACKETS];
static int8u txMessageBuffers[EMBER_AF_PLUGIN_FRAGMENTATION_MAX_OUTGOING_PACKETS]
                             [EMBER_AF_PLUGIN_FRAGMENTATION_BUFFER_SIZE];

static txFragmentedPacket txPacketAwaitingFinalAck = {
  UNUSED_TX_PACKET_ENTRY,
};


#if defined(EMBER_TEST)
  #define NO_BLOCK_TO_DROP 0xFF
  int8u emAfPluginFragmentationArtificiallyDropBlockNumber = NO_BLOCK_TO_DROP;
  #define artificiallyDropBlock(block) (block == emAfPluginFragmentationArtificiallyDropBlockNumber)
  #define clearArtificiallyDropBlock() emAfPluginFragmentationArtificiallyDropBlockNumber = NO_BLOCK_TO_DROP;
  #define artificiallyDropBlockPrintln(format, arg) emberAfCorePrintln((format), (arg))

#else
  #define artificiallyDropBlock(block) FALSE
  #define clearArtificiallyDropBlock()
  #define artificiallyDropBlockPrintln(format, arg)

#endif

//------------------------------------------------------------------------------
// Functions

EmberStatus emAfFragmentationSendUnicast(EmberOutgoingMessageType type,
                                         int16u indexOrDestination,
                                         EmberApsFrame *apsFrame,
                                         int8u *buffer,
                                         int16u bufLen)
{
  EmberStatus status;
  int16u fragments;
  txFragmentedPacket* txPacket;

  if (emberFragmentWindowSize == 0) {
    return EMBER_INVALID_CALL;
  }

  txPacket = getFreeTxPacketEntry();
  if (txPacket == NULL) {
    return EMBER_MAX_MESSAGE_LIMIT_REACHED;
  }

  txPacket->indexOrDestination = indexOrDestination;
  MEMCOPY(&txPacket->apsFrame, apsFrame, sizeof(EmberApsFrame));
  txPacket->apsFrame.options |=
      (EMBER_APS_OPTION_FRAGMENT | EMBER_APS_OPTION_RETRY);
  
  emAfPluginFragmentationHandleSourceRoute(txPacket,
                                           indexOrDestination);

  MEMCOPY(txPacket->bufferPtr, buffer, bufLen);
  txPacket->bufLen = bufLen;
  txPacket->fragmentLen = emberAfMaximumApsPayloadLength(type,
                                                         indexOrDestination,
                                                         &txPacket->apsFrame);
  fragments = ((bufLen + txPacket->fragmentLen - 1) / txPacket->fragmentLen);
  if (fragments > MAX_INT8U_VALUE
      || bufLen > EMBER_AF_PLUGIN_FRAGMENTATION_BUFFER_SIZE) {
    return EMBER_MESSAGE_TOO_LONG;
  }
  txPacket->messageType = type;
  txPacket->fragmentCount = (int8u)fragments;
  txPacket->fragmentBase = 0;
  txPacket->fragmentsInTransit = 0;

  status = sendNextFragments(txPacket);

  if (status == EMBER_SUCCESS) {
    // Set the APS sequence number in the passed apsFrame.
    apsFrame->sequence = txPacket->sequence;
  } else {
    txPacket->messageType = UNUSED_TX_PACKET_ENTRY;
  }

  return status;
}

boolean emAfFragmentationMessageSent(EmberApsFrame *apsFrame,
                                     EmberStatus status)
{
  if (apsFrame->options & EMBER_APS_OPTION_FRAGMENT) {
    // If the outgoing APS frame is fragmented, we should always have a
    // a corresponding record in the txFragmentedPacket array.
    txFragmentedPacket *txPacket = txPacketLookUp(apsFrame);
    if (txPacket == NULL)
      return TRUE;

    if (status == EMBER_SUCCESS) {
      txPacket->fragmentsInTransit--;
      if (txPacket->fragmentsInTransit == 0) {
        txPacket->fragmentBase += emberFragmentWindowSize;
        abortTransmission(txPacket, sendNextFragments(txPacket));
      }
    } else {
      abortTransmission(txPacket, status);
    }
    return TRUE;
  } else {
    return FALSE;
  }
}

static EmberStatus sendNextFragments(txFragmentedPacket* txPacket)
{
  int8u i;
  int16u offset;

  emberAfCorePrintln("Sending fragment %d of %d",
                     txPacket->fragmentBase,
                     txPacket->fragmentCount);

  offset = txPacket->fragmentBase * txPacket->fragmentLen;

  // Send fragments until the window is full.
  for (i = txPacket->fragmentBase;
       i < txPacket->fragmentBase + emberFragmentWindowSize
       && i < txPacket->fragmentCount;
       i++) {
    EmberStatus status;

    // For a message requiring n fragments, the length of each of the first
    // n - 1 fragments is the maximum fragment size.  The length of the last
    // fragment is whatever is leftover.
    int8u fragmentLen = (offset + txPacket->fragmentLen < txPacket->bufLen
                         ? txPacket->fragmentLen
                         : txPacket->bufLen - offset);

    txPacket->apsFrame.groupId = HIGH_LOW_TO_INT(txPacket->fragmentCount, i);

    status = emAfPluginFragmentationSend(txPacket,
                                         i,
                                         fragmentLen,
                                         offset);
    if (status != EMBER_SUCCESS) {
      return status;
    }

    txPacket->fragmentsInTransit++;
    offset += fragmentLen;
  } // close inner for

  if (txPacket->fragmentsInTransit == 0) {
    txPacket->messageType = UNUSED_TX_PACKET_ENTRY;
    emAfFragmentationMessageSentHandler(txPacket->messageType,
                                        txPacket->indexOrDestination,
                                        &txPacket->apsFrame,
                                        txPacket->bufferPtr,
                                        txPacket->bufLen,
                                        EMBER_SUCCESS);
  } else if (freeOutgoingMessagePriorToFinalAck
             && txPacket->bufferPtr != NULL
             && offset >= txPacket->bufLen
             && emberFragmentWindowSize == 1
             && txPacketAwaitingFinalAck.messageType == UNUSED_TX_PACKET_ENTRY) {
    // Awaiting final fragment
    MEMCOPY(&txPacketAwaitingFinalAck, txPacket, sizeof(txFragmentedPacket));
    txPacketAwaitingFinalAck.bufferPtr = NULL;
    txPacketAwaitingFinalAck.bufLen = 0;
    txPacket->messageType = UNUSED_TX_PACKET_ENTRY;
  }

  return EMBER_SUCCESS;
}

static void abortTransmission(txFragmentedPacket *txPacket,
                              EmberStatus status)
{
  if (status != EMBER_SUCCESS && txPacket->messageType != UNUSED_TX_PACKET_ENTRY) {
    emAfFragmentationMessageSentHandler(txPacket->messageType,
                                        txPacket->indexOrDestination,
                                        &txPacket->apsFrame,
                                        txPacket->bufferPtr,
                                        txPacket->bufLen,
                                        status);
    txPacket->messageType = UNUSED_TX_PACKET_ENTRY;
  }
}

static txFragmentedPacket* getFreeTxPacketEntry(void)
{
  int8u i;
  for(i = 0; i < EMBER_AF_PLUGIN_FRAGMENTATION_MAX_OUTGOING_PACKETS; i++) {
    txFragmentedPacket *txPacket = &(txPackets[i]);
    if (txPacket->messageType == UNUSED_TX_PACKET_ENTRY) {
      txPacket->bufferPtr = txMessageBuffers[i];
      return txPacket;
    }
  }
  return NULL;
}

static txFragmentedPacket* txPacketLookUp(EmberApsFrame *apsFrame)
{
  int8u i;
  for(i = 0; i < EMBER_AF_PLUGIN_FRAGMENTATION_MAX_OUTGOING_PACKETS; i++) {
    txFragmentedPacket *txPacket = &(txPackets[i]);
    if (txPacket->messageType == UNUSED_TX_PACKET_ENTRY) {
      continue;
    }

    // Each node has a single source APS counter.
    if (apsFrame->sequence == txPacket->apsFrame.sequence) {
      return txPacket;
    }
  }

  if (txPacketAwaitingFinalAck.messageType != UNUSED_TX_PACKET_ENTRY
      && apsFrame->sequence == txPacketAwaitingFinalAck.apsFrame.sequence) {
    return &txPacketAwaitingFinalAck;
  }
  return NULL;
}


//------------------------------------------------------------------------------
// Receiving.

#define lowBitMask(n) ((1 << (n)) - 1)
static void setFragmentMask(rxFragmentedPacket *rxPacket);
static boolean storeRxFragment(rxFragmentedPacket *rxPacket,
                               int8u fragment,
                               int8u *buffer,
                               int16u bufLen);
static void moveRxWindow(rxFragmentedPacket *rxPacket);
static rxFragmentedPacket* getFreeRxPacketEntry(void);
static rxFragmentedPacket* rxPacketLookUp(EmberApsFrame *apsFrame,
                                          EmberNodeId sender);

static rxFragmentedPacket rxPackets[EMBER_AF_PLUGIN_FRAGMENTATION_MAX_INCOMING_PACKETS];

static void ageAllAckedRxPackets(void)
{
  int8u i;
  for(i=0; i<EMBER_AF_PLUGIN_FRAGMENTATION_MAX_INCOMING_PACKETS; i++) {
    if (rxPackets[i].status == EMBER_AF_PLUGIN_FRAGMENTATION_RX_PACKET_ACKED) {
      rxPackets[i].ackedPacketAge++;
    }
  }
}

boolean emAfFragmentationIncomingMessage(EmberApsFrame *apsFrame,
                                         EmberNodeId sender,
                                         int8u **buffer,
                                         int16u *bufLen)
{
  static boolean rxWindowMoved = FALSE;
  boolean newFragment;
  int8u fragment;
  int8u mask;
  rxFragmentedPacket *rxPacket;

  if (!(apsFrame->options & EMBER_APS_OPTION_FRAGMENT)) {
    return FALSE;
  }

  assert(*bufLen <= MAX_INT8U_VALUE);

  rxPacket = rxPacketLookUp(apsFrame, sender);
  fragment = LOW_BYTE(apsFrame->groupId);

  if (artificiallyDropBlock(fragment)) {
    artificiallyDropBlockPrintln("Artificially dropping block %d", fragment);
    clearArtificiallyDropBlock();
    return TRUE;
  }

  // First fragment for this packet, we need to set up a new entry.
  if (rxPacket == NULL) {
    rxPacket = getFreeRxPacketEntry();
    if (rxPacket == NULL || fragment >= emberFragmentWindowSize)
      return TRUE;

    rxPacket->status = EMBER_AF_PLUGIN_FRAGMENTATION_RX_PACKET_IN_USE;
    rxPacket->fragmentSource = sender;
    rxPacket->fragmentSequenceNumber = apsFrame->sequence;
    rxPacket->fragmentBase = 0;
    rxPacket->windowFinger = 0;
    rxPacket->fragmentsReceived = 0;
    rxPacket->fragmentsExpected = 0xFF;
    rxPacket->fragmentLen = (int8u)(*bufLen);
    setFragmentMask(rxPacket);

    emberEventControlSetDelayMS(*(rxPacket->fragmentEventControl),
                                (emberApsAckTimeoutMs
                                 * ZIGBEE_APSC_MAX_TRANSMIT_RETRIES));
  }

  // All fragments inside the rx window have been received and the incoming
  // fragment is outside the receiving window: let's move the rx window.
  if (rxPacket->fragmentMask == 0xFF
      && rxPacket->fragmentBase + emberFragmentWindowSize <= fragment) {
    moveRxWindow(rxPacket);
    setFragmentMask(rxPacket);
    rxWindowMoved = TRUE;

    emberEventControlSetDelayMS(*(rxPacket->fragmentEventControl),
                                (emberApsAckTimeoutMs
                                 * ZIGBEE_APSC_MAX_TRANSMIT_RETRIES));
  }

  // Fragment outside the rx window.
  if (fragment < rxPacket->fragmentBase
      || fragment >= rxPacket->fragmentBase + emberFragmentWindowSize) {
    return TRUE;
  } else { // Fragment inside the rx window.
    if (rxWindowMoved){
      // We assume that the fragment length for the new rx window is the length
      // of the first fragment received inside the window. However, if the first
      // fragment received is the last fragment of the packet, we do not
      // consider it for setting the fragment length.
      if (fragment < rxPacket->fragmentsExpected - 1) {
        rxPacket->fragmentLen = (int8u)(*bufLen);
        rxWindowMoved = FALSE;
      }
    } else {
      // We enforce that all the subsequent fragments (except for the last
      // fragment) inside the rx window have the same length as the first one.
      if (fragment < rxPacket->fragmentsExpected - 1
          && rxPacket->fragmentLen != (int8u)(*bufLen)) {
        goto kickout;
      }
    }
  }

  mask = 1 << (fragment % emberFragmentWindowSize);
  newFragment = !(mask & rxPacket->fragmentMask);

  // First fragment, setting the total number of expected fragments.
  if (fragment == 0) {
    rxPacket->fragmentsExpected = HIGH_BYTE(apsFrame->groupId);
    if (rxPacket->fragmentsExpected < emberFragmentWindowSize) {
      setFragmentMask(rxPacket);
    }
  }

  rxPacket->fragmentMask |= mask;
  if (newFragment) {
    rxPacket->fragmentsReceived++;
    if (!storeRxFragment(rxPacket, fragment, *buffer, *bufLen)) {
      goto kickout;
    }
  }

  if (fragment == rxPacket->fragmentsExpected - 1
      || (rxPacket->fragmentMask
          | lowBitMask(fragment % emberFragmentWindowSize)) == 0xFF) {
    emAfPluginFragmentationSendReply(sender,
                                     apsFrame,
                                     rxPacket);
  }

  // Received all the expected fragments.
  if (rxPacket->fragmentsReceived == rxPacket->fragmentsExpected) {
    int8u fragmentsInLastWindow =
        rxPacket->fragmentsExpected % emberFragmentWindowSize;
    if (fragmentsInLastWindow == 0)
      fragmentsInLastWindow = emberFragmentWindowSize;

    // Pass the reassembled packet only once to the application.
    if (rxPacket->status == EMBER_AF_PLUGIN_FRAGMENTATION_RX_PACKET_IN_USE) {
      //Age all acked packets first
      ageAllAckedRxPackets();
      // Mark the packet entry as acked.
      rxPacket->status = EMBER_AF_PLUGIN_FRAGMENTATION_RX_PACKET_ACKED;
      // Set the age of the new acked packet as the youngest one.
      rxPacket->ackedPacketAge = 0;
      // This library sends replies for all fragments, so, before passing on the
      // reassembled message, clear the retry bit to prevent the application
      // from sending a duplicate reply.
      apsFrame->options &= ~EMBER_APS_OPTION_RETRY;

      // The total size is the window finger + (n-1) full fragments + the last
      // fragment.
      *bufLen = rxPacket->windowFinger + rxPacket->lastfragmentLen
                 + (fragmentsInLastWindow - 1)*rxPacket->fragmentLen;
      *buffer = rxPacket->buffer;
      return FALSE;
    }
  }
  return TRUE;

kickout:
  emAfFragmentationAbortReception(rxPacket->fragmentEventControl);
  return TRUE;
}

void emAfFragmentationAbortReception(EmberEventControl *control)
{
  int8u i;
  emberEventControlSetInactive(*control);

  for(i = 0; i < EMBER_AF_PLUGIN_FRAGMENTATION_MAX_INCOMING_PACKETS; i++) {
    rxFragmentedPacket *rxPacket = &(rxPackets[i]);
    if (rxPacket->fragmentEventControl == control) {
      rxPacket->status = EMBER_AF_PLUGIN_FRAGMENTATION_RX_PACKET_AVAILABLE;
    }
  }
}

static void setFragmentMask(rxFragmentedPacket *rxPacket)
{
  // Unused bits must be 1.
  int8u highestZeroBit = emberFragmentWindowSize;
  // If we are in the final window, there may be additional unused bits.
  if (rxPacket->fragmentsExpected
      < rxPacket->fragmentBase + emberFragmentWindowSize) {
    highestZeroBit = (rxPacket->fragmentsExpected % emberFragmentWindowSize);
  }
  rxPacket->fragmentMask = ~ lowBitMask(highestZeroBit);
}

static boolean storeRxFragment(rxFragmentedPacket *rxPacket,
                               int8u fragment,
                               int8u *buffer,
                               int16u bufLen)
{
  int16u index = rxPacket->windowFinger;

  if (index + bufLen > EMBER_AF_PLUGIN_FRAGMENTATION_BUFFER_SIZE) {
    return FALSE;
  }

  index += (fragment - rxPacket->fragmentBase)*rxPacket->fragmentLen;
  MEMCOPY(rxPacket->buffer + index, buffer, bufLen);

  // If this is the last fragment of the packet, store its length.
  if (fragment == rxPacket->fragmentsExpected - 1) {
    rxPacket->lastfragmentLen = (int8u)bufLen;
  }

  return TRUE;
}

static void moveRxWindow(rxFragmentedPacket *rxPacket)
{
  rxPacket->fragmentBase += emberFragmentWindowSize;
  rxPacket->windowFinger += emberFragmentWindowSize*rxPacket->fragmentLen;
}

static rxFragmentedPacket* getFreeRxPacketEntry(void)
{
  int8u i;
  rxFragmentedPacket* ackedPacket = NULL;

  // Available entries first.
  for(i = 0; i < EMBER_AF_PLUGIN_FRAGMENTATION_MAX_INCOMING_PACKETS; i++) {
    rxFragmentedPacket *rxPacket = &(rxPackets[i]);
    if (rxPacket->status == EMBER_AF_PLUGIN_FRAGMENTATION_RX_PACKET_AVAILABLE) {
      return rxPacket;
    }
  }

  // Acked packets: Look for the oldest one.
  for(i = 0; i < EMBER_AF_PLUGIN_FRAGMENTATION_MAX_INCOMING_PACKETS; i++) {
    rxFragmentedPacket *rxPacket = &(rxPackets[i]);
    if (rxPacket->status == EMBER_AF_PLUGIN_FRAGMENTATION_RX_PACKET_ACKED) {
      if (ackedPacket == NULL
          || ackedPacket->ackedPacketAge < rxPacket->ackedPacketAge) {
        ackedPacket = rxPacket;
      }
    }
  }

  return ackedPacket;
}

static rxFragmentedPacket* rxPacketLookUp(EmberApsFrame *apsFrame,
                                          EmberNodeId sender)
{
  int8u i;
  for(i = 0; i < EMBER_AF_PLUGIN_FRAGMENTATION_MAX_INCOMING_PACKETS; i++) {
    rxFragmentedPacket *rxPacket = &(rxPackets[i]);
    if (rxPacket->status == EMBER_AF_PLUGIN_FRAGMENTATION_RX_PACKET_AVAILABLE) {
      continue;
    }
    // Each packet is univocally identified by the pair (node id, seq. number).
    if (apsFrame->sequence == rxPacket->fragmentSequenceNumber
        && sender == rxPacket->fragmentSource) {
      return rxPacket;
    }
  }
  return NULL;
}

//------------------------------------------------------------------------------
// Initialization
void emberAfPluginFragmentationInitCallback(void)
{
  int8u i;
  emAfPluginFragmentationPlatformInitCallback();

  for(i = 0; i < EMBER_AF_PLUGIN_FRAGMENTATION_MAX_INCOMING_PACKETS; i++) {
    rxPackets[i].status = EMBER_AF_PLUGIN_FRAGMENTATION_RX_PACKET_AVAILABLE;
    rxPackets[i].fragmentEventControl = &(emAfFragmentationEvents[i]);
  }

  for(i = 0; i < EMBER_AF_PLUGIN_FRAGMENTATION_MAX_OUTGOING_PACKETS; i++) {
    txPackets[i].messageType = 0xFF;
  }
}

