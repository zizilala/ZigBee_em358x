// *******************************************************************
// * drlc-cli.c
// *
// *
// * Copyright 2012 by Ember Corporation. All rights reserved.              *80*
// *******************************************************************

#include "app/framework/include/af.h"
#include "app/util/serial/command-interpreter2.h"
#include "app/framework/plugin/drlc/demand-response-load-control.h"
#include "app/framework/plugin/drlc/load-control-event-table.h"

#if !defined(EMBER_AF_GENERATE_CLI)

void emberAfPluginDrlcOptCommand(void);
void emberAfPluginDrlcPrintCommand(void);
void emberAfPluginDrlcClearCommand(void);

EmberCommandEntry emberAfPluginDrlcOptCommands[] = {
  emberCommandEntryAction("in", emberAfPluginDrlcOptCommand, "uw", ""),
  emberCommandEntryAction("out", emberAfPluginDrlcOptCommand, "uw", ""),
  emberCommandEntryTerminator(),
};

EmberCommandEntry emberAfPluginDrlcCommands[] = {
  emberCommandEntryAction("opt", NULL, (PGM_P)emberAfPluginDrlcOptCommands, ""),
  emberCommandEntryAction("print", emberAfPluginDrlcPrintCommand, "u", ""),
  emberCommandEntryAction("clear", emberAfPluginDrlcClearCommand, "u", ""),
  emberCommandEntryTerminator(),
};
#endif // EMBER_AF_GENERATE_CLI

// plugin drlc opt <in | out> <endpoint:1> <id:4>
void emberAfPluginDrlcOptCommand(void)
{
  emAfLoadControlEventOptInOrOut((int8u)emberUnsignedCommandArgument(0),
                                 emberUnsignedCommandArgument(1),
                                 emberCurrentCommand->name[0] == 'i');
}

// plugin drlc print <endpoint:1>
void emberAfPluginDrlcPrintCommand(void) 
{
  emAfLoadControlEventTablePrint((int8u)emberUnsignedCommandArgument(0));
}

// plugin drlc clear <endpoint:1>
void emberAfPluginDrlcClearCommand(void)
{
  emAfLoadControlEventTableClear((int8u)emberUnsignedCommandArgument(0));

  emberAfCorePrintln("Events cleared");
}
