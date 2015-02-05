// *****************************************************************************
// * end-device-support-cli.c
// *
// *
// * Copyright 2013 Silicon Laboratories, Inc.                              *80*
// *****************************************************************************


#include "app/framework/include/af.h"
#include "app/framework/util/common.h"
#include "app/util/serial/command-interpreter2.h"
#include "end-device-support.h"


// *****************************************************************************
// Forward declarations

void emberAfPluginEndDeviceSupportStatusCommand(void);
void emberAfPluginEndDeviceSupportPollCompletedCallbackCommand(void);
void emberAfPluginEndDeviceSupportForceShortPollCommand(void);

// *****************************************************************************
// Globals

#ifndef EMBER_AF_GENERATE_CLI
EmberCommandEntry emberAfPluginEndDeviceSupportCommands[] = {
  emberCommandEntryAction("status", emberAfPluginEndDeviceSupportStatusCommand, "",
                          "Display the status of the End Device's polling"),

  emberCommandEntryAction("poll-completed-callback", emberAfPluginEndDeviceSupportPollCompletedCallbackCommand, "u",
                          "Sets whether the device's poll completed callback function is enabled"),

  emberCommandEntryAction("force-short-poll", emberAfPluginEndDeviceSupportForceShortPollCommand,  "u",
                          "Sets whether the CLI forces the device to short poll"),

  emberCommandEntryTerminator()
};
#endif // EMBER_AF_GENERATE_CLI

PGM_P pollControlStrings[] = {
  "EMBER_AF_LONG_POLL",
  "EMBER_AF_SHORT_POLL",
};


// *****************************************************************************
// Functions

void emberAfPluginEndDeviceSupportStatusCommand(void)
{
  PGM_P names[] = {
    EMBER_AF_GENERATED_NETWORK_STRINGS
  };
  int8u i;

  emberAfCorePrintln("End Device Poll Information");
  emberAfCorePrintln("EMBER_END_DEVICE_TIMEOUT:       %d", EMBER_END_DEVICE_POLL_TIMEOUT);
  emberAfCorePrintln("EMBER_END_DEVICE_TIMEOUT_SHIFT: %d", EMBER_END_DEVICE_POLL_TIMEOUT_SHIFT);
  emberAfCorePrintln("Poll completed callback: %p",
                     (emAfEnablePollCompletedCallback
                      ? "yes"
                      : "no"));
  emberAfCoreFlush();

  for (i = 0; i < EMBER_SUPPORTED_NETWORKS; i++) {
    if (emAfNetworks[i].type == EM_AF_NETWORK_TYPE_ZIGBEE_PRO
        && EMBER_END_DEVICE <= emAfNetworks[i].variant.pro.nodeType) {
      emberAfPushNetworkIndex(i);
      emberAfCorePrintln("nwk %d [%p]", i, names[i]);
      emberAfCorePrintln("  Current Poll Interval (qs):   %l",
                         emberAfGetCurrentPollIntervalQsCallback());
      emberAfCorePrintln("  Long Poll Interval (qs):      %l",
                         emberAfGetLongPollIntervalQsCallback());
      if (EMBER_SLEEPY_END_DEVICE <= emAfNetworks[i].variant.pro.nodeType) {
        emberAfCorePrintln("  Short Poll Interval (qs):     %l",
                           emberAfGetShortPollIntervalQsCallback());
        emberAfCoreFlush();
        emberAfCorePrintln("  Wake Timeout (qs):            %l",
                           emberAfGetWakeTimeoutQsCallback());
        emberAfCoreFlush();
        emberAfCorePrintln("  Wake Timeout Bitmask:         0x%4x",
                           emberAfGetWakeTimeoutBitmaskCallback());
        emberAfCoreFlush();
        emberAfCorePrintln("  Current App Tasks:            0x%4x",
                           emberAfGetCurrentAppTasksCallback());
        emberAfCorePrintln("  Current Poll Control          %p",
                           pollControlStrings[emberAfGetCurrentPollControlCallback()]);
        emberAfCoreFlush();
        emberAfCorePrintln("  Default Poll Control          %p",
                           pollControlStrings[emberAfGetDefaultPollControlCallback()]);
        emberAfCoreFlush();
        emberAfPopNetworkIndex();
      }
    }
  }
}

void emberAfPluginEndDeviceSupportForceShortPollCommand(void)
{
  boolean shortPoll = (boolean)emberUnsignedCommandArgument(0);
  if (shortPoll) {
    emberAfCorePrintln("Forcing device to short poll");
    emberAfAddToCurrentAppTasksCallback(EMBER_AF_FORCE_SHORT_POLL);
  } else {
    emberAfCorePrintln("Allowing device to long poll");
    emberAfRemoveFromCurrentAppTasksCallback(EMBER_AF_FORCE_SHORT_POLL);
  }
}

void emberAfPluginEndDeviceSupportPollCompletedCallbackCommand(void)
{
  emAfEnablePollCompletedCallback = (boolean)emberUnsignedCommandArgument(0);
}
