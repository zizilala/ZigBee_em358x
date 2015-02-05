// *******************************************************************
// * simple-metering-server-cli.c
// *
// *
// * Copyright 2012 by Ember Corporation. All rights reserved.              *80*
// *******************************************************************

#include "app/framework/include/af.h"
#include "app/util/serial/command-interpreter2.h"


#ifdef EMBER_AF_PLUGIN_SIMPLE_METERING_SERVER_TEST_METER_ENABLE
#include "app/framework/plugin/simple-metering-server/simple-metering-test.h"
static int8u getEndpointArgument(int8u index);
#endif 


#if !defined(EMBER_AF_GENERATE_CLI)

void emAfPluginSimpleMeteringServerCliPrint(void);
void emAfPluginSimpleMeteringServerCliRate(void);
void emAfPluginSimpleMeteringServerCliVariance(void);
void emAfPluginSimpleMeteringServerCliAdjust(void);
void emAfPluginSimpleMeteringServerCliOff(void);

void emAfPluginSimpleMeteringServerCliElectric(void);
void emAfPluginSimpleMeteringServerCliGas(void);

#if defined(EMBER_AF_PLUGIN_SIMPLE_METERING_SERVER_TEST_METER_ERRORS)
void emAfPluginSimpleMeteringServerCliRandomError(void);
void emAfPluginSimpleMeteringServerCliSetError(void);
#endif //EMBER_AF_PLUGIN_SIMPLE_METERING_SERVER_TEST_METER_ERRORS

void emAfPluginSimpleMeteringServerCliProfiles(void);

EmberCommandEntry emberAfPluginSimpleMeteringServerCommands[] = {
#ifdef EMBER_AF_PLUGIN_SIMPLE_METERING_SERVER_TEST_METER_ENABLE
  emberCommandEntryAction("print", emAfPluginSimpleMeteringServerCliPrint, "", ""),
  emberCommandEntryAction("rate", emAfPluginSimpleMeteringServerCliRate, "v", ""),
  emberCommandEntryAction("variance", emAfPluginSimpleMeteringServerCliVariance, "v", ""),
  emberCommandEntryAction("adjust", emAfPluginSimpleMeteringServerCliAdjust, "u", ""),
  emberCommandEntryAction("off", emAfPluginSimpleMeteringServerCliOff, "u", ""),
  emberCommandEntryAction("electric", emAfPluginSimpleMeteringServerCliElectric, "u", ""),
  emberCommandEntryAction("gas", emAfPluginSimpleMeteringServerCliGas, "u", ""),
#ifdef EMBER_AF_PLUGIN_SIMPLE_METERING_SERVER_TEST_METER_ERRORS
  emberCommandEntryAction("rnd_error", emAfPluginSimpleMeteringServerCliRandomError, "u", ""),
  emberCommandEntryAction("set_error", emAfPluginSimpleMeteringServerCliSetError, "uu", ""),
#endif
  emberCommandEntryAction("profiles", emAfPluginSimpleMeteringServerCliProfiles, "u", ""),
#endif //EMBER_AF_PLUGIN_SIMPLE_METERING_SERVER_TEST_METER_ENABLE
  emberCommandEntryTerminator(),
};

#endif // EMBER_AF_GENERATE_CLI

#ifdef EMBER_AF_PLUGIN_SIMPLE_METERING_SERVER_TEST_METER_ENABLE
static int8u getEndpointArgument(int8u index)
{
  int8u endpoint = (int8u)emberUnsignedCommandArgument(index);
  return (endpoint == 0
          ? emberAfPrimaryEndpointForCurrentNetworkIndex()
          : endpoint);
}

// plugin simple-metering-server print
void emAfPluginSimpleMeteringServerCliPrint(void) 
{
  afTestMeterPrint();
}

// plugin simple-metering-server rate <int:2>
void emAfPluginSimpleMeteringServerCliRate(void)
{
  afTestMeterSetConsumptionRate((int16u)emberUnsignedCommandArgument(0));
}

// plugin simple-metering-server variance <int:2>
void emAfPluginSimpleMeteringServerCliVariance(void)
{
  afTestMeterSetConsumptionVariance((int16u)emberUnsignedCommandArgument(0));
}

// plugin simple-metering-server adjust <endpoint:1>
void emAfPluginSimpleMeteringServerCliAdjust(void)
{
  afTestMeterAdjust(getEndpointArgument(0));
}

// plugin simple-metering-server off <endpoint: 1>
void emAfPluginSimpleMeteringServerCliOff(void)
{
  afTestMeterMode(getEndpointArgument(0), 0);
}

// plugin simple-metering-server electric <endpoint:1>
void emAfPluginSimpleMeteringServerCliElectric(void)
{
#if (EMBER_AF_PLUGIN_SIMPLE_METERING_SERVER_TEST_METER_TYPE == EMBER_AF_PLUGIN_SIMPLE_METERING_SERVER_ELECTRIC_METER)
  afTestMeterMode(getEndpointArgument(0), 1);
#else
  emberAfCorePrintln("Not applicable for a non Electric Meter.");
#endif
}

// plugin simple-metering-server gas <endpoint:1>
void emAfPluginSimpleMeteringServerCliGas(void)
{
#if (EMBER_AF_PLUGIN_SIMPLE_METERING_SERVER_TEST_METER_TYPE == EMBER_AF_PLUGIN_SIMPLE_METERING_SERVER_GAS_METER)
  afTestMeterMode(getEndpointArgument(0), 2);
#else
  emberAfCorePrintln("Not applicable for a non Gas Meter.");
#endif
}

#ifdef EMBER_AF_PLUGIN_SIMPLE_METERING_SERVER_TEST_METER_ERRORS
// plugin simple-metering-server rnd_error <data:1>
void emAfPluginSimpleMeteringServerCliRandomError(void)
{
  // enables random error setting at each tick
  afTestMeterRandomError((int8u)emberUnsignedCommandArgument(0));  
}

// plugin simple-metering-server set_error <data:1> <endpoint:1>
void emAfPluginSimpleMeteringServerCliSetError(void)
{
  // sets error, in the process overriding random_error
  afTestMeterSetError(getEndpointArgument(1), 
                      (int8u)emberUnsignedCommandArgument(0));                           
}
#endif //EMBER_AF_PLUGIN_SIMPLE_METERING_SERVER_TEST_METER_ERRORS

// plugin simple-metering-server profiles <data:1>
void emAfPluginSimpleMeteringServerCliProfiles(void)
{
#if (EMBER_AF_PLUGIN_SIMPLE_METERING_SERVER_TEST_METER_PROFILES != 0)
  afTestMeterEnableProfiles((int8u)emberUnsignedCommandArgument(0));
#else
  emberAfCorePrintln("Not applicable for 0 configured profiles.");
#endif
}
#endif //EMBER_AF_PLUGIN_SIMPLE_METERING_SERVER_TEST_METER_ENABLE
