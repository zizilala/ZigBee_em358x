// *******************************************************************
// * tunneling-client-cli.c
// *
// *
// * Copyright 2011 by Ember Corporation. All rights reserved.              *80*
// *******************************************************************

#include "app/framework/include/af.h"
#include "app/util/serial/command-interpreter2.h"
#include "tunneling-client.h"


#if !defined(EMBER_AF_GENERATE_CLI)
void emAfPluginTunnelingClientCliRequest(void);
void emAfPluginTunnelingClientCliTransfer(void);
void emAfPluginTunnelingClientCliClose(void);
void emAfPluginTunnelingClientCliPrint(void);

EmberCommandEntry emberAfPluginTunnelingClientCommands[] = {
  emberCommandEntryAction("request",  emAfPluginTunnelingClientCliRequest,                        "vuuuvu", 
                          "Send a tunnel request command"),
  emberCommandEntryAction("transfer", emAfPluginTunnelingClientCliTransfer,                       "ub",     
                          "Transfer data through a previously setup tunnel"),
  emberCommandEntryAction("close",    emAfPluginTunnelingClientCliClose,                          "u",      
                          "Close a tunnel"),
  emberCommandEntryAction("print",    emAfPluginTunnelingClientCliPrint, "",
                          "Print the list of tunnels"),
  emberCommandEntryTerminator(),
};
#endif // EMBER_AF_GENERATE_CLI

// plugin tunneling-client request <server:2> <client endpoint:1> <server endpoint:1> <protocol id:1> <manufacturer code:2> <flow control support:1>
void emAfPluginTunnelingClientCliRequest(void)
{
  EmberNodeId server = (EmberNodeId)emberUnsignedCommandArgument(0);
  int8u clientEndpoint = (int8u)emberUnsignedCommandArgument(1);
  int8u serverEndpoint = (int8u)emberUnsignedCommandArgument(2);
  int8u protocolId = (int8u)emberUnsignedCommandArgument(3);
  int16u manufacturerCode = (int16u)emberUnsignedCommandArgument(4);
  boolean flowControlSupport = (boolean)emberUnsignedCommandArgument(5);
  EmberAfPluginTunnelingClientStatus status = emberAfPluginTunnelingClientRequestTunnel(server,
                                                                                        clientEndpoint,
                                                                                        serverEndpoint,
                                                                                        protocolId,
                                                                                        manufacturerCode,
                                                                                        flowControlSupport);
  emberAfTunnelingClusterPrintln("%p 0x%x", "request", status);
}

// plugin tunneling-client transfer <tunnel index:1> <data>
void emAfPluginTunnelingClientCliTransfer(void)
{
  int8u tunnelIndex = (int8u)emberUnsignedCommandArgument(0);
  int8u data[255];
  int16u dataLen = emberCopyStringArgument(1, data, sizeof(data), FALSE);
  EmberAfStatus status = emberAfPluginTunnelingClientTransferData(tunnelIndex,
                                                                  data,
                                                                  dataLen);
  emberAfTunnelingClusterPrintln("%p 0x%x", "transfer", status);
}

// plugin tunneling-client close <tunnel index:1>
void emAfPluginTunnelingClientCliClose(void)
{
  int8u tunnelIndex = (int8u)emberUnsignedCommandArgument(0);
  EmberAfStatus status = emberAfPluginTunnelingClientCloseTunnel(tunnelIndex);
  emberAfTunnelingClusterPrintln("%p 0x%x", "close", status);
}
