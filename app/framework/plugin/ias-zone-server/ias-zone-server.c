// ias-zone-server.c

#include "af.h"
#include "ias-zone-server.h"

//-----------------------------------------------------------------------------
// Globals

static int8u myEndpoint = 0;
static int8u clientEndpoint = UNKNOWN_ENDPOINT;

#define DELAY_TIMER_MS 1000 

extern int8u* emAfZclBuffer;

//-----------------------------------------------------------------------------
// Forward declarations

static void setZoneId(int8u zoneId);

//-----------------------------------------------------------------------------
// Functions

static EmberStatus sendToClient(void)
{
  int8u ieeeAddress[EUI64_SIZE];
  EmberStatus status = emberAfReadServerAttribute(myEndpoint,
                                                  ZCL_IAS_ZONE_CLUSTER_ID,
                                                  ZCL_IAS_CIE_ADDRESS_ATTRIBUTE_ID,
                                                  ieeeAddress,
                                                  EUI64_SIZE);
  if (EMBER_SUCCESS != status) {
    emberAfIasZoneClusterPrintln("Error: Failed to get CIE Address attribute.  Cannot send to IAS client.");
    return status;
  }
  EmberNodeId destNodeId = emberLookupNodeIdByEui64(ieeeAddress);
  if (destNodeId == EMBER_NULL_NODE_ID) {
    emberAfIasZoneClusterPrintln("Error: Could not determine node ID for CIE IEEE Address.  Cannot send to IAS Client.");
    return EMBER_ERR_FATAL;
  }
  emberAfSetCommandEndpoints(myEndpoint, clientEndpoint);
  emAfZclBuffer[0] |= ZCL_DISABLE_DEFAULT_RESPONSE_MASK;
  status = emberAfSendCommandUnicast(EMBER_OUTGOING_DIRECT,
                                                 destNodeId);
  if (status != EMBER_SUCCESS) {
    emberAfIasZoneClusterPrintln("Error: Failed to send IAS Zone Server message.");
  }
  return status;
}

static void enrollWithClient(void)
{
  emberAfFillCommandIasZoneClusterZoneEnrollRequest(EMBER_AF_PLUGIN_IAS_ZONE_SERVER_ZONE_TYPE,
                                                    EMBER_AF_MANUFACTURER_CODE);
  if (EMBER_SUCCESS == sendToClient()) {
    emberAfIasZoneClusterPrintln("Sent enroll request to IAS Zone client.");
  }
}

void emberAfIasZoneClusterServerAttributeChangedCallback(int8u endpoint,
                                                         EmberAfAttributeId attributeId)
{
  if (endpoint != myEndpoint
      || (attributeId != ZCL_IAS_CIE_ADDRESS_ATTRIBUTE_ID)) {
    return;
  }
  clientEndpoint = endpoint;

  // We need to delay to get around a bug where we can't send a command
  // at this point because then the Write Attributes response will not
  // be sent.  But we also delay to give the client time to configure us.
  emberAfIasZoneClusterPrintln("Sending enrollment after %d ms", DELAY_TIMER_MS);
  emberAfScheduleServerTickExtended(myEndpoint,
                                    ZCL_IAS_ZONE_CLUSTER_ID,
                                    DELAY_TIMER_MS,
                                    EMBER_AF_SHORT_POLL,
                                    EMBER_AF_STAY_AWAKE);
}

static boolean amIEnrolled(void)
{
  int8u zoneState = EMBER_ZCL_IAS_ZONE_STATE_NOT_ENROLLED;
  emberAfReadServerAttribute(myEndpoint,
                             ZCL_IAS_ZONE_CLUSTER_ID,
                             ZCL_ZONE_STATE_ATTRIBUTE_ID,
                             &zoneState,
                             1);  // int8u size
  return (zoneState == EMBER_ZCL_IAS_ZONE_STATE_ENROLLED);
}

static void updateEnrollState(boolean enrolled)
{
  int8u zoneState = (enrolled
                     ? EMBER_ZCL_IAS_ZONE_STATE_ENROLLED
                     : EMBER_ZCL_IAS_ZONE_STATE_NOT_ENROLLED);
  emberAfWriteAttribute(myEndpoint,
                        ZCL_IAS_ZONE_CLUSTER_ID,
                        ZCL_ZONE_STATE_ATTRIBUTE_ID, 
                        CLUSTER_MASK_SERVER,
                        (int8u*)&zoneState,
                        ZCL_INT8U_ATTRIBUTE_TYPE);
  emberAfIasZoneClusterPrintln("IAS Zone Server State: %pEnrolled",
                               (enrolled
                                ? ""
                                : "NOT "));
}

boolean emberAfIasZoneClusterZoneEnrollResponseCallback(int8u enrollResponseCode,
                                                        int8u zoneId)
{
  updateEnrollState(enrollResponseCode
                    == EMBER_ZCL_IAS_ENROLL_RESPONSE_CODE_SUCCESS);
  setZoneId((enrollResponseCode
              == EMBER_ZCL_IAS_ENROLL_RESPONSE_CODE_SUCCESS)
            ? zoneId
            : UNDEFINED_ZONE_ID);
  return TRUE;
} 

static EmberStatus sendZoneUpdate(int8u timeSinceStatusOccurredQs)
{
  EmberStatus status;
  int16u zoneStatus;
  if (!amIEnrolled()) {
    return EMBER_INVALID_CALL;
  }
  emberAfReadServerAttribute(myEndpoint,
                             ZCL_IAS_ZONE_CLUSTER_ID,
                             ZCL_ZONE_STATUS_ATTRIBUTE_ID,
                             (int8u*)&zoneStatus,
                             2);  // int16u size  
  emberAfFillCommandIasZoneClusterZoneStatusChangeNotification(zoneStatus,
                                                               0, // extended status
                                                                  // must be zero per the spec.
                                                               emberAfPluginIasZoneServerGetZoneId(),
                                                               timeSinceStatusOccurredQs);  // known as delay in the spec 
  status = sendToClient();
  if (EMBER_SUCCESS == status) {
    emberAfIasZoneClusterPrintln("Sent zone status update (0x%2X)", zoneStatus);
  }
  return status;
}

EmberStatus emberAfPluginIasZoneServerUpdateZoneStatus(int16u newStatus,
                                                       int8u timeSinceStatusOccurredQs)
{
  emberAfWriteAttribute(myEndpoint,
                        ZCL_IAS_ZONE_CLUSTER_ID,
                        ZCL_ZONE_STATUS_ATTRIBUTE_ID, 
                        CLUSTER_MASK_SERVER,
                        (int8u*)&newStatus,
                        ZCL_INT16U_ATTRIBUTE_TYPE);  
  return sendZoneUpdate(timeSinceStatusOccurredQs);
}

void emberAfIasZoneClusterServerInitCallback(int8u endpoint)
{
  myEndpoint = endpoint;

  int8u ieeeAddress[] = { 0, 0, 0, 0, 0, 0, 0, 0 };
  emberAfWriteAttribute(endpoint,
                        ZCL_IAS_ZONE_CLUSTER_ID,
                        ZCL_IAS_CIE_ADDRESS_ATTRIBUTE_ID,
                        CLUSTER_MASK_SERVER,
                        (int8u*)ieeeAddress,
                        ZCL_IEEE_ADDRESS_ATTRIBUTE_TYPE);

  int16u zoneType = EMBER_AF_PLUGIN_IAS_ZONE_SERVER_ZONE_TYPE;
  emberAfWriteAttribute(endpoint,
                        ZCL_IAS_ZONE_CLUSTER_ID,
                        ZCL_ZONE_TYPE_ATTRIBUTE_ID, 
                        CLUSTER_MASK_SERVER,
                        (int8u*)&zoneType,
                        ZCL_INT16U_ATTRIBUTE_TYPE);

  setZoneId(UNDEFINED_ZONE_ID);

  updateEnrollState(FALSE);  // enrolled?

  emberAfPluginIasZoneServerUpdateZoneStatus(0,   // status
                                             0);  // time since status occurred
} 

void emberAfIasZoneClusterServerTickCallback(int8u endpoint)
{
  enrollWithClient();
}                   

int8u emAfGetIasZoneServerEndpoint(void)
{
  return myEndpoint;
}

int8u emAfGetRemoteIasZoneClientEndpoint(void)
{
  return clientEndpoint;
}

int8u emberAfPluginIasZoneServerGetZoneId(void)
{
  int8u zoneId = UNDEFINED_ZONE_ID;
  emberAfReadServerAttribute(myEndpoint,
                             ZCL_IAS_ZONE_CLUSTER_ID,
                             ZCL_ZONE_ID_ATTRIBUTE_ID,
                             &zoneId,
                             emberAfGetDataSize(ZCL_INT8U_ATTRIBUTE_TYPE));
  return zoneId;
}

static void setZoneId(int8u zoneId)
{
  emberAfWriteAttribute(myEndpoint,
                        ZCL_IAS_ZONE_CLUSTER_ID,
                        ZCL_ZONE_ID_ATTRIBUTE_ID,
                        CLUSTER_MASK_SERVER,
                        &zoneId,
                        ZCL_INT8U_ATTRIBUTE_TYPE);
}


// New DC code
void unenrollSecurityDevice( void )
{
  myEndpoint = emAfEndpoints[0].endpoint;
  //emberSerialPrintf(APP_SERIAL, "unenroll A %x\r\n", myEndpoint);

  int8u ieeeAddress[] = { 0, 0, 0, 0, 0, 0, 0, 0 };
  emberAfWriteAttribute(myEndpoint,
                        ZCL_IAS_ZONE_CLUSTER_ID,
                        ZCL_IAS_CIE_ADDRESS_ATTRIBUTE_ID,
                        CLUSTER_MASK_SERVER,
                        (int8u*)ieeeAddress,
                        ZCL_IEEE_ADDRESS_ATTRIBUTE_TYPE);

  //emberSerialPrintf(APP_SERIAL, "unenroll B\r\n");
  int16u zoneType = EMBER_AF_PLUGIN_IAS_ZONE_SERVER_ZONE_TYPE;
  emberAfWriteAttribute(myEndpoint,
                        ZCL_IAS_ZONE_CLUSTER_ID,
                        ZCL_ZONE_TYPE_ATTRIBUTE_ID, 
                        CLUSTER_MASK_SERVER,
                        (int8u*)&zoneType,
                        ZCL_INT16U_ATTRIBUTE_TYPE);

  //emberSerialPrintf(APP_SERIAL, "unenroll C\r\n");
  setZoneId(UNDEFINED_ZONE_ID);

  //emberSerialPrintf(APP_SERIAL, "unenroll D\r\n");
  updateEnrollState(FALSE);  // enrolled?
}
