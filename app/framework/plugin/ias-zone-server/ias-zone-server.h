#define UNDEFINED_ZONE_ID 0xFF
#define UNKNOWN_ENDPOINT 0

EmberStatus emberAfPluginIasZoneServerUpdateZoneStatus(int16u newStatus, 
                                                       int8u  timeSinceStatusOccurredQs);

int8u emAfGetIasZoneServerEndpoint(void);
int8u emAfGetRemoteIasZoneClientEndpoint(void); 
int8u emberAfPluginIasZoneServerGetZoneId(void);