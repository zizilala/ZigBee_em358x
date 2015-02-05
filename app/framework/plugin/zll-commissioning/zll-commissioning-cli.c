// *******************************************************************
// * zll-commissioning-cli.c
// *
// *
// * Copyright 2010 by Ember Corporation. All rights reserved.              *80*
// *******************************************************************

#include "app/framework/include/af.h"
#include "app/util/serial/command-interpreter2.h"
#include "app/framework/plugin/zll-commissioning/zll-commissioning.h"

// ZLL commissioning commands.
void formNetwork(void);
void initiateTouchLink(void);
void scanTouchLink(void);

// ZLL ZCL commands.
void endpointInformation(void);
void getGroupIdentifiersRequest(void);
void getEndpointListRequest(void);

// ZLL misc commands
void printChannels(void);
void printZllTokens(void);
void setScanChannel(void);
void setScanMask(void);
void setIdentifyDuration(void);
void statusCommand(void);

// ZLL network interoperability commands.
void joinable(void);
void unused(void);

#ifndef EMBER_AF_GENERATE_CLI
EmberCommandEntry emberAfPluginZllCommissioningCommands[] = {
  emberCommandEntryAction("form",      formNetwork,                 "usv",  "Forms a ZLL network"),
  emberCommandEntryAction("link",      initiateTouchLink,           "?",    "Initiates a touch link"),
  emberCommandEntryAction("abort",     emberAfZllAbortTouchLink,    "",     "Aborts a previously initiated touch link"),
  emberCommandEntryAction("info",      endpointInformation,         "vuu",  "Sends an Endpoint Information request"),
  emberCommandEntryAction("groups",    getGroupIdentifiersRequest,  "vuuu", "Sends a Group Identifier Request"),
  emberCommandEntryAction("endpoints", getEndpointListRequest,      "vuuu", "Sends a Get Endpoint List request"),
  emberCommandEntryAction("tokens",    printZllTokens,              "",     "Prints the internal ZLL tokens"),
  emberCommandEntryAction("channel",   setScanChannel,              "u",    "Sets the scan channel"),
  emberCommandEntryAction("mask",      setScanMask,                 "u",    "Sets the scan channel mask"),
  emberCommandEntryAction("identify",  setIdentifyDuration,         "v",    "Sets the identify duration"),
  emberCommandEntryAction("status",    statusCommand,               "",     "Prints the ZLL status"),
  emberCommandEntryAction("joinable",  joinable,                    "",     "Attempts to join any Zigbee network"),
  emberCommandEntryAction("unused",    unused,                      "",     "Attempt to form on an unused PAN ID"),
  emberCommandEntryAction("reset",     emberAfZllResetToFactoryNew, "",     "Resets the local device to factory new"),
  emberCommandEntryTerminator(),
};
#endif //  EMBER_AF_GENERATE_CLI

static int32u channelMasks[] = {
  0x02108800UL, // standard (11, 15, 20, 25)
  0x04211000UL, // +1 (12, 16, 21, 26)
  0x004A2000UL, // +2 (13, 17, 22, 19)
  0x01844000UL, // +3 (14, 18, 23, 24)
  0x07FFF800UL, // all (11--26)
};

// plugin zll-commissioning form <channel:1> <power:1> <pan id:2>
void formNetwork(void)
{
  EmberStatus status = emAfZllFormNetwork((int8u)emberUnsignedCommandArgument(0),       // channel
                                          (int8s)emberSignedCommandArgument(1),         // power
                                          (EmberPanId)emberUnsignedCommandArgument(2)); // pan id
  emberAfAppPrintln("%p 0x%x", "form", status);
}

// Leaving intact for legacy CLI purposes, but for generated CLI, the options have been moved to
// scanTouchLink().

// plugin zll-commissioning link [device|identify|reset]
void initiateTouchLink(void)
{
  EmberStatus status;
  switch (emberCommandArgumentCount()) {
  case 0:
    status = emberAfZllInitiateTouchLink();
    emberAfAppPrintln("%p 0x%x", "touch link", status);
    return;
  case 1:
    // -1 because we're smarter than command-interpreter2.
    switch (emberStringCommandArgument(0, NULL)[-1]) {
    case 'd':
      status = emberAfZllDeviceInformationRequest();
      emberAfAppPrintln("%p 0x%x", "device information", status);
      return;
    case 'i':
      status = emberAfZllIdentifyRequest();
      emberAfAppPrintln("%p 0x%x", "identify", status);
      return;
    case 'r':
      status = emberAfZllResetToFactoryNewRequest();
      emberAfAppPrintln("%p 0x%x", "reset to factory new", status);
      return;
    }
  }
  emberAfAppPrintln("Usage:");
  emberAfAppPrintln("plugin zll-commissioning link");
  emberAfAppPrintln("plugin zll-commissioning link device");
  emberAfAppPrintln("plugin zll-commissioning link identify");
  emberAfAppPrintln("plugin zll-commissioning link reset");
}

// Generated CLI version of "link" commands

// plugin zll-commissioning scan [device|identify|reset]
void scanTouchLink(void)
{
  EmberStatus status;
  switch (emberStringCommandArgument(-1, NULL)[0]) {
    case 'd':
      status = emberAfZllDeviceInformationRequest();
      emberAfAppPrintln("%p 0x%x", "device information", status);
      return;
    case 'i':
      status = emberAfZllIdentifyRequest();
      emberAfAppPrintln("%p 0x%x", "identify", status);
      return;
    case 'r':
      status = emberAfZllResetToFactoryNewRequest();
      emberAfAppPrintln("%p 0x%x", "reset to factory new", status);
      return;
  }
}

// plugin zll-commissioning info <destination:2> <src endpoint:1> <dst endpoint:1>
void endpointInformation(void)
{
  EmberEUI64 eui64;
  EmberStatus status = EMBER_INVALID_ENDPOINT;
  int8u endpoint = (int8u)emberUnsignedCommandArgument(1);
  int8u index = emberAfIndexFromEndpoint(endpoint);
  if (index != 0xFF) {
    emberAfGetEui64(eui64);
    emberAfFillCommandZllCommissioningClusterEndpointInformation(eui64,
                                                                 emberAfGetNodeId(),
                                                                 endpoint,
                                                                 EMBER_ZLL_PROFILE_ID,
                                                                 emberAfDeviceIdFromIndex(index),
                                                                 emberAfDeviceVersionFromIndex(index));
    emberAfSetCommandEndpoints(endpoint,
                               (int8u)emberUnsignedCommandArgument(2));
    status = emberAfSendCommandUnicast(EMBER_OUTGOING_DIRECT,
                                       (int16u)emberUnsignedCommandArgument(0));
  }
  emberAfAppPrintln("%p 0x%x", "endpoint information", status);
}

// plugin zll-commissioning groups <destination:2> <src endpoint:1> <dst endpoint:1> <startIndex:1>
void getGroupIdentifiersRequest(void)
{
  EmberStatus status = EMBER_INVALID_ENDPOINT;
  int8u endpoint = (int8u)emberUnsignedCommandArgument(1);
  int8u index = emberAfIndexFromEndpoint(endpoint);
  if (index != 0xFF) {
    emberAfFillCommandZllCommissioningClusterGetGroupIdentifiersRequest((int8u)emberUnsignedCommandArgument(3));
    emberAfSetCommandEndpoints(endpoint,
                               (int8u)emberUnsignedCommandArgument(2));
    status = emberAfSendCommandUnicast(EMBER_OUTGOING_DIRECT,
                                       (int16u)emberUnsignedCommandArgument(0));
  }
  emberAfAppPrintln("%p 0x%x", "get group identifiers", status);
}

// plugin zll-commissioning endpoints <destination:2> <src endpoint:1> <dst endpoint:1> <startIndex:1>
void getEndpointListRequest(void)
{
  EmberStatus status = EMBER_INVALID_ENDPOINT;
  int8u endpoint = (int8u)emberUnsignedCommandArgument(1);
  int8u index = emberAfIndexFromEndpoint(endpoint);
  if (index != 0xFF) {
    emberAfFillCommandZllCommissioningClusterGetEndpointListRequest((int8u)emberUnsignedCommandArgument(3));
    emberAfSetCommandEndpoints(endpoint,
                               (int8u)emberUnsignedCommandArgument(2));
    status = emberAfSendCommandUnicast(EMBER_OUTGOING_DIRECT,
                                       (int16u)emberUnsignedCommandArgument(0));
  }
  emberAfAppPrintln("%p 0x%x", "get endpoint list", status);
}


void printZllTokens(void)
{
  EmberTokTypeStackZllData token;
  EmberTokTypeStackZllSecurity security;
  emberZllGetTokenStackZllData(&token);
  emberZllGetTokenStackZllSecurity(&security);

  emberAfAppFlush();
  emberAfAppPrintln("ZLL Tokens");
  emberAfAppPrintln("Bitmask: 0x%4x", token.bitmask);
  emberAfAppPrintln("Free Node IDs,  Min: 0x%2x, Max: 0x%2x",
                    token.freeNodeIdMin,
                    token.freeNodeIdMax);
  emberAfAppFlush();
  emberAfAppPrintln("Free Group IDs, Min: 0x%2x, Max: 0x%2x",
                    token.freeGroupIdMin,
                    token.freeGroupIdMax);
  emberAfAppFlush();
  emberAfAppPrintln("My Group ID Min: 0x%2x",
                    token.myGroupIdMin);
  emberAfAppFlush();
  emberAfAppPrintln("RSSI Correction: %d",
                    token.rssiCorrection);
  emberAfAppFlush();

  emberAfAppPrintln("Security Bitmask: 0x%4x", security.bitmask);
  emberAfAppFlush();
  emberAfAppPrintln("Security Key Index: %d", security.keyIndex);
  emberAfAppFlush();
  emberAfAppPrint("Security Encryption Key: ");
  emberAfAppDebugExec(emberAfPrintZigbeeKey(security.encryptionKey));
  emberAfAppPrintln("");
  emberAfAppFlush();
}

void setScanChannel(void)
{
  int8u channel = (int8u)emberUnsignedCommandArgument(0);
  if (channel > EMBER_MAX_802_15_4_CHANNEL_NUMBER
      || channel < EMBER_MIN_802_15_4_CHANNEL_NUMBER) {
    emberAfAppPrintln("Invalid channel %d", channel);
    return;
  }
  emAfZllPrimaryChannelMask = 1 << channel;
  printChannels();
}

void setScanMask(void)
{
  int8u index = (int8u)emberUnsignedCommandArgument(0);
  if (COUNTOF(channelMasks) <= index) {
    emberAfAppPrintln("Invalid channel mask index %d", index);
    return;
  }
  emAfZllPrimaryChannelMask = channelMasks[index];
  printChannels();
}

void setIdentifyDuration(void)
{
#ifdef EMBER_AF_PLUGIN_ZLL_COMMISSIONING_LINK_INITIATOR
  emAfZllIdentifyDurationS = (int16u)emberUnsignedCommandArgument(0);
#endif //EMBER_AF_PLUGIN_ZLL_COMMISSIONING_LINK_INITIATOR
}

void statusCommand(void)
{
  printChannels();
  printZllTokens();
}

void printChannels(void)
{
  emberAfAppPrint("%p channels: ", "Primary");
  emberAfAppDebugExec(emberAfPrintChannelListFromMask(emAfZllPrimaryChannelMask));
  emberAfAppPrintln(" (0x%4x)", emAfZllPrimaryChannelMask);
#ifdef EMBER_AF_PLUGIN_ZLL_COMMISSIONING_SCAN_SECONDARY_CHANNELS
  emberAfAppPrint("%p channels: ", "Secondary");
  emberAfAppDebugExec(emberAfPrintChannelListFromMask(emAfZllSecondaryChannelMask));
  emberAfAppPrintln(" (0x%4x)", emAfZllSecondaryChannelMask);
#endif
}

void joinable(void)
{
  EmberStatus status = emberAfStartSearchForJoinableNetworkCallback();
  emberAfAppPrintln("%p 0x%x", "joinable", status);
}

void unused(void)
{
  EmberStatus status = emberAfFindUnusedPanIdAndFormCallback();
  emberAfAppPrintln("%p 0x%x", "unused", status);
}
