
typedef struct {
  EmberEUI64 ieeeAddress;
  int16u zoneType;
  int16u zoneStatus;
  int8u zoneState;
  int8u endpoint;
  int8u zoneId;
} IasZoneDevice;


extern IasZoneDevice emberAfIasZoneClientKnownServers[];

#define NO_INDEX 0xFF
#define UNKNOWN_ENDPOINT 0

#define UNKNOWN_ZONE_ID 0xFF

void emAfClearServers(void);

void emberAfPluginIasZoneClientZdoCallback(EmberNodeId emberNodeId,
                                           EmberApsFrame* apsFrame,
                                           int8u* message,
                                           int16u length);

void emberAfPluginIasZoneClientWriteAttributesResponseCallback(EmberAfClusterId clusterId,
                                                               int8u * buffer,
                                                               int16u bufLen);                                           


void emberAfPluginIasZoneClientReadAttributesResponseCallback(EmberAfClusterId clusterId,
                                                              int8u * buffer,
                                                              int16u bufLen); 

