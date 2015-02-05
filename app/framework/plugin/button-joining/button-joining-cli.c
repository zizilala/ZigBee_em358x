// *****************************************************************************
// * button-joining-cli.c
// *
// * CLI commands for forming/joining using the hardware buttons.
// *
// * Copyright 2013 by Ember Corporation. All rights reserved.              *80*
// *****************************************************************************

#include "app/framework/include/af.h"
#include "app/util/serial/command-interpreter2.h"

void emberAfPluginButtonJoiningButton0Command(void);
void emberAfPluginButtonJoiningButton1Command(void);

extern void emberAfPluginButtonJoiningPressButton(int8u button);

#if !defined(EMBER_AF_GENERATE_CLI)
EmberCommandEntry emberAfPluginButtonJoiningCommands[] = {
  emberCommandEntryAction("button0",
                          emberAfPluginButtonJoiningButton0Command, 
                          "",
                          "Press button 0"),
  emberCommandEntryAction("button1",
                          emberAfPluginButtonJoiningButton1Command, 
                          "",
                          "Press button 1"),
  emberCommandEntryTerminator(),
};
#endif // EMBER_AF_GENERATE_CLI

void emberAfPluginButtonJoiningButton0Command(void)
{
  emberAfPluginButtonJoiningPressButton(BUTTON0);
}

void emberAfPluginButtonJoiningButton1Command(void)
{
  emberAfPluginButtonJoiningPressButton(BUTTON1);
}
