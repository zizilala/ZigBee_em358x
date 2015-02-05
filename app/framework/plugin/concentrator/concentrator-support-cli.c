// *****************************************************************************
// * concentrator-support-cli.c
// *
// * CLI interface to manage the concentrator's periodic MTORR broadcasts.
// *
// * Copyright 2012 by Ember Corporation. All rights reserved.              *80*
// *****************************************************************************

#include "app/framework/include/af.h"
#include "app/util/serial/command-interpreter2.h"
#include "app/framework/plugin/concentrator/concentrator-support.h"
#include "app/util/source-route-common.h"

// *****************************************************************************
// Forward Declarations

void emberAfPluginConcentratorStatus(void);
void emberAfPluginConcentratorStartDiscovery(void);
void emberAfPluginConcentratorAggregationCommand(void);
void emberAfPluginConcentratorPrintSourceRouteTable(void);
void emberAfGetSourceRoute(void);
// *****************************************************************************
// Globals
 
#if !defined(EMBER_AF_GENERATE_CLI)
EmberCommandEntry emberAfPluginConcentratorCommands[] = {
  emberCommandEntryAction("status", emberAfPluginConcentratorStatus, "",
                          "Prints current status and configured parameters of the concentrator"),
  emberCommandEntryAction("start",  emberAfPluginConcentratorStartDiscovery, "",
                          "Starts the periodic broadcast of MTORRs"),
  emberCommandEntryAction("stop",   emberAfPluginConcentratorStopDiscovery, "",
                          "Stops the periodic broadcast of MTORRs"),
  emberCommandEntryAction("agg", emberAfPluginConcentratorAggregationCommand, "", 
                          "Sends an MTORR broadcast now."),
  emberCommandEntryAction("print-table", 
                          emberAfPluginConcentratorPrintSourceRouteTable, 
                          "",
                          "Prints the source route table."),
  emberCommandEntryTerminator(),
};
#endif // EMBER_AF_GENERATE_CLI

// *****************************************************************************
// Functions

void emberAfPluginConcentratorPrintSourceRouteTable(void)
{
  int8u i;
  // emberAfAppPrintln("Source Route Table\n");
  // emberAfAppPrintln(  "index dest    closer  older");
  // for (i = 0; i < sourceRouteTableSize; i++) {
  //   emberAfAppPrintln("%d:    0x%2X  %d      %d",
  //                     i,
  //                     sourceRouteTable[i].destination,
  //                     sourceRouteTable[i].closerIndex,
  //                     sourceRouteTable[i].olderIndex);
  // }
  emberAfAppPrintln("Source Routes");

  for (i = 0; i < sourceRouteGetCount(); i++) {
    int8u closerIndex = sourceRouteTable[i].closerIndex;
    emberAfAppPrint("%d: 0x%2X -> ", 
                    i,
                    sourceRouteTable[i].destination);

    while (closerIndex != NULL_INDEX) {
      emberAfAppPrint("0x%2X -> ", sourceRouteTable[closerIndex].destination);
      closerIndex = sourceRouteTable[closerIndex].closerIndex;
    }
    emberAfAppPrintln("0x%2X (Me)", emberAfGetNodeId());
  }
  emberAfAppPrintln("%d of %d total entries.",
                    sourceRouteGetCount(),
                    sourceRouteTableSize);
}

void emberAfPluginConcentratorStatus(void)
{
  boolean active = (emberAfPluginConcentratorUpdateEventControl.status
                    != EMBER_EVENT_INACTIVE);
  int32u nowMS32 = halCommonGetInt32uMillisecondTick();

  emberAfAppPrintln("Active: %p", 
                    (active
                     ? "yes"
                     : "no"));
  emberAfAppPrintln("Type:  %p RAM",
                    ((EMBER_AF_PLUGIN_CONCENTRATOR_CONCENTRATOR_TYPE
                      == EMBER_LOW_RAM_CONCENTRATOR)
                     ? "Low"
                     : "High"));
  emberAfAppPrintln("Time before next broadcast (ms):   %l",
                    emberAfPluginConcentratorUpdateEventControl.timeToExecute - nowMS32);
  emberAfAppPrintln("Min Time Between Broadcasts (sec): %d", 
                    EMBER_AF_PLUGIN_CONCENTRATOR_MIN_TIME_BETWEEN_BROADCASTS_SECONDS);
  emberAfAppPrintln("Max Time Between Broadcasts (sec): %d", 
                    EMBER_AF_PLUGIN_CONCENTRATOR_MAX_TIME_BETWEEN_BROADCASTS_SECONDS);
  emberAfAppPrintln("Max Hops: %d",
                    (EMBER_AF_PLUGIN_CONCENTRATOR_MAX_HOPS == 0
                     ? EMBER_MAX_HOPS
                     : EMBER_AF_PLUGIN_CONCENTRATOR_MAX_HOPS));
  emberAfAppPrintln("Route Error Threshold:      %d",
                    EMBER_AF_PLUGIN_CONCENTRATOR_ROUTE_ERROR_THRESHOLD);
  emberAfAppPrintln("Delivery Failure Threshold: %d",
                    EMBER_AF_PLUGIN_CONCENTRATOR_DELIVERY_FAILURE_THRESHOLD);
}

void emberAfPluginConcentratorStartDiscovery(void)
{
  int32u qsLeft = emberAfPluginConcentratorQueueDiscovery();
  emberAfAppPrintln("%d sec until next MTORR broadcast", (qsLeft >> 2));
}

void emberAfPluginConcentratorAggregationCommand(void)
{
  emberAfPluginConcentratorQueueDiscovery();
}
