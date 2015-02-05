#include "af.h"
#include "app/util/serial/command-interpreter2.h"
#include "ias-zone-server.h"

//-----------------------------------------------------------------------------
// Globals

static void infoCommand(void);
static void changeStatusCommand(void);

static const char* changeStatusArguments[] = {
  "new-status",
  "time-since-occurred-seconds",
  NULL,
};

#ifndef EMBER_AF_GENERATE_CLI
EmberCommandEntry emberAfPluginIasZoneServerCommands[] = {
  emberCommandEntryAction("info", infoCommand, "", "Print IAS Zone information"),
  emberCommandEntryActionWithDetails("change-status", 
                                     changeStatusCommand, 
                                     "vu", 
                                     "Change the current Zone Status",
                                     changeStatusArguments),
  emberCommandEntryTerminator(),
};
#endif // EMBER_AF_GENERATE_CLI

typedef struct {
  int16u zoneType;
  const char* zoneTypeString;
} ZoneTypeToStringMap;

static ZoneTypeToStringMap zoneTypeToStringMap[] = {
  { 0x0000, "Standard CIE" },
  { 0x000d, "Motion Sensor" },
  { 0x0015, "Contact Switch" },
  { 0x0028, "Fire Sensor" },
  { 0x002a, "Water Sensor" },
  { 0x002b, "Gas Sensor" },
  { 0x002c, "Peersonal Emergency Device" },
  { 0x002d, "Vibration / Movement Sensor" },
  { 0x010f, "Remote Control" },
  { 0x0115, "Key Fob" },
  { 0x021d, "Keypad" },
  { 0x0225, "Standard Warning Device" },
  { 0xFFFF, NULL } // terminator
};

#define RESERVED_END 0x7FFF
#define MANUFACTURER_SPECIFIC_START 0x8000
#define MANUFACTURER_SPECIFIC_END   0xFFFE

static const char manufacturerSpecificString[] = "Manufacturer Specific";
static const char invalidZoneTypeString[] = "Invalid";
static const char reservedString[] = "Reserved";

static const char notEnrolledString[] = "NOT Enrolled";
static const char enrolledString[] = "Enrolled";
static const char unknownZoneStateString[] = "Unknown";

//-----------------------------------------------------------------------------
// Functions

static const char* getZoneTypeString(int16u type)
{
  int16u i = 0;
  while (zoneTypeToStringMap[i].zoneTypeString != NULL) {
    if (zoneTypeToStringMap[i].zoneType == type)
    return zoneTypeToStringMap[i].zoneTypeString;
    i++;
  }

  if (type <= RESERVED_END) {
    return reservedString;
  }

  if (type >= MANUFACTURER_SPECIFIC_START
      && type <= MANUFACTURER_SPECIFIC_END) {
    return manufacturerSpecificString;
  }

  return invalidZoneTypeString;
}

static const char* getZoneStateString(int8u zoneState)
{
  switch (zoneState) {
    case EMBER_ZCL_IAS_ZONE_STATE_ENROLLED:
      return enrolledString;
    case EMBER_ZCL_IAS_ZONE_STATE_NOT_ENROLLED:
      return notEnrolledString;
  }
  return unknownZoneStateString;
}

static void getAttributes(int8u*  returnCieAddress, 
                          int16u* returnZoneStatus,
                          int16u* returnZoneType,
                          int8u*  returnZoneState)
{
  EMBER_TEST_ASSERT(emAfGetIasZoneServerEndpoint() != UNKNOWN_ENDPOINT);

  emberAfReadServerAttribute(emAfGetIasZoneServerEndpoint(),
                             ZCL_IAS_ZONE_CLUSTER_ID,
                             ZCL_IAS_CIE_ADDRESS_ATTRIBUTE_ID,
                             returnCieAddress,
                             EUI64_SIZE);

  emberAfReadServerAttribute(emAfGetIasZoneServerEndpoint(),
                             ZCL_IAS_ZONE_CLUSTER_ID,
                             ZCL_ZONE_STATUS_ATTRIBUTE_ID,
                             (int8u*)returnZoneStatus,
                             2);   // int16u size  

  emberAfReadServerAttribute(emAfGetIasZoneServerEndpoint(),
                             ZCL_IAS_ZONE_CLUSTER_ID,
                             ZCL_ZONE_TYPE_ATTRIBUTE_ID,
                             (int8u*)returnZoneType,
                             2);  // int16u size   

  emberAfReadServerAttribute(emAfGetIasZoneServerEndpoint(),
                             ZCL_IAS_ZONE_CLUSTER_ID,
                             ZCL_ZONE_STATE_ATTRIBUTE_ID,
                             (int8u*)returnZoneState,
                             1);  // int8u size

}

static void infoCommand(void)
{
  int8u cieAddress[EUI64_SIZE];
  int16u zoneStatus;
  int16u zoneType;
  int8u zoneState;
  getAttributes(cieAddress,
                &zoneStatus,
                &zoneType,
                &zoneState);
  emberAfIasZoneClusterPrint("CIE Address: ");
  emberAfPrintBigEndianEui64(cieAddress);
  emberAfIasZoneClusterPrintln("");
  emberAfIasZoneClusterPrintln("Zone Type:   0x%2X (%p)", 
                               zoneType, 
                               getZoneTypeString(zoneType));
  emberAfIasZoneClusterPrintln("Zone State:  0x%X   (%p)",
                               zoneState,
                               getZoneStateString(zoneState));
  emberAfIasZoneClusterPrintln("Zone Status: 0x%2X", 
                               zoneStatus);
  emberAfIasZoneClusterPrintln("Zone ID:     0x%2X", 
                               emberAfPluginIasZoneServerGetZoneId());
}

static void changeStatusCommand(void)
{
  int16u newStatus = (int16u)emberUnsignedCommandArgument(0);
  int8u  timeSinceOccurredSeconds = (int8u)emberUnsignedCommandArgument(1);
  emberAfPluginIasZoneServerUpdateZoneStatus(newStatus,
                                             timeSinceOccurredSeconds << 2);
}
