// *******************************************************************
// * poll-control-client-cli.c
// *
// *
// * Copyright 2013 Silicon Laboratories, Inc.                              *80*
// *******************************************************************

#include "app/framework/include/af.h"
#include "app/util/serial/command-interpreter2.h"
#include "app/framework/plugin/poll-control-client/poll-control-client.h"

void mode(void);
void timeout(void);
void respond(void);
void print(void);

#ifndef EMBER_AF_GENERATE_CLI
EmberCommandEntry emberAfPluginPollControlClientCommands[] = {
  emberCommandEntryAction("mode",  mode, "u", ""),
  emberCommandEntryAction("timeout", timeout, "v", ""),
  emberCommandEntryAction("respond",  respond, "u", ""),
  emberCommandEntryAction("print", print, "", ""),
  emberCommandEntryTerminator(),
};
#endif // EMBER_AF_GENERATE_CLI

// plugin poll-control-client mode <mode:1>
void mode(void)
{
  int8u mode = (int8u)emberUnsignedCommandArgument(0);
  emAfSetFastPollingMode(mode);
  emberAfPollControlClusterPrintln("%p 0x%x", "mode", mode);
}

// plugin poll-control-client timeout <timeout:2>
void timeout(void)
{
  int16u timeout = (int16u)emberUnsignedCommandArgument(0);
  emAfSetFastPollingTimeout(timeout);
  emberAfPollControlClusterPrintln("%p 0x%2x", "timeout", timeout);
}

// plugin poll-control-client respond <mode:1>
void respond(void)
{
  int8u mode = (int8u)emberUnsignedCommandArgument(0);
  emAfSetResponseMode(mode);
  emberAfPollControlClusterPrintln("%p 0x%x", "respond", mode);
}

void print(void)
{
  emAfPollControlClientPrint();
}
