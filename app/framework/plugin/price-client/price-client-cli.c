// *******************************************************************
// * price-client-cli.c
// *
// *
// * Copyright 2012 by Ember Corporation. All rights reserved.              *80*
// *******************************************************************

#include "app/framework/include/af.h"
#include "app/util/serial/command-interpreter2.h"
#include "app/framework/plugin/price-client/price-client.h"

void emAfPriceClientCliPrint(void);

#if !defined(EMBER_AF_GENERATE_CLI)
EmberCommandEntry emberAfPluginPriceClientCommands[] = {
  emberCommandEntryAction("print",  emAfPriceClientCliPrint, "u", "Print the price info"),
  emberCommandEntryTerminator(),
};
#endif // EMBER_AF_GENERATE_CLI

// plugin price-client print <endpoint:1>
void emAfPriceClientCliPrint(void) 
{
  int8u endpoint = (int8u)emberUnsignedCommandArgument(0);
  emAfPluginPriceClientPrintInfo(endpoint);
}
