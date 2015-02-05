
extern int8u emAfRouteErrorCount;
extern int8u emAfDeliveryFailureCount;

extern EmberEventControl emberAfPluginConcentratorUpdateEventControl;

#define LOW_RAM_CONCENTRATOR  EMBER_LOW_RAM_CONCENTRATOR
#define HIGH_RAM_CONCENTRATOR EMBER_HIGH_RAM_CONCENTRATOR

#define emAfConcentratorStartDiscovery emberAfPluginConcentratorQueueDiscovery
void emAfConcentratorStopDiscovery(void);


int32u emberAfPluginConcentratorQueueDiscovery(void);
void emberAfPluginConcentratorStopDiscovery(void);

