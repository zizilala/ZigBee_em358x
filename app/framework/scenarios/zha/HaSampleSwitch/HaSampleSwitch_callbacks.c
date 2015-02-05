// Copyright 2013 Silicon Laboratories, Inc.

#include "app/framework/include/af.h"
#include "app/framework/plugin/ezmode-commissioning/ez-mode.h"


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

#define REPAIR_DELAY_MS 10
#define REPAIR_LIMIT_MS (MILLISECOND_TICKS_PER_SECOND << 1)

static boolean on = FALSE;
static int16u buttonPressTime;
static int16u clusterIds[] = {ZCL_ON_OFF_CLUSTER_ID};

extern boolean tuneDone;

void buttonEventHandler(void)
{
  emberEventControlSetInactive(buttonEventControl);
  if (halButtonState(BUTTON0) == BUTTON_PRESSED) {
    EmberNetworkStatus state = emberNetworkState();
    if (state == EMBER_JOINED_NETWORK) {
      emberAfFillExternalBuffer((ZCL_CLUSTER_SPECIFIC_COMMAND
                                 | ZCL_FRAME_CONTROL_CLIENT_TO_SERVER),
                                ZCL_ON_OFF_CLUSTER_ID,
                                (on ? ZCL_OFF_COMMAND_ID : ZCL_ON_COMMAND_ID),
                                "");
      emberAfGetCommandApsFrame()->profileId           = emberAfPrimaryProfileId();
      emberAfGetCommandApsFrame()->sourceEndpoint      = emberAfPrimaryEndpoint();
      emberAfGetCommandApsFrame()->destinationEndpoint = EMBER_BROADCAST_ENDPOINT;
      if (emberAfSendCommandUnicastToBindings() == EMBER_SUCCESS) {
        on = !on;
      } else {
        halPlayTune_P(sadTune, TRUE);
      }
    } else if (state == EMBER_NO_NETWORK) {
      halPlayTune_P((emberAfStartSearchForJoinableNetwork() == EMBER_SUCCESS
                     ? waitTune
                     : sadTune),
                    TRUE);
    } else {
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
  } else if (halButtonState(BUTTON1) == BUTTON_PRESSED) {
    emberAfEzmodeClientCommission(emberAfPrimaryEndpoint(),
                                  EMBER_AF_EZMODE_COMMISSIONING_CLIENT_TO_SERVER,
                                  clusterIds,
                                  COUNTOF(clusterIds));
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
  if (status == EMBER_NETWORK_UP) {
    halPlayTune_P(happyTune, TRUE);
  } else if (status == EMBER_NETWORK_DOWN) {
    halPlayTune_P(sadTune, TRUE);
  }
  return FALSE;
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
  if ((button == BUTTON0 || button == BUTTON1)
      && state == BUTTON_PRESSED
      && !emberEventControlGetActive(buttonEventControl)) {
    buttonPressTime = halCommonGetInt16uMillisecondTick();
    emberEventControlSetActive(buttonEventControl);
  }
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
