// *****************************************************************************
// * meter-mirror.c
// *
// * Code to handle being a mirror for a sleepy meter.
// *
// * Copyright 2014 Silicon Laboratories, Inc.                              *80*
// *****************************************************************************

#include "app/framework/include/af.h"
#include "app/framework/util/util.h"
#include "meter-mirror.h"

//----------------------------------------------------------------------------
// Globals

#define HAVE_MIRROR_CAPACITY 0x01
#define NO_MIRROR_CAPACITY   0x00

// Although technically endpoints are 8-bit, the returned value to the
// Mirror request and Mirror Remove commands are 16-bit.  Therefore
// we will use a 16-bit value to indicate an invalid endpoint.
#define INVALID_MIRROR_ENDPOINT 0xFFFF

#define INVALID_INDEX 0xFF

EmberEUI64 nullEui64 = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff};

#define MIRROR_NOT_FOUND_ZCL_RETURN_CODE 0xFFFF

// Each record in the report has a two-byte attribute id, a one-byte type, and
// variable-length data.
#define ATTRIBUTE_OVERHEAD 3

// Notification schemes
#define NO_NOTIFICATION_SCHEME 0x00 
#define PREDEFINED_NOTIFICATION_SCHEME_A 0x01
#define PREDEFINED_NOTIFICATION_SCHEME_B 0x02

// Notification flags
#define SUPPORTED_NOTIFICATION_FLAGS 5
#define FIRST_SUPPORTED_NOTIFICATION_FLAG_ID ZCL_PRICE_NOTIFICATION_FLAGS_ATTRIBUTE_ID

static int8u populateMirrorReportAttributeResponse(int8u endpoint);

// This is NOT stored across reboots.  Future versions of the plugin will need
// support to persistently store this.
typedef struct {
  int8u eui64[EUI64_SIZE];
} MirrorEntry;

static MirrorEntry mirrorList[EMBER_AF_PLUGIN_METER_MIRROR_MAX_MIRRORS];

//-----------------------------------------------------------------------------
// Functions

int8u emAfPluginMeterMirrorGetMirrorsAllocated(void)
{
  int8u mirrors = 0;
  int8u i;
  for (i = 0; i < EMBER_AF_PLUGIN_METER_MIRROR_MAX_MIRRORS; i++) {
    if (0 != MEMCOMPARE(mirrorList[i].eui64, nullEui64, EUI64_SIZE)) {
      mirrors++;
    }
  }
  return mirrors;
}

static void updatePhysicalEnvironment(void) 
{
  int8u physEnv = (emAfPluginMeterMirrorGetMirrorsAllocated()
                   < EMBER_AF_PLUGIN_METER_MIRROR_MAX_MIRRORS
                   ? HAVE_MIRROR_CAPACITY
                   : NO_MIRROR_CAPACITY);
  EmberStatus status
  = emberAfWriteServerAttribute(EMBER_AF_PLUGIN_METER_MIRROR_METER_CLIENT_ENDPOINT,
                                ZCL_BASIC_CLUSTER_ID,
                                ZCL_PHYSICAL_ENVIRONMENT_ATTRIBUTE_ID,
                                (int8u *)&physEnv,
                                ZCL_INT8U_ATTRIBUTE_TYPE);
  if (status) {
    emberAfReportingPrintln("Error:  Could not update mirror capacity on endpoint %d.", 
                       EMBER_AF_PLUGIN_METER_MIRROR_METER_CLIENT_ENDPOINT);
  }
}

static int8u getIndexFromEndpoint(int8u endpoint)
{
  if (endpoint < EMBER_AF_PLUGIN_METER_MIRROR_ENDPOINT_START
      ||  (endpoint >= (EMBER_AF_PLUGIN_METER_MIRROR_ENDPOINT_START
                        + EMBER_AF_PLUGIN_METER_MIRROR_MAX_MIRRORS))) {
    return INVALID_INDEX;
  }
  return (endpoint - EMBER_AF_PLUGIN_METER_MIRROR_ENDPOINT_START);
}

static void clearMirrorByIndex(int8u index, boolean print)
{
  int8u endpoint = index + EMBER_AF_PLUGIN_METER_MIRROR_ENDPOINT_START;
  MEMSET(mirrorList[index].eui64, 0xFF, EUI64_SIZE);

  if (print) {
    emberAfReportingPrintln("Removed meter mirror at endpoint %d",
                            endpoint);
  }
  emberAfEndpointEnableDisable(endpoint, FALSE);  
  updatePhysicalEnvironment();
}

static void clearAllMirrors(boolean print)
{
  int8u i;
  for (i = 0; i < EMBER_AF_PLUGIN_METER_MIRROR_MAX_MIRRORS; i++) {
    clearMirrorByIndex(i, print);
  }  
}

boolean emberAfPluginMeterMirrorGetEui64ByEndpoint(int8u endpoint,
                                                   EmberEUI64 returnEui64)
{
  int8u index = getIndexFromEndpoint(endpoint);
  if (index == INVALID_INDEX) {
    return FALSE;
  }
  MEMCOPY(returnEui64, mirrorList[index].eui64, EUI64_SIZE);
  return TRUE;
}

void emberAfPluginMeterMirrorInitCallback(void)
{
  clearAllMirrors(FALSE); // print?
}

boolean emberAfPluginMeterMirrorIsMirrorUsed(int8u endpoint)
{
  int8u index = getIndexFromEndpoint(endpoint);
  if (INVALID_INDEX == index) {
    emberAfReportingPrintln("Error:  Endpoint %d is not a valid mirror endpoint.",
                            endpoint);
    return FALSE;
  }
  return (0 != MEMCOMPARE(mirrorList[index].eui64, nullEui64, EUI64_SIZE));
}

void emberAfPluginMeterMirrorStackStatusCallback(EmberStatus status)
{
  if (status == EMBER_NETWORK_DOWN
      && !emberStackIsPerformingRejoin()) {
    emberAfReportingPrintln("Re-initializing mirrors due to stack down.");
    clearAllMirrors(TRUE);  // print?
  } 
}

static int8u findMirrorIndex(EmberEUI64 requestingDeviceIeeeAddress)
{
  int8u i;

  for (i = 0; i < EMBER_AF_PLUGIN_METER_MIRROR_MAX_MIRRORS; i++) {
    if (0 == MEMCOMPARE(requestingDeviceIeeeAddress,
                        mirrorList[i].eui64,
                        EUI64_SIZE)) {
      return i;
    }
  }
  return INVALID_INDEX;
}

int16u emberAfPluginSimpleMeteringClientRequestMirrorCallback(EmberEUI64 requestingDeviceIeeeAddress)
{
  int8u meteringDeviceTypeAttribute =  EMBER_ZCL_METER_DEVICE_TYPE_UNDEFINED_MIRROR_METER;
  int8u index = findMirrorIndex(requestingDeviceIeeeAddress);
  int8u endpoint = EMBER_AF_PLUGIN_METER_MIRROR_ENDPOINT_START;
  EmberStatus status;

  if (index != INVALID_INDEX) {
    endpoint += index; 
    emberAfReportingPrintln("Mirror already allocated on endpoint %d",
                            endpoint);
    return endpoint;
  }

  index = findMirrorIndex(nullEui64);
  if (index == INVALID_INDEX) {
    emberAfReportingPrintln("No free mirror endpoints for new mirror.\n");
    return INVALID_MIRROR_ENDPOINT;
  }

  endpoint += index;

  MEMCOPY(mirrorList[index].eui64, requestingDeviceIeeeAddress, EUI64_SIZE);
  emberAfEndpointEnableDisable(endpoint, TRUE);  
  emberAfReportingPrint("Registered new meter mirror: ");
  emberAfPrintBigEndianEui64(requestingDeviceIeeeAddress);
  emberAfReportingPrintln(".  Endpoint: %d", endpoint);
  updatePhysicalEnvironment();

  status
  = emberAfWriteServerAttribute(endpoint,
                                ZCL_SIMPLE_METERING_CLUSTER_ID, 
                                ZCL_METERING_DEVICE_TYPE_ATTRIBUTE_ID,
                                (int8u*) &meteringDeviceTypeAttribute,
                                ZCL_INT8U_ATTRIBUTE_TYPE);
  if (status) {
    emberAfReportingPrintln("Failed to write Metering device type attribute on ep: %d",
                            endpoint);
  }

  return endpoint;
}

boolean emberAfSimpleMeteringClusterConfigureMirrorCallback(int32u issuerEventId,
                                                            int32u changeReportingProfile,
                                                            int8u mirrorNotificationReporting,
                                                            int8u notificationScheme)
{
  EmberAfStatus status;
  int8u endpoint = emberAfCurrentEndpoint();
  int8u *reportingProfile = (int8u *)&changeReportingProfile;

  emberAfReportingPrintln("ConfigureMirror on endpoint 0x%x", endpoint);

#if (BIGENDIAN_CPU)
  reportingProfile++;
#endif
  
  // Write the attributes to configure the mirror
  status 
    = emberAfWriteClientAttribute(endpoint,
                                  ZCL_SIMPLE_METERING_CLUSTER_ID,
                                  ZCL_REPORTING_PROFILE_ATTRIBUTE_ID,
                                  reportingProfile,
                                  ZCL_INT24U_ATTRIBUTE_TYPE);
  if (status != EMBER_ZCL_STATUS_SUCCESS) {
    emberAfReportingPrintln("ConfigureMirror: couldn't write ReportingProfile attribute");
    goto kickout;
  }

  status 
    = emberAfWriteClientAttribute(endpoint,
                                  ZCL_SIMPLE_METERING_CLUSTER_ID,
                                  ZCL_MIRROR_REPORTING_ATTRIBUTE_ID,
                                  (int8u *)&mirrorNotificationReporting,
                                  ZCL_BOOLEAN_ATTRIBUTE_TYPE);
  if (status != EMBER_ZCL_STATUS_SUCCESS) {
    emberAfReportingPrintln("ConfigureMirror: couldn't write MirrorReporting attribute");
    goto kickout;
  }

  status 
    = emberAfWriteClientAttribute(endpoint,
                                  ZCL_SIMPLE_METERING_CLUSTER_ID,
                                  ZCL_NOTIFICATION_SCHEME_ATTRIBUTE_ID,
                                  (int8u *)&notificationScheme,
                                  ZCL_ENUM8_ATTRIBUTE_TYPE);
  if (status != EMBER_ZCL_STATUS_SUCCESS) {
    emberAfReportingPrintln("ConfigureMirror: couldn't write NotificationScheme attribute");
    goto kickout;
  }

kickout:
  emberAfSendImmediateDefaultResponse(status);
  return TRUE;
}

int16u emberAfPluginSimpleMeteringClientRemoveMirrorCallback(EmberEUI64 requestingDeviceIeeeAddress)
{
  int8u index;

  if (0 == MEMCOMPARE(nullEui64, requestingDeviceIeeeAddress, EUI64_SIZE)) {
    emberAfReportingPrintln("Rejecting mirror removal using NULL EUI64");
    return INVALID_MIRROR_ENDPOINT;
  }
  
  index = findMirrorIndex(requestingDeviceIeeeAddress);
  
  if (index == INVALID_INDEX) {
    emberAfReportingPrint("Unknown mirror for remove: ");
    emberAfPrintBigEndianEui64(requestingDeviceIeeeAddress);
    return MIRROR_NOT_FOUND_ZCL_RETURN_CODE;
  }

  clearMirrorByIndex(index,
                     TRUE);  // print?
  return index + EMBER_AF_PLUGIN_METER_MIRROR_ENDPOINT_START;
}

boolean emberAfReportAttributesCallback(EmberAfClusterId clusterId,
                                        int8u *buffer,
                                        int16u bufLen)
{
  EmberEUI64 sendersEui;
  int16u bufIndex = 0;
  int8u endpoint;
  int8u index;

  if (clusterId != ZCL_SIMPLE_METERING_CLUSTER_ID) {
    emberAfReportingPrintln("Meter Mirror Plugin ignoring non Meter Cluster ID: 0x%2X", 
                      clusterId);
    emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_UNREPORTABLE_ATTRIBUTE);
    return TRUE;
  }

  if (EMBER_SUCCESS
       != emberLookupEui64ByNodeId(emberAfCurrentCommand()->source, sendersEui)) {
    emberAfReportingPrintln("Error: Meter Mirror plugin cannot determine EUI64 for node ID 0x%2X", 
                      emberAfCurrentCommand()->source);
    emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_FAILURE);
    return TRUE;
  }

  if (emberAfCurrentCommand()->direction
      == ZCL_DIRECTION_CLIENT_TO_SERVER) {
    emberAfReportingPrintln("Error:  Meter Mirror Plugin does not accept client to server attributes.\n",
                       emberAfCurrentCommand()->direction);
    emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_FAILURE);
    return TRUE;
  }
  
  index = findMirrorIndex(sendersEui);
  if (index == INVALID_INDEX) {
    emberAfReportingPrint("Error: Meter mirror plugin received unknown report from ");
    emberAfPrintBigEndianEui64(sendersEui);
    emberAfReportingPrintln("");
    emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_NOT_AUTHORIZED);
    return TRUE;
  }

  if (emberAfCurrentCommand()->mfgSpecific) {
    // Here is where we could handle a MFG specific Report attributes and interpret 
    // it.  This code does not do that, just politely returns an error.
    emberAfReportingPrintln("Error: Unknown MFG Code for mirror: 0x%2X", 
                            emberAfCurrentCommand()->mfgCode);
    emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_UNSUP_MANUF_GENERAL_COMMAND);
    return TRUE;
  }

  endpoint = (index + EMBER_AF_PLUGIN_METER_MIRROR_ENDPOINT_START);
  while (bufIndex + ATTRIBUTE_OVERHEAD < bufLen) {
    EmberAfStatus status;
    EmberAfAttributeId attributeId;
    EmberAfAttributeType dataType;
    int8u dataSize;

    attributeId = (EmberAfAttributeId)emberAfGetInt16u(buffer,
                                                       bufIndex,
                                                       bufLen);
    bufIndex += 2;
    dataType = (EmberAfAttributeType)emberAfGetInt8u(buffer, bufIndex, bufLen);
    bufIndex++;

    // For strings, the data size is the length of the string (specified by the
    // first byte of data) plus one for the length byte itself.  For everything
    // else, the size is just the size of the data type.
    dataSize = (emberAfIsThisDataTypeAStringType(dataType)
                ? emberAfStringLength(buffer + bufIndex) + 1
                : emberAfGetDataSize(dataType));

    {
#if (BIGENDIAN_CPU)
      int8u data[ATTRIBUTE_LARGEST];
      if (isThisDataTypeSentLittleEndianOTA(dataType)) {
        emberReverseMemCopy(data, buffer + bufIndex, dataSize);
      } else {
        MEMCOPY(data, buffer + bufIndex, dataSize);
      }
#else
      int8u *data = buffer + bufIndex;
#endif

      status = emberAfWriteServerAttribute(endpoint,
                                           clusterId,
                                           attributeId,
                                           data,
                                           dataType);
    }

    emberAfReportingPrintln("Mirror attribute 0x%2x: 0x%x", attributeId, status);
    bufIndex += dataSize;
  }

  // Notification flags
  {
    int8u mirrorReporting, scheme;
    EmberAfStatus status;
    status = emberAfReadClientAttribute(endpoint,
                                        ZCL_SIMPLE_METERING_CLUSTER_ID,
                                        ZCL_MIRROR_REPORTING_ATTRIBUTE_ID,
                                        (int8u *)&mirrorReporting,
                                        sizeof(mirrorReporting));
    emberAfReportingPrintln("Mirror reporting ep: 0x%x, val: 0x%x, status: 0x%x", endpoint, mirrorReporting, status);
    if (status == EMBER_ZCL_STATUS_SUCCESS && mirrorReporting) {
      scheme = populateMirrorReportAttributeResponse(endpoint);
      if (scheme != NO_NOTIFICATION_SCHEME) {
        emberAfReportingPrintln("Mirror reporting has notification scheme 0x%x", scheme);
        emberAfSendResponse();
      }
    }
  }

  emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_SUCCESS);
  return TRUE;
}

static int8u populateMirrorReportAttributeResponse(int8u endpoint)
{
  int8u notificationScheme = 0, status, flag;
  int8u notificationFlagsBuffer[SUPPORTED_NOTIFICATION_FLAGS * 4];
  int32u readFlag;

  // Read the notification scheme
  status = emberAfReadClientAttribute(endpoint,
                                      ZCL_SIMPLE_METERING_CLUSTER_ID,
                                      ZCL_NOTIFICATION_SCHEME_ATTRIBUTE_ID,
                                      (int8u *)&notificationScheme,
                                      sizeof(notificationScheme));
  if (status != EMBER_ZCL_STATUS_SUCCESS ||
      (notificationScheme != PREDEFINED_NOTIFICATION_SCHEME_A &&
      notificationScheme != PREDEFINED_NOTIFICATION_SCHEME_B)) {
    emberAfReportingPrintln("Mirror reporting has no notification scheme on endpoint 0x%x, status: 0x%x", endpoint, status);
    return NO_NOTIFICATION_SCHEME;
  }

  // Functional Notification Flags
  status = emberAfReadClientAttribute(endpoint,
                                      ZCL_SIMPLE_METERING_CLUSTER_ID,
                                      ZCL_FUNCTIONAL_NOTIFICATION_FLAGS_ATTRIBUTE_ID,
                                      (int8u *)&readFlag,
                                      sizeof(int32u));
  if (status != EMBER_ZCL_STATUS_SUCCESS) {
    MEMSET(notificationFlagsBuffer, 0, sizeof(int32u));
  } else {
    emberAfCopyInt32u(notificationFlagsBuffer, 0, readFlag);
  }

  // Predefined Notification Scheme A prescribes only reporting the functional
  // notification flags
  if (notificationScheme == PREDEFINED_NOTIFICATION_SCHEME_A) {
    goto kickout;
  }

  // Predefined Notification Scheme B requires reporting all notification flags
  for (flag = FIRST_SUPPORTED_NOTIFICATION_FLAG_ID; 
       flag < SUPPORTED_NOTIFICATION_FLAGS; 
       flag++) {
    status = emberAfReadClientAttribute(endpoint,
                                        ZCL_SIMPLE_METERING_CLUSTER_ID,
                                        flag,
                                        (int8u *)&readFlag,
                                        sizeof(int32u));
    if (status != EMBER_ZCL_STATUS_SUCCESS) {
      MEMSET((&notificationFlagsBuffer) + (flag * 4), 0, sizeof(int32u));
    } else {
      emberAfCopyInt32u(notificationFlagsBuffer, (flag * 4), readFlag);
    }
  }

kickout:
  // Populate the command buffer
  emberAfFillCommandSimpleMeteringClusterMirrorReportAttributeResponse(notificationScheme,
                                                                       (int8u *) notificationFlagsBuffer,
                                                                       (notificationScheme == PREDEFINED_NOTIFICATION_SCHEME_A)
                                                                       ? 4
                                                                       : 20);
  emberAfReportingPrintln("Mirror report attribute response buffer populated");
  return notificationScheme;
}
