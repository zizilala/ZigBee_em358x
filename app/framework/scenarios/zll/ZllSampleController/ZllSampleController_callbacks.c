// Copyright 2014 Silicon Laboratories, Inc.
//
//

// This callback file is created for your convenience. You may add application code
// to this file. If you regenerate this file over a previous version, the previous
// version will be overwritten and any code you have added will be lost.

#include "app/framework/include/af.h"
#include "app/framework/plugin/zll-commissioning/zll-commissioning.h"


// Event control struct declarations
EmberEventControl buttonEventControl;
EmberEventControl identifyEventControl;

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

static int32u identifyDurationMs = 0; // milliseconds
#define DEFAULT_IDENTIFY_DURATION_MS (3 * MILLISECOND_TICKS_PER_SECOND)
#define LED BOARDLED1

#define BUTTON BUTTON0
#define REPAIR_DELAY_MS 10
#define REPAIR_LIMIT_MS (MILLISECOND_TICKS_PER_SECOND << 1)

static boolean on = FALSE;
static int16u buttonPressTime;

extern boolean tuneDone;

void buttonEventHandler(void)
{
  emberEventControlSetInactive(buttonEventControl);

  EmberNetworkStatus state = emberNetworkState();
  if (state == EMBER_JOINED_NETWORK) {
    emberAfFillExternalBuffer((ZCL_CLUSTER_SPECIFIC_COMMAND
                               | ZCL_FRAME_CONTROL_CLIENT_TO_SERVER),
                              ZCL_ON_OFF_CLUSTER_ID,
                              (on ? ZCL_OFF_COMMAND_ID : ZCL_ON_COMMAND_ID),
                              "");
    emberAfGetCommandApsFrame()->profileId           = emberAfProfileIdFromIndex(0);
    emberAfGetCommandApsFrame()->sourceEndpoint      = emberAfEndpointFromIndex(0);
    emberAfGetCommandApsFrame()->destinationEndpoint = EMBER_BROADCAST_ENDPOINT;
    if (emberAfSendCommandBroadcast(EMBER_SLEEPY_BROADCAST_ADDRESS)
        == EMBER_SUCCESS) {
      on = !on;
    } else {
      halPlayTune_P(sadTune, TRUE);
    }
  } else if (state == EMBER_NO_NETWORK) {
    halPlayTune_P((emberAfZllInitiateTouchLink() == EMBER_SUCCESS
                   ? waitTune
                   : sadTune),
                  TRUE);
  } else {
    int16u elapsed = elapsedTimeInt16u(buttonPressTime,
                                       halCommonGetInt16uMillisecondTick());
    if (REPAIR_LIMIT_MS
        < elapsedTimeInt16u(buttonPressTime,
                            halCommonGetInt16uMillisecondTick())) {
      halPlayTune_P(sadTune, TRUE);
    } else {
      if (state == EMBER_JOINED_NETWORK_NO_PARENT
          && !emberStackIsPerformingRejoin()) {
        halPlayTune_P((emberFindAndRejoinNetwork(TRUE, 0) == EMBER_SUCCESS
                       ? waitTune
                       : sadTune),
                      TRUE);
      }
      emberEventControlSetDelayMS(buttonEventControl, REPAIR_DELAY_MS);
    }
  }
}

void identifyEventHandler(void)
{
  if (identifyDurationMs == 0) {
    halClearLed(LED);
    emberEventControlSetInactive(identifyEventControl);
  } else {
    halToggleLed(LED);
    if (identifyDurationMs >= MILLISECOND_TICKS_PER_QUARTERSECOND) {
      identifyDurationMs -= MILLISECOND_TICKS_PER_QUARTERSECOND;
    } else {
      identifyDurationMs = 0;
    }
    emberEventControlSetDelayMS(identifyEventControl,
                                MILLISECOND_TICKS_PER_QUARTERSECOND);
  }
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
  if (button == BUTTON
      && state == BUTTON_PRESSED
      && !emberEventControlGetActive(buttonEventControl)) {
    buttonPressTime = halCommonGetInt16uMillisecondTick();
    emberEventControlSetActive(buttonEventControl);
  }
}

/** @brief Ok To Sleep
 *
 * This function is called by the Idle/Sleep plugin before sleeping.  The
 * application should return TRUE if the device may sleep or FALSE otherwise.
 *
 * @param durationMs The maximum duration in milliseconds that the device will
 * sleep.  Ver.: always
 */
boolean emberAfPluginIdleSleepOkToSleepCallback(int32u durationMs)
{
  return tuneDone;
}

/** @brief Wake Up
 *
 * This function is called by the Idle/Sleep plugin after sleeping.
 *
 * @param durationMs The duration in milliseconds that the device slept.
 * Ver.: always
 */
void emberAfPluginIdleSleepWakeUpCallback(int32u durationMs)
{
}

/** @brief Ok To Idle
 *
 * This function is called by the Idle/Sleep plugin before idling.  The
 * application should return TRUE if the device may idle or FALSE otherwise.
 *
 */
boolean emberAfPluginIdleSleepOkToIdleCallback(void)
{
  return tuneDone;
}

/** @brief Active
 *
 * This function is called by the Idle/Sleep plugin after idling.
 *
 */
void emberAfPluginIdleSleepActiveCallback(void)
{
}

/** @brief Initial Security State
 *
 * This function is called by the ZLL Commissioning plugin to determine the
 * initial security state to be used by the device.  The application must
 * populate the ::EmberZllInitialSecurityState structure with a configuration
 * appropriate for the network being formed, joined, or started.  Once the
 * device forms, joins, or starts a network, the same security configuration
 * will remain in place until the device leaves the network.
 *
 * @param securityState The security configuration to be populated by the
 * application and ultimately set in the stack.  Ver.: always
 */
void emberAfPluginZllCommissioningInitialSecurityStateCallback(EmberZllInitialSecurityState *securityState)
{
  // By default, the plugin will configure the stack for the certification
  // encryption algorithm.  Devices that are certified should instead use the
  // master encryption algorithm and set the appropriate encryption key and
  // pre-configured link key.
}

/** @brief Touch Link Complete
 *
 * This function is called by the ZLL Commissioning plugin when touch linking
 * completes.
 *
 * @param networkInfo The ZigBee and ZLL-specific information about the
 * network and target.  Ver.: always
 * @param deviceInformationRecordCount The number of sub-device information
 * records for the target.  Ver.: always
 * @param deviceInformationRecordList The list of sub-device information
 * records for the target.  Ver.: always
 */
void emberAfPluginZllCommissioningTouchLinkCompleteCallback(const EmberZllNetwork *networkInfo,
                                                            int8u deviceInformationRecordCount,
                                                            const EmberZllDeviceInfoRecord *deviceInformationRecordList)
{
  halPlayTune_P(happyTune, TRUE);
}

/** @brief Touch Link Failed
 *
 * This function is called by the ZLL Commissioning plugin if touch linking
 * fails.
 *
 * @param status The reason the touch link failed.  Ver.: always
 */
void emberAfPluginZllCommissioningTouchLinkFailedCallback(EmberAfZllCommissioningStatus status)
{
  halPlayTune_P(sadTune, TRUE);
}

/** @brief Group Identifier Count
 *
 * This function is called by the ZLL Commissioning plugin to determine the
 * number of group identifiers in use by a specific endpoint on the device.
 * The total number of group identifiers on the device, which are shared by
 * all endpoints, is defined by ::EMBER_ZLL_GROUP_ADDRESSES.
 *
 * @param endpoint The endpoint for which the group identifier count is
 * requested.  Ver.: always
 */
int8u emberAfPluginZllCommissioningGroupIdentifierCountCallback(int8u endpoint)
{
  // Devices with multiple endpoints will have to distribute the group ids
  // available to the device among the individual endpoints.  This application
  // has one endpoint, so it uses all available group ids.
  return EMBER_ZLL_GROUP_ADDRESSES;
}

/** @brief Group Identifier
 *
 * This function is called by the ZLL Commissioning plugin to obtain
 * information about the group identifiers in use by a specific endpoint on
 * the device.  The application should populate the record with information
 * about the group identifier and return TRUE.  If no information is available
 * for the given endpoint and index, the application should return FALSE.
 *
 * @param endpoint The endpoint for which the group identifier is requested.
 * Ver.: always
 * @param index The index of the group on the endpoint.  Ver.: always
 * @param record The group information record.  Ver.: always
 */
boolean emberAfPluginZllCommissioningGroupIdentifierCallback(int8u endpoint,
                                                             int8u index,
                                                             EmberAfPluginZllCommissioningGroupInformationRecord *record)
{
  // Devices with multiple endpoints will have to distribute the group ids
  // available to the device among the individual endpoints.  This application
  // has one endpoint, so it uses all available group ids in order.
  EmberTokTypeStackZllData token;
  emberZllGetTokenStackZllData(&token);
  record->groupId = token.myGroupIdMin + index;
  record->groupType = 0x00; // fixed at zero
  return TRUE;
}

/** @brief Endpoint Information Count
 *
 * This function is called by the ZLL Commissioning plugin to determine the
 * number of remote endpoints controlled by a specific endpoint on the local
 * device.
 *
 * @param endpoint The local endpoint for which the remote endpoint
 * information count is requested.  Ver.: always
 */
int8u emberAfPluginZllCommissioningEndpointInformationCountCallback(int8u endpoint)
{
  // Devices that control remote endpoints individually will have to maintain
  // a list of those endpoints.  This application does not control remote
  // endpoints individually.
  return 0x00;
}

/** @brief Endpoint Information
 *
 * This function is called by the ZLL Commissioning plugin to obtain
 * information about the remote endpoints controlled by a specific endpoint on
 * the local device.  The application should populate the record with
 * information about the remote endpoint and return TRUE.  If no information
 * is available for the given endpoint and index, the application should
 * return FALSE.
 *
 * @param endpoint The local endpoint for which the remote endpoint
 * information is requested.  Ver.: always
 * @param index The index of the remote endpoint information on the local
 * endpoint.  Ver.: always
 * @param record The endpoint information record.  Ver.: always
 */
boolean emberAfPluginZllCommissioningEndpointInformationCallback(int8u endpoint,
                                                                 int8u index,
                                                                 EmberAfPluginZllCommissioningEndpointInformationRecord *record)
{
  // Devices that control remote endpoints individually will have to maintain
  // a list of those endpoints.  This application does not control remote
  // endpoints individually.
  return FALSE;
}

/** @brief Identify
 *
 * This function is called by the ZLL Commissioning plugin to notify the
 * application that it should take an action to identify itself.  This
 * typically occurs when an Identify Request is received via inter-PAN
 * messaging.
 *
 * @param durationS If the duration is zero, the device should exit identify
 * mode.  If the duration is 0xFFFF, the device should remain in identify mode
 * for the default time.  Otherwise, the duration specifies the length of time
 * in seconds that the device should remain in identify mode.  Ver.: always
 */
void emberAfPluginZllCommissioningIdentifyCallback(int16u durationS)
{
  if (durationS != 0) {
    halStackIndicatePresence();
  }
  identifyDurationMs = (durationS == 0xFFFF
                        ? DEFAULT_IDENTIFY_DURATION_MS
                        : durationS * MILLISECOND_TICKS_PER_SECOND);
  emberEventControlSetActive(identifyEventControl);
}

/** @brief Reset To Factory New
 *
 * This function is called by the ZLL Commissioning plugin when a request to
 * reset to factory new is received.  The plugin will leave the network, reset
 * attributes managed by the framework to their default values, and clear the
 * group and scene tables.  The application should preform any other necessary
 * reset-related operations in this callback, including resetting any
 * externally-stored attributes.
 *
 */
void emberAfPluginZllCommissioningResetToFactoryNewCallback(void)
{
}

/** @brief Join
 *
 * This callback is called by the ZLL Commissioning plugin when a joinable
 * network has been found.  If the application returns TRUE, the plugin will
 * attempt to join the network.  Otherwise, the plugin will ignore the network
 * and continue searching.  Applications can use this callback to implement a
 * network blacklist.  Note that this callback is not called during touch
 * linking.
 *
 * @param networkFound   Ver.: always
 * @param lqi   Ver.: always
 * @param rssi   Ver.: always
 */
boolean emberAfPluginZllCommissioningJoinCallback(EmberZigbeeNetwork *networkFound,
                                                  int8u lqi,
                                                  int8s rssi)
{
  return TRUE;
}

/** @brief Poll Completed
 *
 * This function is called by the application framework after a poll is
 * completed.
 *
 * @param status Return status of a completed poll operation  Ver.: always
 */
void emberAfPluginEndDeviceSupportPollCompletedCallback(EmberStatus status)
{
}
