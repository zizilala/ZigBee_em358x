// *******************************************************************
// * smart-energy-registration-cli.c
// *
// *
// * Copyright 2011 by Ember Corporation. All rights reserved.              *80*
// *******************************************************************

#include "app/framework/include/af.h"
#include "app/util/serial/command-interpreter2.h"
#include "smart-energy-registration.h"

void setRegistrationDelayPeriod(void);

#if !defined(EMBER_AF_GENERATE_CLI)
EmberCommandEntry emberAfPluginSmartEnergyRegistrationCommands[] = {
#ifdef EMBER_AF_PLUGIN_SMART_ENERGY_REGISTRATION_DELAY_PERIOD
  // Allows to set the discovery period (in seconds).
  emberCommandEntryAction("set-period",      setRegistrationDelayPeriod, "w", "Sets the discovery period (in seconds)"),
#endif //EMBER_AF_PLUGIN_SMART_ENERGY_REGISTRATION_DELAY_PERIOD

  emberCommandEntryTerminator(),
};
#endif // EMBER_AF_GENERATE_CLI

void setRegistrationDelayPeriod(void)
{
#ifdef EMBER_AF_PLUGIN_SMART_ENERGY_REGISTRATION_DELAY_PERIOD
  emAfPluginSmartEnergyRegistrationDiscoveryPeriod =
      (int32u)emberUnsignedCommandArgument(0) * MILLISECOND_TICKS_PER_SECOND;

  emberAfAppPrintln("Smart energy registration discovery period set to 0x%4x",
        emAfPluginSmartEnergyRegistrationDiscoveryPeriod);
#else
  emberAfAppPrintln("Not supported.");
#endif //EMBER_AF_PLUGIN_SMART_ENERGY_REGISTRATION_DELAY_PERIOD
}
