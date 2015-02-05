// *****************************************************************************
// * end-device-support.c
// *
// * Code common to SOC and host to handle managing polling
// *
// * Copyright 2013 Silicon Laboratories, Inc.                              *80*
// *****************************************************************************

#include "app/framework/include/af.h"
#include "callback.h"
#include "app/framework/util/util.h"
#include "app/framework/util/common.h"
#include "app/framework/util/af-event.h"

#include "app/framework/plugin/end-device-support/end-device-support.h"

#if defined(EMBER_SCRIPTED_TEST)
  #include "app/framework/util/af-event-test.h"

  #define EMBER_AF_PLUGIN_END_DEVICE_SUPPORT_LONG_POLL_INTERVAL_SECONDS 15
  #define EMBER_AF_PLUGIN_END_DEVICE_SUPPORT_SHORT_POLL_INTERVAL_SECONDS 1
#endif

// *****************************************************************************
// Globals

typedef struct {
  EmberAfApplicationTask currentAppTasks;
  EmberAfApplicationTask wakeTimeoutBitmask;
  int32u longPollIntervalMs;
  int16u shortPollIntervalMs;
  int16u wakeTimeoutMs;
  int16u lastAppTaskScheduleTime;
  EmberAfEventPollControl pollControl;
} State;
static State states[EMBER_SUPPORTED_NETWORKS];


#if defined(EMBER_AF_PLUGIN_END_DEVICE_SUPPORT_ENABLE_POLL_COMPLETED_CALLBACK)
  #define ENABLE_POLL_COMPLETED_CALLBACK_DEFAULT TRUE
#else
  #define ENABLE_POLL_COMPLETED_CALLBACK_DEFAULT FALSE
#endif
boolean emAfEnablePollCompletedCallback = ENABLE_POLL_COMPLETED_CALLBACK_DEFAULT;

#ifndef EMBER_AF_HAS_END_DEVICE_NETWORK
  #error "End device support only allowed on end devices."
#endif

// *****************************************************************************
// Functions

void emberAfPluginEndDeviceSupportInitCallback(void)
{
  int8u i;
  for (i = 0; i < EMBER_SUPPORTED_NETWORKS; i++) {
    states[i].currentAppTasks = 0;
    states[i].wakeTimeoutBitmask = EMBER_AF_PLUGIN_END_DEVICE_SUPPORT_WAKE_TIMEOUT_BITMASK;
    states[i].longPollIntervalMs = (EMBER_AF_PLUGIN_END_DEVICE_SUPPORT_LONG_POLL_INTERVAL_SECONDS
                                    * MILLISECOND_TICKS_PER_SECOND);
    states[i].shortPollIntervalMs = (EMBER_AF_PLUGIN_END_DEVICE_SUPPORT_SHORT_POLL_INTERVAL_SECONDS
                                     * MILLISECOND_TICKS_PER_SECOND);
    states[i].wakeTimeoutMs = (EMBER_AF_PLUGIN_END_DEVICE_SUPPORT_WAKE_TIMEOUT_SECONDS
                               * MILLISECOND_TICKS_PER_SECOND);
    states[i].lastAppTaskScheduleTime = 0;
    states[i].pollControl = EMBER_AF_LONG_POLL;
  }
}

void emberAfAddToCurrentAppTasksCallback(EmberAfApplicationTask tasks)
{
  if (emAfCurrentNetwork->type == EM_AF_NETWORK_TYPE_ZIGBEE_PRO
      && EMBER_SLEEPY_END_DEVICE <= emAfCurrentNetwork->variant.pro.nodeType) {
    State *state = &states[emberGetCurrentNetwork()];
    state->currentAppTasks |= tasks;
    if (tasks & state->wakeTimeoutBitmask) {
      state->lastAppTaskScheduleTime = halCommonGetInt16uMillisecondTick();
    }
  }
}

void emberAfRemoveFromCurrentAppTasksCallback(EmberAfApplicationTask tasks)
{
  State *state = &states[emberGetCurrentNetwork()];
  state->currentAppTasks &= (~tasks);
}

int32u emberAfGetCurrentAppTasksCallback(void)
{
  State *state = &states[emberGetCurrentNetwork()];
  return state->currentAppTasks;
}

int32u emberAfGetLongPollIntervalMsCallback(void)
{
  State *state = &states[emberGetCurrentNetwork()];
  return state->longPollIntervalMs;
}

int32u emberAfGetLongPollIntervalQsCallback(void)
{
  return (emberAfGetLongPollIntervalMsCallback()
          / MILLISECOND_TICKS_PER_QUARTERSECOND);
}

void emberAfSetLongPollIntervalMsCallback(int32u longPollIntervalMs)
{
  State *state = &states[emberGetCurrentNetwork()];
  state->longPollIntervalMs = longPollIntervalMs;
}

void emberAfSetLongPollIntervalQsCallback(int32u longPollIntervalQs)
{
  emberAfSetLongPollIntervalMsCallback(longPollIntervalQs
                                       * MILLISECOND_TICKS_PER_QUARTERSECOND);
}

int16u emberAfGetShortPollIntervalMsCallback(void)
{
  State *state = &states[emberGetCurrentNetwork()];
  return state->shortPollIntervalMs;
}

int16u emberAfGetShortPollIntervalQsCallback(void)
{
  return (emberAfGetShortPollIntervalMsCallback()
          / MILLISECOND_TICKS_PER_QUARTERSECOND);
}

void emberAfSetShortPollIntervalMsCallback(int16u shortPollIntervalMs)
{
  State *state = &states[emberGetCurrentNetwork()];
  state->shortPollIntervalMs = shortPollIntervalMs;
}

void emberAfSetShortPollIntervalQsCallback(int16u shortPollIntervalQs)
{
  emberAfSetShortPollIntervalMsCallback(shortPollIntervalQs
                                        * MILLISECOND_TICKS_PER_QUARTERSECOND);
}

#ifdef EZSP_HOST
  #define emberOkToLongPoll() TRUE
#endif

int32u emberAfGetCurrentPollIntervalMsCallback(void)
{
  if (emAfCurrentNetwork->type == EM_AF_NETWORK_TYPE_ZIGBEE_PRO
      && EMBER_SLEEPY_END_DEVICE <= emAfCurrentNetwork->variant.pro.nodeType) {
    State *state = &states[emberGetCurrentNetwork()];
    if (elapsedTimeInt16u(state->lastAppTaskScheduleTime,
                          halCommonGetInt16uMillisecondTick())
        > state->wakeTimeoutMs) {
      state->currentAppTasks &= ~state->wakeTimeoutBitmask;
    }
    if (!emberOkToLongPoll()
        || state->currentAppTasks != 0
        || emberAfGetCurrentPollControlCallback() == EMBER_AF_SHORT_POLL) {
      return emberAfGetShortPollIntervalMsCallback();
    }
  }
  return emberAfGetLongPollIntervalMsCallback();
}

int32u emberAfGetCurrentPollIntervalQsCallback(void)
{
  return (emberAfGetCurrentPollIntervalMsCallback()
          / MILLISECOND_TICKS_PER_QUARTERSECOND);
}

int16u emberAfGetWakeTimeoutMsCallback(void)
{
  State *state = &states[emberGetCurrentNetwork()];
  return state->wakeTimeoutMs;
}

int16u emberAfGetWakeTimeoutQsCallback(void)
{
  return (emberAfGetWakeTimeoutMsCallback()
          / MILLISECOND_TICKS_PER_QUARTERSECOND);
}

void emberAfSetWakeTimeoutMsCallback(int16u wakeTimeoutMs)
{
  State *state = &states[emberGetCurrentNetwork()];
  state->wakeTimeoutMs = wakeTimeoutMs;
}

void emberAfSetWakeTimeoutQsCallback(int16u wakeTimeoutQs)
{
  emberAfSetWakeTimeoutMsCallback(wakeTimeoutQs
                                  * MILLISECOND_TICKS_PER_QUARTERSECOND);
}

EmberAfApplicationTask emberAfGetWakeTimeoutBitmaskCallback(void)
{
  State *state = &states[emberGetCurrentNetwork()];
  return state->wakeTimeoutBitmask;
}

void emberAfSetWakeTimeoutBitmaskCallback(EmberAfApplicationTask tasks)
{
  State *state = &states[emberGetCurrentNetwork()];
  state->wakeTimeoutBitmask = tasks;
}

EmberAfEventPollControl emberAfGetCurrentPollControlCallback(void)
{
  int8u networkIndex = emberGetCurrentNetwork();
  EmberAfEventPollControl pollControl = states[networkIndex].pollControl;
#ifdef EMBER_AF_GENERATED_EVENT_CONTEXT
  int8u i;
  for (i = 0; i < emAfAppEventContextLength; i++) {
    EmberAfEventContext *context = &emAfAppEventContext[i];
    if (networkIndex == emberAfNetworkIndexFromEndpoint(context->endpoint)
        && emberEventControlGetActive(*context->eventControl)
        && pollControl < context->pollControl) {
      pollControl = context->pollControl;
    }
  }
#endif
  return pollControl;
}

EmberAfEventPollControl emberAfGetDefaultPollControlCallback(void)
{
  State *state = &states[emberGetCurrentNetwork()];
  return state->pollControl;
}

void emberAfSetDefaultPollControlCallback(EmberAfEventPollControl pollControl)
{
  State *state = &states[emberGetCurrentNetwork()];
  state->pollControl = pollControl;
}
