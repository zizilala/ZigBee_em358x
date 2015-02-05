// meter-mirror.h


// A bit confusing, the EMBER_AF_MANUFACTURER_CODE is actually the manufacturer
// code defined in AppBuilder.  This is usually the specific vendor of
// the local application.  It does not have to be "Ember's" (Silabs) manufacturer
// code, but that is the default.
#define EM_AF_APPLICATION_MANUFACTURER_CODE EMBER_AF_MANUFACTURER_CODE

int8u emAfPluginMeterMirrorGetMirrorsAllocated(void);

extern EmberEUI64 nullEui64;

#define EM_AF_MIRROR_ENDPOINT_END \
  (EMBER_AF_PLUGIN_METER_MIRROR_ENDPOINT_START \
   + EMBER_AF_PLUGIN_METER_MIRROR_MAX_MIRRORS)

boolean emberAfPluginMeterMirrorGetEui64ByEndpoint(int8u endpoint,
                                                   EmberEUI64 returnEui64);
boolean emberAfPluginMeterMirrorIsMirrorUsed(int8u endpoint);

