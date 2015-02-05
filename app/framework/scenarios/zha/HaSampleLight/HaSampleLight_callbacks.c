// Copyright 2013 Silicon Laboratories, Inc.

#include "app/framework/include/af.h"
#include "app/util/common/form-and-join.h"
#include "app/framework/plugin/ezmode-commissioning/ez-mode.h"
#include "app/framework/plugin/reporting/reporting.h"


// Event control struct declarations
EmberEventControl buttonEventControl;

static int8u PGM happyTune[] = {
  NOTE_B4, 1,
  0,       1,
  NOTE_B5, 1,
  0,       0
};
static int8u PGM sadTune[] = {
  NOTE_B5, 1,
  0,       1,
  NOTE_B4, 5,
  0,       0
};
static int8u PGM waitTune[] = {
  NOTE_B4, 1,
  0,       0
};

#define LED BOARDLED1
#define MIN_INTERVAL_S 2
#define MAX_INTERVAL_S 8

static int16u clusterIds[] = {ZCL_ON_OFF_CLUSTER_ID};

void buttonEventHandler(void)
{
  emberEventControlSetInactive(buttonEventControl);
  if (halButtonState(BUTTON0) == BUTTON_PRESSED) {
    EmberNetworkStatus state = emberNetworkState();
    if (state == EMBER_JOINED_NETWORK) {
      emberAfEzmodeClientCommission(emberAfPrimaryEndpoint(),
                                    EMBER_AF_EZMODE_COMMISSIONING_SERVER_TO_CLIENT,
                                    clusterIds,
                                    COUNTOF(clusterIds));
    } else if (state == EMBER_NO_NETWORK) {
      halPlayTune_P(((emberFormAndJoinIsScanning()
                      || (emberAfStartSearchForJoinableNetwork()
                          == EMBER_SUCCESS))
                     ? waitTune
                     : sadTune),
                    TRUE);
    }
  } else if (halButtonState(BUTTON1) == BUTTON_PRESSED) {
    emberAfEzmodeServerCommission(emberAfPrimaryEndpoint());
  }
}

/** @brief Stack Status
 *
 * This function is called by the application framework from the stack status
 * handler.  This callbacks provides applications an opportunity to be
 * notified of changes to the stack status and take appropriate action.  The
 * application should return TRUE if the status has been handled and should
 * not be handled by the application framework.
 *
 * @param status   Ver.: always
 */
boolean emberAfStackStatusCallback(EmberStatus status)
{
  // If we go up or down, let the user know, although the down case shouldn't
  // happen.
  if (status == EMBER_NETWORK_UP) {
    halPlayTune_P(happyTune, TRUE);
  } else if (status == EMBER_NETWORK_DOWN) {
    halPlayTune_P(sadTune, TRUE);
  }
  return FALSE;
}

/** @brief Main Init
 *
 * This function is called from the application’s main function. It gives the
 * application a chance to do any initialization required at system startup.
 * Any code that you would normally put into the top of the application’s
 * main() routine should be put into this function.
        Note: No callback
 * in the Application Framework is associated with resource cleanup. If you
 * are implementing your application on a Unix host where resource cleanup is
 * a consideration, we expect that you will use the standard Posix system
 * calls, including the use of atexit() and handlers for signals such as
 * SIGTERM, SIGINT, SIGCHLD, SIGPIPE and so on. If you use the signal()
 * function to register your signal handler, please mind the returned value
 * which may be an Application Framework function. If the return value is
 * non-null, please make sure that you call the returned function from your
 * handler to avoid negating the resource cleanup of the Application Framework
 * itself.
 *
 */
void emberAfMainInitCallback(void)
{
  EmberAfPluginReportingEntry reportingEntry;
  reportingEntry.direction = EMBER_ZCL_REPORTING_DIRECTION_REPORTED;
  reportingEntry.endpoint = emberAfPrimaryEndpoint();
  reportingEntry.clusterId = ZCL_ON_OFF_CLUSTER_ID;
  reportingEntry.attributeId = ZCL_ON_OFF_ATTRIBUTE_ID;
  reportingEntry.mask = CLUSTER_MASK_SERVER;
  reportingEntry.manufacturerCode = EMBER_AF_NULL_MANUFACTURER_CODE;
  reportingEntry.data.reported.minInterval = MIN_INTERVAL_S;
  reportingEntry.data.reported.maxInterval = MAX_INTERVAL_S;
  reportingEntry.data.reported.reportableChange = 0; // unused
  emberAfPluginReportingConfigureReportedAttribute(&reportingEntry);
}

/** @brief emberAfHalButtonIsrCallback
 *
 *
 */
// Hal Button ISR Callback
// This callback is called by the framework whenever a button is pressed on the
// device. This callback is called within ISR context.
void emberAfHalButtonIsrCallback(int8u button, int8u state)
{
  if ((button == BUTTON0 || button == BUTTON1) && state == BUTTON_PRESSED) {
    emberEventControlSetActive(buttonEventControl);
  }
}

/** @brief Server Init
 *
 * On/off cluster, Server Init
 *
 * @param endpoint Endpoint that is being initialized  Ver.: always
 */
void emberAfOnOffClusterServerInitCallback(int8u endpoint)
{
  // At startup, trigger a read of the attribute and possibly a toggle of the
  // LED to make sure they are always in sync.
  emberAfOnOffClusterServerAttributeChangedCallback(endpoint,
                                                    ZCL_ON_OFF_ATTRIBUTE_ID);
}

/** @brief Server Attribute Changed
 *
 * On/off cluster, Server Attribute Changed
 *
 * @param endpoint Endpoint that is being initialized  Ver.: always
 * @param attributeId Attribute that changed  Ver.: always
 */
void emberAfOnOffClusterServerAttributeChangedCallback(int8u endpoint,
                                                       EmberAfAttributeId attributeId)
{
  // When the on/off attribute changes, set the LED appropriately.  If an error
  // occurs, ignore it because there's really nothing we can do.
  if (attributeId == ZCL_ON_OFF_ATTRIBUTE_ID) {
    boolean onOff;
    if (emberAfReadServerAttribute(endpoint,
                                   ZCL_ON_OFF_CLUSTER_ID,
                                   ZCL_ON_OFF_ATTRIBUTE_ID,
                                   (int8u *)&onOff,
                                   sizeof(onOff))
        == EMBER_ZCL_STATUS_SUCCESS) {
      if (onOff) {
        halSetLed(LED);
      } else {
        halClearLed(LED);
      }
    }
  }
}

/** @brief Client Complete
 *
 * This function is called by the EZ-Mode Commissioning plugin when client
 * commissioning completes.
 *
 * @param bindingIndex The binding index that was created or
 * ::EMBER_NULL_BINDING if an error occurred.  Ver.: always
 */
void emberAfPluginEzmodeCommissioningClientCompleteCallback(int8u bindingIndex)
{
}

/** @brief Get Group Name
 *
 * This function returns the name of a group with the provided group ID,
 * should it exist.
 *
 * @param endpoint Endpoint  Ver.: always
 * @param groupId Group ID  Ver.: always
 * @param groupName Group Name  Ver.: always
 */
void emberAfPluginGroupsServerGetGroupNameCallback(int8u endpoint,
                                                   int16u groupId,
                                                   int8u * groupName)
{
}

/** @brief Set Group Name
 *
 * This function sets the name of a group with the provided group ID.
 *
 * @param endpoint Endpoint  Ver.: always
 * @param groupId Group ID  Ver.: always
 * @param groupName Group Name  Ver.: always
 */
void emberAfPluginGroupsServerSetGroupNameCallback(int8u endpoint,
                                                   int16u groupId,
                                                   int8u * groupName)
{
}

/** @brief Group Names Supported
 *
 * This function returns whether or not group names are supported.
 *
 * @param endpoint Endpoint  Ver.: always
 */
boolean emberAfPluginGroupsServerGroupNamesSupportedCallback(int8u endpoint)
{
  return FALSE;
}

/** @brief Finished
 *
 * This callback is fired when the network-find plugin is finished with the
 * forming or joining process.  The result of the operation will be returned
 * in the status parameter.
 *
 * @param status   Ver.: always
 */
void emberAfPluginNetworkFindFinishedCallback(EmberStatus status)
{
}

/** @brief Get Radio Power For Channel
 *
 * This callback is called by the framework when it is setting the radio power
 * during the discovery process. The framework will set the radio power
 * depending on what is returned by this callback.
 *
 * @param channel   Ver.: always
 */
int8s emberAfPluginNetworkFindGetRadioPowerForChannelCallback(int8u channel)
{
  return EMBER_AF_PLUGIN_NETWORK_FIND_RADIO_TX_POWER;
}

/** @brief Join
 *
 * This callback is called by the plugin when a joinable network has been
 * found.  If the application returns TRUE, the plugin will attempt to join
 * the network.  Otherwise, the plugin will ignore the network and continue
 * searching.  Applications can use this callback to implement a network
 * blacklist.
 *
 * @param networkFound   Ver.: always
 * @param lqi   Ver.: always
 * @param rssi   Ver.: always
 */
boolean emberAfPluginNetworkFindJoinCallback(EmberZigbeeNetwork *networkFound,
                                             int8u lqi,
                                             int8s rssi)
{
  return TRUE;
}

/** @brief Configured
 *
 * This callback is called by the Reporting plugin whenever a reporting entry
 * is configured, including when entries are deleted or updated.  The
 * application can use this callback for scheduling readings or measurements
 * based on the minimum and maximum reporting interval for the entry.  The
 * application should return EMBER_ZCL_STATUS_SUCCESS if it can support the
 * configuration or an error status otherwise.  Note: attribute reporting is
 * required for many clusters and attributes, so rejecting a reporting
 * configuration may violate ZigBee specifications.
 *
 * @param entry   Ver.: always
 */
EmberAfStatus emberAfPluginReportingConfiguredCallback(const EmberAfPluginReportingEntry * entry)
{
  return EMBER_ZCL_STATUS_SUCCESS;
}
