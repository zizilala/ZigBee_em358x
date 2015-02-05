// Copyright 2013 Silicon Laboratories, Inc.

#include "app/framework/include/af.h"
#include "app/framework/util/af-event.h"
#include "app/framework/util/af-main.h"
#include "idle-sleep.h"

#ifdef EMBER_AF_HAS_RX_ON_WHEN_IDLE_NETWORK
  #warning Idling and sleeping is not available on hosts with RX-on-when-idle networks.
#else
  #define IDLE_AND_SLEEP_SUPPORTED
#endif

#ifdef IDLE_AND_SLEEP_SUPPORTED

static volatile boolean ncpIsAwake = FALSE;

static int32u fullSleep(int32u sleepDurationMS, int8u eventIndex)
{
  EmberStatus status;
  int32u time = halCommonGetInt32uMillisecondTick();
  int32u elapsedSleepTimeMS;
  int8u timerUnit;
  int16u timerDuration;
  int32u timerDurationMS;

  // Get the proper timer duration and units we will use to set our
  // wake timer on the NCP. We are basically converting MS to
  // MS, QS or minutes.
  emAfGetTimerDurationAndUnitFromMS(sleepDurationMS, &timerDuration, &timerUnit);

  // Translate back to MS so that we call the PreGoToSleepCallback
  // with the accurate MS (corrected for rounding due to units)
  timerDurationMS = emAfGetMSFromTimerDurationAndUnit(timerDuration, timerUnit);

  emAfPrintSleepDuration(sleepDurationMS, eventIndex);

  emberAfEepromShutdownCallback();

  // If we have anything waiting in the queue send it
  // before we sleep
  emberSerialWaitSend(EMBER_AF_PRINT_OUTPUT);

  // set the variable that will instruct the stack to sleep after
  // processing the next EZSP command
  ezspSleepMode = EZSP_FRAME_CONTROL_DEEP_SLEEP;

  // Set timer on NCP so we get woken up, this call will
  // also put the NCP to sleep due to sleep mode setting above.
  status = ezspSetTimer(0, timerDuration, timerUnit, FALSE);

  // check that we were able to set the timer and that an
  // ezsp error was not received in the call.
  if (status == EMBER_SUCCESS && !emberAfNcpNeedsReset()) {

    // setting awake flag to false since NCP is sleeping.
    ncpIsAwake = FALSE;
    // this disables interrupts and reenables them once it completes
    ATOMIC(
      halPowerDown();  // turn off board and peripherals
      // Use maintain timer so that we can accurately
      // measure how long we have actually slept and don't
      // have to manage the system time.
      halSleep(SLEEPMODE_MAINTAINTIMER);
      halPowerUp();   // power up board and peripherals
    );
  } else {
    emberAfDebugPrintln("%p: setTimer fail: 0x%X", "Error", status);
  }

  // Make sure the NCP is awake since the host could have been waken up via
  // other external interrupt like button press.  If it's not then wake it.
  if (!ncpIsAwake) {
    ezspWakeUp();
    do {
#ifdef EMBER_TEST
      // Simulation only.
      simulatedTimePasses();
#endif
    } while (!ncpIsAwake);
  }

  ezspSleepMode = EZSP_FRAME_CONTROL_IDLE;

  // Get the amount of time that has actually ellapsed since we went to sleep
  elapsedSleepTimeMS = elapsedTimeInt32u(time, halCommonGetInt32uMillisecondTick());

  emberAfDebugPrintln("wakeup %l ms", elapsedSleepTimeMS);

  emAfPrintForceAwakeStatus();

  return elapsedSleepTimeMS;
}

int32u emberAfCheckForSleepCallback(void)
{
  if (emAfOkToIdleOrSleep()) {
    int8u nextEventIndex;
    int32u durationMs = emberAfMsToNextEventExtended(MAX_TIMER_MILLISECONDS_HOST,
                                                     &nextEventIndex);
    if (durationMs != 0
        && emberAfPluginIdleSleepOkToSleepCallback(durationMs)
        && !emberAfPreGoToSleepCallback(durationMs)) { // deprecated
      durationMs = fullSleep(durationMs, nextEventIndex);
      emberAfPluginIdleSleepWakeUpCallback(durationMs);
      emberAfPostWakeUpCallback(durationMs); // deprecated
      return durationMs;
    }
  }
  return 0;
}

// WARNING:  This executes in ISR context.
void emberAfNcpIsAwakeIsrCallback(void)
{
  ncpIsAwake = TRUE;
}

#else // !IDLE_AND_SLEEP_SUPPORTED

int32u emberAfCheckForSleepCallback(void)
{
  return 0;
}

void emberAfNcpIsAwakeIsrCallback(void)
{
}

#endif // IDLE_AND_SLEEP_SUPPORTED
