// *******************************************************************
// * identify-cli.c
// *
// *
// * Copyright 2012 by Ember Corporation. All rights reserved.              *80*
// *******************************************************************

#include "app/framework/include/af.h"
#include "app/util/serial/command-interpreter2.h"

void emAfPluginIdentifyCliPrint(void);

#if !defined(EMBER_AF_GENERATE_CLI)
EmberCommandEntry emberAfPluginIdentifyCommands[] = {
  emberCommandEntryAction("print",  emAfPluginIdentifyCliPrint, "", "Print the identify state of each endpoint"),
  emberCommandEntryTerminator(),
};
#endif // EMBER_AF_GENERATE_CLI

// plugin identify print
void emAfPluginIdentifyCliPrint(void) 
{
#if defined(EMBER_AF_PRINT_ENABLE) && defined(EMBER_AF_PRINT_IDENTIFY_CLUSTER)
  int8u i;
  for (i = 0; i < emberAfEndpointCount(); ++i) {
    int8u endpoint = emberAfEndpointFromIndex(i);
    emberAfIdentifyClusterPrintln("Endpoint 0x%x is identifying: %p",
                                  endpoint,
                                  (emberAfIsDeviceIdentifying(endpoint)
                                   ? "TRUE"
                                   : "FALSE"));
  }
#endif //defined(EMBER_AF_PRINT_ENABLE) && defined(EMBER_AF_PRINT_IDENTIFY_CLUSTER)
}
