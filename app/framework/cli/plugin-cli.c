// *******************************************************************
// * plugin-cli.c
// *
// *
// * Copyright 2010 by Ember Corporation. All rights reserved.              *80*
// *******************************************************************

#include "app/framework/include/af.h"
#include "app/util/serial/command-interpreter2.h"
#include "plugin-cli.h"

#ifndef EMBER_AF_GENERATE_CLI

#ifdef EMBER_AF_GENERATED_PLUGIN_COMMAND_ENTRIES
EmberCommandEntry emberAfPluginCommands[] = {
  EMBER_AF_GENERATED_PLUGIN_COMMAND_ENTRIES
  emberCommandEntryTerminator(),
};
#endif

#endif // EMBER_AF_GENERATE_CLI
