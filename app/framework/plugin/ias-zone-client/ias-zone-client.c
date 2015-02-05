// IAS Zone Plugin

// Client Operation:
//   1. Look for ZDO device announce notification.
//   2. Perform ZDO match descriptor on device.
//   3. If supports IAS Zone Server, Add that server to our known list.
//     Write CIE Address.
//   4. Read CIE address, verify it is ours.  This is done mostly because
//     the test case requires it.
//   5. Read the IAS Zone Server attributes.
//     Record in table.
//   6. When we get an enroll request, give them our (only) zone ID.
//   7. When we get a notification, read their attributes.

// Improvements that could be made:
//   Add support for multiple endpoints on server.  Most often this is a
//   legacy security system retrofitted with a single ZigBee radio.  Therefore
//   each sensor is on a different endpoint.  Right now our client only 
//   handles a single endpoint per node.
//   
//   Add better handling of multihop and cases where the node Id is unknown
//   because it is not a child or neighbor of the client.  This would require
//   an additional step in certain cases to obtain the IEEE from the node Id,
//   or better use of the address table for this information.
//
//   Integration with Poll Control.  When the device boots we should configure
//   its polling to make it possible to read/write its attributes.
//   

#include "af.h"
#include "ias-zone-client.h"

//-----------------------------------------------------------------------------
// Globals

IasZoneDevice emberAfIasZoneClientKnownServers[EMBER_AF_PLUGIN_IAS_ZONE_CLIENT_MAX_DEVICES];

typedef enum {
  IAS_ZONE_CLIENT_STATE_NONE,
  IAS_ZONE_CLIENT_STATE_DISCOVER_ENDPOINT,
  IAS_ZONE_CLIENT_STATE_SET_CIE_ADDRESS,
  IAS_ZONE_CLIENT_STATE_READ_CIE_ADDRESS,
  IAS_ZONE_CLIENT_STATE_READ_ATTRIBUTES,
} IasZoneClientState;

static IasZoneClientState iasZoneClientState = IAS_ZONE_CLIENT_STATE_NONE;
static int8u currentIndex = NO_INDEX;
static int8u myEndpoint = 0;
static int8u serverCount = 0;

EmberEventControl emberAfPluginIasZoneClientStateMachineEventControl;

int8u emAfPluginIasZoneClientZoneId = EMBER_AF_PLUGIN_IAS_ZONE_CLIENT_ZONE_ID;

//-----------------------------------------------------------------------------
// Forward Declarations

void readIasZoneServerAttributes(EmberNodeId nodeId);

//-----------------------------------------------------------------------------
// Functions

void emAfClearServers(void)
{
  MEMSET(emberAfIasZoneClientKnownServers, 0xFF, 
         sizeof(IasZoneDevice)
         * EMBER_AF_PLUGIN_IAS_ZONE_CLIENT_MAX_DEVICES);

}

void emberAfIasZoneClusterClientInitCallback(int8u endpoint)
{
  emAfClearServers();
  myEndpoint = endpoint;
}

static void clearState(void)
{
  currentIndex = 0;
  iasZoneClientState = IAS_ZONE_CLIENT_STATE_NONE;
}

static int8u findIasZoneServerByIeee(int8u* ieeeAddress)
{
  int8u i;
  for (i = 0; i < EMBER_AF_PLUGIN_IAS_ZONE_CLIENT_MAX_DEVICES; i++) {
    if (0 == MEMCOMPARE(ieeeAddress, 
                        emberAfIasZoneClientKnownServers[i].ieeeAddress, 
                        EUI64_SIZE)) {
      return i;
    }
  }
  return NO_INDEX;
}

static int8u findIasZoneServerByNodeId(EmberNodeId nodeId)
{
  EmberEUI64 remoteEUi64;
  int8u serverIndex = NO_INDEX;
  if (EMBER_SUCCESS == emberLookupEui64ByNodeId(nodeId,
                                                remoteEUi64)) {
    serverIndex = findIasZoneServerByIeee(remoteEUi64);
  }
  return serverIndex;
}

boolean emberAfIasZoneClusterZoneStatusChangeNotificationCallback(int16u zoneStatus,
                                                                  int8u extendedStatus,
                                                                  int8u zoneId,
                                                                  int16u delay)
{
  int8u serverIndex = findIasZoneServerByNodeId(emberAfCurrentCommand()->source);
  int8u status = EMBER_ZCL_STATUS_NOT_FOUND;
  if (serverIndex != NO_INDEX) {
    status = EMBER_ZCL_STATUS_SUCCESS;
    emberAfIasZoneClientKnownServers[serverIndex].zoneStatus = zoneStatus;

    emberAfIasZoneClusterPrintln("Zone %d status change, 0x%2X from 0x%2X",
                                 zoneId,
                                 zoneStatus,
                                 emberAfCurrentCommand()->source);

    // The Test case calls for readding attributes after status change.
    //   that is silly for the production device.
    // readIasZoneServerAttributes(emberAfCurrentCommand()->source);
  }
  emberAfSendDefaultResponse(emberAfCurrentCommand(), status);
  return TRUE;
}

boolean emberAfIasZoneClusterZoneEnrollRequestCallback(int16u zoneType,
                                                       int16u manufacturerCode)
{
  EmberAfIasEnrollResponseCode responseCode = EMBER_ZCL_IAS_ENROLL_RESPONSE_CODE_NO_ENROLL_PERMIT;
  int8u zoneId = UNKNOWN_ZONE_ID;
  int8u serverIndex = findIasZoneServerByNodeId(emberAfCurrentCommand()->source);
  if (serverIndex != NO_INDEX) {
    responseCode = EMBER_ZCL_IAS_ENROLL_RESPONSE_CODE_SUCCESS;
    zoneId = emAfPluginIasZoneClientZoneId;
    emberAfIasZoneClientKnownServers[serverIndex].zoneId = zoneId;
  }
  emberAfFillCommandIasZoneClusterZoneEnrollResponse(responseCode,
                                                     zoneId);
  EmberStatus status = emberAfSendResponse();
  emberAfCorePrintln("Sent enroll response, status: 0x%X", status);  
  return TRUE;
}

void emberAfPluginIasZoneClientStateMachineEventHandler(void)
{
  emberAfIasZoneClusterPrintln("IAS Zone Client Timeout waiting for message response.");
  emberEventControlSetInactive(emberAfPluginIasZoneClientStateMachineEventControl);
  clearState();
}

static int8u addServer(int8u* ieeeAddress)
{
  int8u i = findIasZoneServerByIeee(ieeeAddress);
  if (i != NO_INDEX) {
    return i;
  }

  for (i = 0; i < EMBER_AF_PLUGIN_IAS_ZONE_CLIENT_MAX_DEVICES; i++) {
    const int8u unsetEui64[] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
    if (0 == MEMCOMPARE(emberAfIasZoneClientKnownServers[i].ieeeAddress, 
                        unsetEui64, 
                        EUI64_SIZE)) {
      MEMCOPY(emberAfIasZoneClientKnownServers[i].ieeeAddress, ieeeAddress, EUI64_SIZE);
      emberAfIasZoneClientKnownServers[i].endpoint = UNKNOWN_ENDPOINT;
      serverCount++;
      return i;
    }
  }
  return NO_INDEX;
}

static void removeServer(int8u* ieeeAddress)
{
  int8u index = findIasZoneServerByIeee(ieeeAddress);
  MEMSET(emberAfIasZoneClientKnownServers[index].ieeeAddress,
         0xFF,
         sizeof(IasZoneDevice));
}

static EmberStatus sendCommand(EmberNodeId destAddress)
{
  emberAfSetCommandEndpoints(myEndpoint, emberAfIasZoneClientKnownServers[currentIndex].endpoint);
  EmberStatus status = emberAfSendCommandUnicast(EMBER_OUTGOING_DIRECT, destAddress);
  emberAfIasZoneClusterPrintln("Sent IAS Zone Client Command to 0x%2X (%d -> %d) status: 0x%X", 
                               destAddress,
                               myEndpoint, 
                               emberAfIasZoneClientKnownServers[currentIndex].endpoint, 
                               status);
  if (status != EMBER_SUCCESS) {
    clearState();
  }
  return status;
}

static void setCieAddress(EmberNodeId destAddress)
{
  int8u writeAttributes[] = {
    LOW_BYTE(ZCL_IAS_CIE_ADDRESS_ATTRIBUTE_ID), 
    HIGH_BYTE(ZCL_IAS_CIE_ADDRESS_ATTRIBUTE_ID),
    ZCL_IEEE_ADDRESS_ATTRIBUTE_TYPE,
    0, 0, 0, 0, 0, 0, 0, 0,  // ieee (filled in later)
  };
  emberAfGetEui64(&writeAttributes[3]);
  emberAfFillCommandGlobalClientToServerWriteAttributes(ZCL_IAS_ZONE_CLUSTER_ID,
                                                        writeAttributes, 
                                                        sizeof(writeAttributes));
  emberAfIasZoneClusterPrintln("Writing CIE Address to IAS Zone Server");
  if (EMBER_SUCCESS == sendCommand(destAddress)) {
    iasZoneClientState = IAS_ZONE_CLIENT_STATE_SET_CIE_ADDRESS;
  }

}

static void iasZoneClientServiceDiscoveryCallback(const EmberAfServiceDiscoveryResult* result)
{
  if (result->status == EMBER_AF_UNICAST_SERVICE_DISCOVERY_COMPLETE_WITH_RESPONSE
      && result->zdoRequestClusterId == MATCH_DESCRIPTORS_REQUEST) {
      const EmberAfEndpointList* endpointList = (const EmberAfEndpointList *)result->responseData;
      if (endpointList->count > 0) {
        emberAfIasZoneClientKnownServers[currentIndex].endpoint = endpointList->list[0];
        emberAfIasZoneClusterPrintln("Device 0x%2X supports IAS Zone Server",
                                     result->matchAddress);
        setCieAddress(result->matchAddress);
        return;
      }
  }
  clearState();
}

static void checkForIasZoneServer(EmberNodeId emberNodeId, int8u* ieeeAddress)
{
  int8u endpointIndex = emberAfIndexFromEndpoint(myEndpoint);	
  int16u profileId = emberAfProfileIdFromIndex(endpointIndex);
  int8u serverIndex = addServer(ieeeAddress);

  if (serverIndex == NO_INDEX) {
    emberAfIasZoneClusterPrintln("Error: Could not add IAS Zone server.");
    return;
  }
  if (emberAfIasZoneClientKnownServers[serverIndex].endpoint != UNKNOWN_ENDPOINT) {
    emberAfIasZoneClusterPrintln("Node 0x%2X already known to IAS client", emberNodeId);
    return;
  }

  currentIndex = serverIndex;
  EmberStatus status = emberAfFindDevicesByProfileAndCluster(emberNodeId,
                                                             profileId,
                                                             ZCL_IAS_ZONE_CLUSTER_ID,
                                                             TRUE,  // server cluster?
                                                             iasZoneClientServiceDiscoveryCallback);

  if (status != EMBER_SUCCESS) {
    emberAfIasZoneClusterPrintln("Error: Failed to initiate service discovery for IAS Zone Server 0x%2X", emberNodeId);
    clearState();
  }
}

void emberAfPluginIasZoneClientZdoCallback(EmberNodeId emberNodeId,
                                           EmberApsFrame* apsFrame,
                                           int8u* message,
                                           int16u length)
{
  emberAfIasZoneClusterPrintln("Incoming ZDO, Cluster: 0x%2X", apsFrame->clusterId);
  if (apsFrame->clusterId == END_DEVICE_ANNOUNCE) {
    checkForIasZoneServer(emberNodeId, &(message[3]));
  }
}

void readIasZoneServerAttributes(EmberNodeId nodeId)
{
  int8u iasZoneAttributeIds[] = {
    LOW_BYTE(ZCL_ZONE_STATE_ATTRIBUTE_ID),
    HIGH_BYTE(ZCL_ZONE_STATE_ATTRIBUTE_ID),

    LOW_BYTE(ZCL_ZONE_TYPE_ATTRIBUTE_ID),
    HIGH_BYTE(ZCL_ZONE_TYPE_ATTRIBUTE_ID),

    LOW_BYTE(ZCL_ZONE_STATUS_ATTRIBUTE_ID),
    HIGH_BYTE(ZCL_ZONE_STATUS_ATTRIBUTE_ID),
  };
  emberAfFillCommandGlobalClientToServerReadAttributes(ZCL_IAS_ZONE_CLUSTER_ID,
                                                       iasZoneAttributeIds,
                                                       sizeof(iasZoneAttributeIds));
  if (EMBER_SUCCESS == sendCommand(nodeId)) {
    iasZoneClientState = IAS_ZONE_CLIENT_STATE_READ_ATTRIBUTES;
  }

}

void readIasZoneServerCieAddress(EmberNodeId nodeId)
{
  int8u iasZoneAttributeIds[] = {
    LOW_BYTE(ZCL_IAS_CIE_ADDRESS_ATTRIBUTE_ID),
    HIGH_BYTE(ZCL_IAS_CIE_ADDRESS_ATTRIBUTE_ID),
  };
  emberAfFillCommandGlobalClientToServerReadAttributes(ZCL_IAS_ZONE_CLUSTER_ID,
                                                       iasZoneAttributeIds,
                                                       sizeof(iasZoneAttributeIds));
  if (EMBER_SUCCESS == sendCommand(nodeId)) {
    iasZoneClientState = IAS_ZONE_CLIENT_STATE_READ_CIE_ADDRESS;
  }

}

void emberAfPluginIasZoneClientWriteAttributesResponseCallback(EmberAfClusterId clusterId,
                                                                  int8u * buffer,
                                                                  int16u bufLen)
{
  if (clusterId == ZCL_IAS_ZONE_CLUSTER_ID
      && iasZoneClientState == IAS_ZONE_CLIENT_STATE_SET_CIE_ADDRESS
      && buffer[0] == EMBER_ZCL_STATUS_SUCCESS) {
    readIasZoneServerCieAddress(emberAfCurrentCommand()->source);
    return;
  }
  return;
}

void emberAfPluginIasZoneClientReadAttributesResponseCallback(EmberAfClusterId clusterId,
                                                              int8u * buffer,
                                                              int16u bufLen) 
{
  if (clusterId == ZCL_IAS_ZONE_CLUSTER_ID
      && (iasZoneClientState == IAS_ZONE_CLIENT_STATE_READ_ATTRIBUTES
          || iasZoneClientState == IAS_ZONE_CLIENT_STATE_READ_CIE_ADDRESS)) {
    int16u i = 0;
    while ((i+3) <= bufLen) {  // 3 to insure we can read at least the attribute ID
                               // and the status
      int16u attributeId = buffer[i] + (buffer[i+1] << 8);
      int8u status = buffer[i+2];
      i += 3;
      //emberAfIasZoneClusterPrintln("Parsing Attribute 0x%2X, Status: 0x%X", attributeId, status);
      if (status == EMBER_ZCL_STATUS_SUCCESS) {
        if ((i+1) > bufLen) {
          // Too short, dump the message.
          return;
        }
        i++;  // skip the type of the attribute.  We already know what it should be.
        switch (attributeId) {
          case ZCL_ZONE_STATUS_ATTRIBUTE_ID:
            if ((i+2) > bufLen) {
              // Too short, dump the message.
              return;
            }
            emberAfIasZoneClientKnownServers[currentIndex].zoneStatus = (buffer[i]
                                                                      + (buffer[i+1] << 8));
            i += 2;
            break;
          case ZCL_ZONE_TYPE_ATTRIBUTE_ID:
            if ((i+2) > bufLen) {
              // Too short, dump the message.
              return;
            }
            emberAfIasZoneClientKnownServers[currentIndex].zoneType = (buffer[i]
                                                                    + (buffer[i+1] << 8));
            i += 2;
            break;
          case ZCL_ZONE_STATE_ATTRIBUTE_ID:
            if ((i+1) > bufLen) {
              // Too short, dump the message
              return;
            }
            emberAfIasZoneClientKnownServers[currentIndex].zoneState = buffer[i];
            i++;
            break;
          case ZCL_IAS_CIE_ADDRESS_ATTRIBUTE_ID: {
            int8u myIeee[EUI64_SIZE];
            emberAfGetEui64(myIeee);
            if ((i+8) > bufLen) {
              // Too short, dump the message
            } else if (0 != MEMCOMPARE(&(buffer[i]),  myIeee, EUI64_SIZE)) {
              emberAfIasZoneClusterPrintln("CIE Address not set to mine, removing IAS zone server.");
              removeServer(&(buffer[i]));
              clearState();
            } else {
              readIasZoneServerAttributes(emberAfCurrentCommand()->source);
            }
            return;
          }
        }
      }
    }
    emberAfIasZoneClusterPrintln("Retrieved IAS Zone Server attributes from 0x%2X", 
                                 emberAfCurrentCommand()->source);
    clearState();
  }
}


