// *****************************************************************************
// * fragmentation-host.c
// *
// * Host specific code for fragmentation.
// *
// * Copyright 2013 by Silicon Labs.      All rights reserved.              *80*
// *****************************************************************************

#include "app/framework/include/af.h"
#include "app/framework/util/af-main.h"
#include "app/util/source-route-host.h"
#include "fragmentation.h"

//-----------------------------------------------------------------------------
// Globals

int16u emberApsAckTimeoutMs    = 0;
int8u  emberFragmentWindowSize = EMBER_AF_PLUGIN_FRAGMENTATION_RX_WINDOW_SIZE;

//-----------------------------------------------------------------------------
// Functions

void emberAfPluginFragmentationNcpInitCallback(void)
{
  ezspGetConfigurationValue(EZSP_CONFIG_APS_ACK_TIMEOUT, &emberApsAckTimeoutMs);
  emberAfSetEzspConfigValue(EZSP_CONFIG_FRAGMENT_WINDOW_SIZE,
                            emberFragmentWindowSize,
                            "Fragmentation RX window size");
}

void emAfPluginFragmentationPlatformInitCallback(void)
{

}

EmberStatus emAfPluginFragmentationSend(txFragmentedPacket* txPacket,
                                        int8u fragmentNumber,
                                        int16u fragmentLen,
                                        int16u offset)
{
  EmberStatus status;

#ifdef EZSP_APPLICATION_HAS_ROUTE_RECORD_HANDLER
  if (txPacket->sourceRoute) {
    status = ezspSetSourceRoute(txPacket->indexOrDestination,
                                txPacket->relayCount,
                                txPacket->relayList);
    if (status != EMBER_SUCCESS) {
      return status;
    }
  }
#endif //EZSP_APPLICATION_HAS_ROUTE_RECORD_HANDLER

  status = ezspSendUnicast(txPacket->messageType,
                           txPacket->indexOrDestination,
                           &(txPacket->apsFrame),
                           fragmentNumber,
                           fragmentLen,
                           txPacket->bufferPtr + offset,
                           &(txPacket->apsFrame.sequence));
  return status;
}


void emAfPluginFragmentationHandleSourceRoute(txFragmentedPacket* txPacket,
                                              int16u indexOrDestination)
{
#if defined(EZSP_APPLICATION_HAS_ROUTE_RECORD_HANDLER)
  txPacket->sourceRoute = emberFindSourceRoute(indexOrDestination,
                                               &txPacket->relayCount,
                                               txPacket->relayList);
#endif
}

void emAfPluginFragmentationSendReply(EmberNodeId sender,
                                      EmberApsFrame* apsFrame,
                                      rxFragmentedPacket* rxPacket)
{
  apsFrame->groupId = HIGH_LOW_TO_INT(rxPacket->fragmentMask, rxPacket->fragmentBase);
  ezspSendReply(sender, apsFrame, 0, NULL);
}