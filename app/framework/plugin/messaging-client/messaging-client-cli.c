// *******************************************************************
// * messaging-client-cli.c
// *
// *
// * Copyright 2012 by Ember Corporation. All rights reserved.              *80*
// *******************************************************************

#include "app/framework/include/af.h"
#include "app/util/serial/command-interpreter2.h"
#include "app/framework/plugin/messaging-client/messaging-client.h"

void emAfMessagingClientCliConfirm(void);
void emAfMessagingClientCliPrint(void);

#if !defined(EMBER_AF_GENERATE_CLI)
EmberCommandEntry emberAfPluginMessagingClientCommands[] = {
  emberCommandEntryAction("confirm",  emAfMessagingClientCliConfirm, "u", ""),
  emberCommandEntryAction("print", emAfMessagingClientCliPrint, "u", ""),
  emberCommandEntryTerminator(),
};
#endif // EMBER_AF_GENERATE_CLI

// plugin messaging-client confirm <endpoint:1>
void emAfMessagingClientCliConfirm(void)
{
  int8u endpoint = (int8u)emberUnsignedCommandArgument(0);
  EmberAfStatus status = emberAfPluginMessagingClientConfirmMessage(endpoint);
  emberAfMessagingClusterPrintln("%p 0x%x", "confirm", status);
}

// plugin messaging-client print <endpoint:1>
void emAfMessagingClientCliPrint(void)
{
  int8u endpoint = (int8u)emberUnsignedCommandArgument(0);
  emAfPluginMessagingClientPrintInfo(endpoint);
}
