// File: gateway-support-cli.c
//
// Description:  Gateway specific CLI behavior for a host application.
//   In this case we assume our application is running on
//   a PC with Unix library support, connected to an NCP via serial uart.
//
// Copyright 2014 Silicon Laboratories, Inc.                                *80*
//
//------------------------------------------------------------------------------

// common include file
#include "app/framework/util/common.h"

#include <time.h>

//------------------------------------------------------------------------------
// Forward Declarations

void emberAfPluginGatewaySupportTimeSyncLocal(void);

//------------------------------------------------------------------------------
// Globals

// The difference in seconds between the ZigBee Epoch: January 1st, 2000
// and the Unix Epoch: January 1st 1970.
#define UNIX_ZIGBEE_EPOCH_DELTA (int32u)946684800UL

#if !defined(EMBER_AF_GENERATE_CLI)

EmberCommandEntry emberAfPluginGatewayCommands[] = {
  emberCommandEntryAction("time-sync-local", 
                          emberAfPluginGatewaySupportTimeSyncLocal, 
                          "",
                          "This command retrieves the local unix time and syncs the Time Server attribute to it."),
  emberCommandEntryTerminator(),
};

#endif

//------------------------------------------------------------------------------
// Functions

void emberAfPluginGatewaySupportTimeSyncLocal(void)
{
  time_t unixTime = time(NULL);
  unixTime -= UNIX_ZIGBEE_EPOCH_DELTA;
  emberAfSetTime(unixTime);
  emberAfPrintTime(emberAfGetCurrentTime());
}
