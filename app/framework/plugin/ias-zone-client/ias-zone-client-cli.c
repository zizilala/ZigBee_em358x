
#include "af.h"
#include "app/util/serial/command-interpreter2.h"
#include "ias-zone-client.h"

//-----------------------------------------------------------------------------
// Globals

static void printServersCommand(void);
static void clearAllServersCommand(void);

#ifndef EMBER_AF_GENERATE_CLI
EmberCommandEntry emberAfPluginIasZoneClientCommands[] = {
  emberCommandEntryAction("print-servers", printServersCommand, "",    "Print the known IAS Zone Servers"),
  emberCommandEntryAction("clear-all",     clearAllServersCommand, "", "Clear all known IAS Zone Servers from local device"),
  emberCommandEntryTerminator(),
};
#endif // EMBER_AF_GENERATE_CLI

//-----------------------------------------------------------------------------
// Functions

static void printServersCommand(void)
{
  int8u i;
  emberAfIasZoneClusterPrintln("Index IEEE                 EP   Type   Status State");
  emberAfIasZoneClusterPrintln("---------------------------------------------------");
  for (i = 0; i < EMBER_AF_PLUGIN_IAS_ZONE_CLIENT_MAX_DEVICES; i++) {
    if (i < 10) {
      emberAfIasZoneClusterPrint(" ");
    }
    emberAfIasZoneClusterPrint("%d    (>)%X%X%X%X%X%X%X%X  ", 
                                 i,
                                 emberAfIasZoneClientKnownServers[i].ieeeAddress[7],
                                 emberAfIasZoneClientKnownServers[i].ieeeAddress[6],
                                 emberAfIasZoneClientKnownServers[i].ieeeAddress[5],
                                 emberAfIasZoneClientKnownServers[i].ieeeAddress[4],
                                 emberAfIasZoneClientKnownServers[i].ieeeAddress[3],
                                 emberAfIasZoneClientKnownServers[i].ieeeAddress[2],
                                 emberAfIasZoneClientKnownServers[i].ieeeAddress[1],
                                 emberAfIasZoneClientKnownServers[i].ieeeAddress[0]);
    if (emberAfIasZoneClientKnownServers[i].endpoint < 10) {
      emberAfIasZoneClusterPrint(" ");
    }    
    if (emberAfIasZoneClientKnownServers[i].endpoint < 100) {
      emberAfIasZoneClusterPrint(" ");
    }
    emberAfIasZoneClusterPrint("%d  ", emberAfIasZoneClientKnownServers[i].endpoint);
    emberAfIasZoneClusterPrintln("0x%2X 0x%2X 0x%X", 
                                 emberAfIasZoneClientKnownServers[i].zoneType, 
                                 emberAfIasZoneClientKnownServers[i].zoneStatus, 
                                 emberAfIasZoneClientKnownServers[i].zoneState); 
  }
}

static void clearAllServersCommand(void)
{
  emAfClearServers();
}

static void queryDeviceCommand(void)
{

}
