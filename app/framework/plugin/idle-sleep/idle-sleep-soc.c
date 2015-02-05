// Copyright 2013 Silicon Laboratories, Inc.

#include "app/framework/include/af.h"
#include "app/framework/util/af-event.h"
#include "idle-sleep.h"

#if defined(EMBER_AF_HAS_RX_ON_WHEN_IDLE_NETWORK) && defined(EMBER_NO_IDLE_SUPPORT)
  #warning Idling and sleeping is not available on SoCs with RX-on-when-idle networks that do not support idling.
#else
  #define IDLE_AND_SLEEP_SUPPORTED
#endif

#ifdef IDLE_AND_SLEEP_SUPPORTED

enum {
  AWAKE,
  IDLE,
  SLEEP,
};

#ifndef EMBER_AF_HAS_RX_ON_WHEN_IDLE_NETWORK
static int32u fullSleep(int32u sleepDurationMS, int8u eventIndex)
{
  int32u sleepDurationAttemptedMS = sleepDurationMS;

  emAfPrintSleepDuration(sleepDurationMS, eventIndex);

  emberAfEepromShutdownCallback();

  // turn off the radio
  emberStackPowerDown();

  ATOMIC(
    // turn off board and peripherals
    halPowerDown();
    // turn micro to power save mode - wakes on external interrupt
    // or when the time specified expires
    halSleepForMilliseconds(&sleepDurationMS);
    // power up board and peripherals
    halPowerUp();
  );
  // power up radio
  emberStackPowerUp();

  emberAfEepromNoteInitializedStateCallback(FALSE);

  // Allow the stack to time out any of its events and check on its
  // own network state.
  emberTick();

  // Inform the application how long we have slept, sleepDuration has
  // been decremented by the call to halSleepForMilliseconds() by the amount
  // of time that we slept.  To get the actual sleep time we must determine
  // the delta between the amount we asked for and the amount of sleep time
  // LEFT in our request value.
  emberAfDebugPrintln("wakeup %l ms",
                      (sleepDurationAttemptedMS - sleepDurationMS));

  emAfPrintForceAwakeStatus();

  return (sleepDurationAttemptedMS - sleepDurationMS);
}
#endif

int32u emberAfCheckForSleepCallback(void)
{
  if (emberOkToNap() && emAfOkToIdleOrSleep()) {
#ifndef EMBER_AF_HAS_RX_ON_WHEN_IDLE_NETWORK
    // If the stack says not to hibernate, it is because it has events that
    // need to be serviced and therefore we need to consider those events in
    // our sleep calculation.
    int8u nextEventIndex;
    int32u durationMs = emberAfMsToNextEventExtended((emberOkToHibernate()
                                                      ? MAX_INT32U_VALUE
                                                      : emberMsToNextStackEvent()),
                                                     &nextEventIndex);
    if (durationMs != 0
        && emberAfPluginIdleSleepOkToSleepCallback(durationMs)
        && !emberAfPreGoToSleepCallback(durationMs)) { // deprecated
      durationMs = fullSleep(durationMs, nextEventIndex);
      emberAfPluginIdleSleepWakeUpCallback(durationMs);
      emberAfPostWakeUpCallback(durationMs); // deprecated
      return durationMs;
    }
#endif
#ifndef EMBER_NO_IDLE_SUPPORT
    ATOMIC(
      if (emberSerialReadAvailable(APP_SERIAL) == 0
          && emberAfPluginIdleSleepOkToIdleCallback()) {
        if (emberMarkTaskIdle(emAfTaskId)) {
          emberAfPluginIdleSleepActiveCallback();
        }
      }
    )
    emberMarkTaskActive(emAfTaskId);
#endif
  }
  return 0;
}

#else // !IDLE_AND_SLEEP_SUPPORTED

int32u emberAfCheckForSleepCallback(void)
{
  return 0;
}

#endif // IDLE_AND_SLEEP_SUPPORTED
