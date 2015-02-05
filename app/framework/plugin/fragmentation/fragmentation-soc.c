// *****************************************************************************
// * fragmentation-soc.c
// *
// * System on chip specific code for fragmentation.
// *
// * Copyright 2013 by Silicon Labs.      All rights reserved.              *80*
// *****************************************************************************

#include "app/framework/include/af.h"
#include "app/framework/util/af-main.h"
#include "fragmentation.h"

//-----------------------------------------------------------------------------
// Globals

//-----------------------------------------------------------------------------
// Functions

void emAfPluginFragmentationPlatformInitCallback(void)
{
   emberFragmentWindowSize = EMBER_AF_PLUGIN_FRAGMENTATION_RX_WINDOW_SIZE;
}

EmberStatus emAfPluginFragmentationSend(txFragmentedPacket* txPacket,
                                        int8u fragmentNumber,
                                        int16u fragmentLen,
                                        int16u offset)
{
  EmberStatus status;
  EmberMessageBuffer message;
  message = emberFillLinkedBuffers(txPacket->bufferPtr + offset,
                                   fragmentLen);
  if (message == EMBER_NULL_MESSAGE_BUFFER) {
    return EMBER_NO_BUFFERS;
  }
  status = emberSendUnicast(txPacket->messageType,
                            txPacket->indexOrDestination,
                            &txPacket->apsFrame,
                            message);

  emberReleaseMessageBuffer(message);
  return status;  
}

void emAfPluginFragmentationHandleSourceRoute(txFragmentedPacket* txPacket,
                                              int16u indexOrDestination)
{
  // Nothing to do on SOC.  This will be handled later by the NWK code
  // calling into the application to ask for a source route.
}

void emAfPluginFragmentationSendReply(EmberNodeId sender,
                                      EmberApsFrame* apsFrame,
                                      rxFragmentedPacket* rxPacket)
{
  emberSetReplyFragmentData(HIGH_LOW_TO_INT(rxPacket->fragmentMask,
                                            rxPacket->fragmentBase));
  emberSendReply(apsFrame->clusterId, EMBER_NULL_MESSAGE_BUFFER);
}