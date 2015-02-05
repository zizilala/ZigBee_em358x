// * standalone-bootloader-client-cli.c
// *
// * This file defines the standalone bootloader client CLI.
// * 
// * Copyright 2012 Silicon Laboratories, Inc.                              *80*
// *****************************************************************************

#include "app/framework/include/af.h"
#include "app/framework/plugin/standalone-bootloader-common/bootloader-protocol.h"
#include "standalone-bootloader-client.h"

#include "app/util/serial/command-interpreter2.h"

//------------------------------------------------------------------------------
// Forward declarations



//------------------------------------------------------------------------------
// Globals

#ifndef EMBER_AF_GENERATE_CLI
EmberCommandEntry emberAfPluginStandaloneBootloaderClientCommands[] = {
  emberCommandEntryAction("status", emAfStandaloneBootloaderClientPrintStatus, "",
                          "Prints the current status of the client"),
  emberCommandEntryTerminator(),
};
#endif // EMBER_AF_GENERATE_CLI
//------------------------------------------------------------------------------
// Functions

