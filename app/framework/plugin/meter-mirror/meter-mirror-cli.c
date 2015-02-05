// *****************************************************************************
// * meter-mirror-cli.c
// *
// * Code to manage the mirrors via the CLI.
// * 
// * Copyright 2013 by Silicon Labs. All rights reserved.                   *80*
// *****************************************************************************

#include "app/framework/include/af.h"
#include "app/framework/util/util.h"
#include "meter-mirror.h"

//-----------------------------------------------------------------------------
// Globals

void emAfPluginMeterMirrorStatusCommand(void)
{
  int8u meteringDeviceTypeAttribute = 0xFF;
  int16u endpoint;
  int8u mirrorEui[EUI64_SIZE];

  emberAfReportingPrintln("Known meters");
    // Go over our endpoints and find an unallocated meter mirror
  for (endpoint = EMBER_AF_PLUGIN_METER_MIRROR_ENDPOINT_START;
       endpoint < EMBER_AF_PLUGIN_METER_MIRROR_ENDPOINT_START + EMBER_AF_PLUGIN_METER_MIRROR_MAX_MIRRORS;
       endpoint++) {
    boolean validEndpoint;

    // Reset these every run 
    MEMSET(mirrorEui, 0xFF, EUI64_SIZE);
    meteringDeviceTypeAttribute = 0xFF;
    validEndpoint = emberAfPluginMeterMirrorGetEui64ByEndpoint(endpoint,
                                                               mirrorEui);
    
    if (!validEndpoint) {
      continue;
    }
    if (emberAfPluginMeterMirrorIsMirrorUsed(endpoint)) {
      // Retrieve the meter device type, this will tell us what 
      // type of meter we are looking at on this endpoint
      emberAfReadServerAttribute(endpoint,
                                 ZCL_SIMPLE_METERING_CLUSTER_ID,
                                 ZCL_METERING_DEVICE_TYPE_ATTRIBUTE_ID,
                                 &meteringDeviceTypeAttribute,
                                 sizeof(meteringDeviceTypeAttribute));
    }
    
    emberAfReportingPrint("ep: %d  ieee:", endpoint);
    emberAfPrintBigEndianEui64(mirrorEui);
    emberAfReportingPrintln("  type: 0x%X", meteringDeviceTypeAttribute);
  }
  // Explicitly usea  function call to find the number of mirrors allocated instead
  // of just calculating it.  In case there is ever a discrepancy,
  // this will help track down any bugs.
  emberAfReportingPrintln("%d of %d mirrors used.",
                          emAfPluginMeterMirrorGetMirrorsAllocated(),
                          EMBER_AF_PLUGIN_METER_MIRROR_MAX_MIRRORS);
}

void emAfPluginMeterMirrorRemoveCommand(void)
{
  int8u endpoint = (int8u)emberUnsignedCommandArgument(0);
  EmberEUI64 mirrorEui64;
  boolean success = emberAfPluginMeterMirrorGetEui64ByEndpoint(endpoint, mirrorEui64);
  if (!success) {
    emberAfReportingPrintln("Error: Failed to obtain EUI64 for endpoint %d", endpoint);
    return;
  }
  emberAfPluginSimpleMeteringClientRemoveMirrorCallback(mirrorEui64);
}