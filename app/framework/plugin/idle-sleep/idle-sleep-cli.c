// Copyright 2013 Silicon Laboratories, Inc.

#include "app/framework/include/af.h"
#include "idle-sleep.h"

#ifndef EMBER_AF_GENERATE_CLI
  #error The Idle/Sleep plugin is not compatible with the legacy CLI.
#endif

void emberAfPluginIdleSleepStatusCommand(void)
{
  emberAfCorePrintln("Stay awake when not joined: %p",
                     (emAfStayAwakeWhenNotJoined ? "yes" : "no"));
  emberAfCorePrintln("Forced stay awake:          %p",
                     (emAfForceEndDeviceToStayAwake ? "yes" : "no"));
}

void emberAfPluginIdleSleepStayAwakeCommand(void)
{
  boolean stayAwake = (boolean)emberUnsignedCommandArgument(0);
  if (stayAwake) {
    emberAfCorePrintln("Forcing device to stay awake");
  } else {
    emberAfCorePrintln("Allowing device to go to sleep");
  }
  emberAfForceEndDeviceToStayAwake(stayAwake);
}

void emberAfPluginIdleSleepAwakeWhenNotJoinedCommand(void)
{
  emAfStayAwakeWhenNotJoined = (boolean)emberUnsignedCommandArgument(0);
}
