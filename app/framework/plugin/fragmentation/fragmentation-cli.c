// *****************************************************************************
// * fragmentation-cli.c
// *
// * CLI commands related to the fragmentation code.
// *
// * Copyright 2013 by Silicon Labs. All rights reserved.                   *80*
// *****************************************************************************

#include "app/framework/include/af.h"
#include "fragmentation.h"

//------------------------------------------------------------------------------
// CLI stuff

void emAfPluginFragmentationSetWindowSizeCommand(void);

#ifdef EZSP_HOST
void emAfResetAndInitNCP(void);
#endif //EZSP_HOST

//------------------------------------------------------------------------------
// Globals

#if !defined(EMBER_AF_GENERATE_CLI)

EmberCommandEntry emberAfPluginFragmentationCommands[] = {
  // This is commented out for everything but simulation due to the fact
  // it is non-standard.  Smart Energy defines the window size of 1 as
  // the only supported configuration.
#ifdef EMBER_TEST
  emberCommandEntryAction("set-rx-window-size",      
                          emAfPluginFragmentationSetWindowSizeCommand, 
                          "u", 
                          "Sets the window size for fragmented transmissions."),
  emberCommandEntryAction("artificial-block-drop",
                          emAfPluginFragmentationArtificialBlockDropCommand,
                          "Sets up the plugin to artificially drop a block"),

#endif //EMBER_TEST
  emberCommandEntryTerminator(),
};

#endif

//------------------------------------------------------------------------------
// Functions

void emAfPluginFragmentationSetWindowSizeCommand(void)
{
  emberFragmentWindowSize = (int8u)emberUnsignedCommandArgument(0);
  emberAfAppPrintln("Fragmentation RX window size set to %d",
                    emberFragmentWindowSize);

#ifdef EZSP_HOST
  emAfResetAndInitNCP();
#endif //EZSP_HOST
}

void emAfPluginFragmentationArtificialBlockDropCommand(void)
{
#if defined(EMBER_TEST)
   emAfPluginFragmentationArtificiallyDropBlockNumber = emberUnsignedCommandArgument(0);
#endif
}
