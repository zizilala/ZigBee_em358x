// File: network-cli.c
//
// Copyright 2009 by Ember Corporation. All rights reserved.                *80*

#include "app/framework/include/af.h"
#include "app/framework/util/af-main.h"
#include "app/util/serial/command-interpreter2.h"
#include "app/util/zigbee-framework/zigbee-device-common.h"
#include "network-cli.h"

//------------------------------------------------------------------------------

void networkJoinCommand(void);
void networkRejoinCommand(void);
void networkFormCommand(void);
void networkExtendedPanIdCommand(void);
void networkLeaveCommand(void);
void networkPermitJoinCommand(void);
void findJoinableNetworkCommand(void);
void findUnusedPanIdCommand(void);
void networkChangeChannelCommand(void);
void networkSetCommand(void);
void networkIdCommand(void);

// TODO: gate this again when we have the mechanism for doing so
// with the generated CLI
//#if defined(EMBER_AF_TC_SWAP_OUT_TEST)
  void networkInitCommand(void);
//#endif

/**
 * @addtogroup cli
 * @{
 */
/**
 * @brief 
 *        <b>network form &lt;channel&gt; &lt;power> &lt;panid></b>
 *        - <i>Form a network on a given channel, with a given Tx Power and Pan Id.</i>
 *           - <i>channel - int8u. The channel on which to form the network.</i>
 *           - <i>power   - int8s. One byte signed value indicating the TX power 
 *                                 That the radio should be set to.</i>
 *           - <i>panId   - int16u. The Pan Id on which to form the network.</i> 
 * 
 *        <b>network join &lt;channel&gt; &lt;power&gt; &lt;panid&gt;</b>
 *        - <i>Join a network on a given channel, with a given TX Power and Pan Id.</i>
 *           - <i>channel - int8u. The channel on which to join the network.</i>
 *           - <i>power   - int8s. One byte signed value indicating the TX power 
 *                                 That the radio should be set to.</i>
 *           - <i>panId   - int16u. The Pan Id on which to join the network.</i>
 *
 *        <b>network rejoin &lt;encrypted&gt;</b>
 *        - <i>Find and rejoin the previous network the device was connected to.</i>
 *           - <i>encrypted - int8u. A single byte boolean 0 or 1 indicating
 *                            whether or not the rejoin should be done with encryption.</i>
 *
 *        <b>network leave</b>
 *        - <i>Leave the current network that the device is connected to.</i>
 *
 *        <b>network pjoin &lt;seconds&gt;</b>
 *        - <i>Turn permit joining on for the amount of time indicated.</i>
 *           - <i>seconds - int8u. A single byte indicating how long the device
 *                         should have permit joining turn on for. A value of
 *                         0xff turns permit join of indefinitely.</i>
 *
 *        <b>network broad-pjoin &lt;seconds&gt;</b>
 *        - <i>Turn permit joining on for the amount of time indicated AND
 *             broadcast a ZDO Mgmt Permit Joining request to all routers.
 *           - <i>seconds - int8u. A single byte indicating how long the device
 *                         should have permit joining turn on for. A value of
 *                         0xff turns permit join of indefinitely.</i>
 *
 *        <b>network extpanid &lt;bytes&gt;</b>
 *        - <i>Write the extended pan id of the device.</i>
 *           - <i>bytes - byte array. An array of bytes which represent
 *                        the extended pan id for the device.</i>
 *
 *        <b>network find unused</b>
 *        - <i>Begins a search for an unused Channel and Pan Id. Will automatically
 *             form a network on the first unused Channel and Pan Id it finds.</i>
 * 
 *        <b>network find joinable</b>
 *        - <i>Begins a search for a joinable network. Will automatically
 *             attempt to join the first network it finds.</i>
 */
#define EMBER_AF_DOXYGEN_CLI__NETWORK_COMMANDS
/** @} END addtogroup */

#ifndef EMBER_AF_GENERATE_CLI

// form and join library ocmmands.
static EmberCommandEntry findCommands[] = {
  {"joinable", findJoinableNetworkCommand, "" },
  {"unused",   findUnusedPanIdCommand, "" },
  { NULL },
};
EmberCommandEntry networkCommands[] = {
  {"form", networkFormCommand, "usv"},
  {"join", networkJoinCommand, "usv"},
  {"rejoin", networkRejoinCommand, "uw"},
  {"leave", networkLeaveCommand, ""},
  {"pjoin", networkPermitJoinCommand, "u"},
  {"broad-pjoin", networkPermitJoinCommand, "u"},
  {"extpanid", networkExtendedPanIdCommand, "b"},
  {"find", NULL, (PGM_P)findCommands},

  emberCommandEntryAction("id",
                          networkIdCommand,
                          "",
                          "Print the current network IDs"),

  {"change-channel", networkChangeChannelCommand, "u"},

#if defined(EMBER_AF_TC_SWAP_OUT_TEST)
  // Do not document this command.
  {"init", networkInitCommand, "" },
#endif

  {"set", networkSetCommand, "u"},

  { NULL }
};

#endif // EMBER_AF_GENERATE_CLI

int8u emAfCliNetworkIndex = EMBER_AF_DEFAULT_NETWORK_INDEX;

//------------------------------------------------------------------------------

void initNetworkParams(EmberNetworkParameters *networkParams)
{
  MEMSET(networkParams, 0, sizeof(EmberNetworkParameters));
  emberAfGetFormAndJoinExtendedPanIdCallback(networkParams->extendedPanId);
  networkParams->radioChannel = (int8u)emberUnsignedCommandArgument(0);
  networkParams->radioTxPower = (int8s)emberSignedCommandArgument(1);
  networkParams->panId = (int16u)emberUnsignedCommandArgument(2);
}

// network join <channel> <power> <panid>
void networkJoinCommand(void)
{
  EmberStatus status;
  EmberNetworkParameters networkParams;
  initNetworkParams(&networkParams);
  status = emberAfJoinNetwork(&networkParams);
  emberAfAppPrintln("%p 0x%x", "join", status);
}

// network rejoin <haveCurrentNetworkKey:1> 
void networkRejoinCommand(void)
{
  boolean haveCurrentNetworkKey = (boolean)emberUnsignedCommandArgument(0);
  int32u channelMask = emberUnsignedCommandArgument(1);
  EmberStatus status = emberFindAndRejoinNetworkWithReason(haveCurrentNetworkKey,
                                                           EMBER_ALL_802_15_4_CHANNELS_MASK,
                                                           EMBER_AF_REJOIN_DUE_TO_CLI_COMMAND);
  emberAfAppPrintln("%p 0x%x", "rejoin", status);
}

// network form <channel> <power> <panid>
void networkFormCommand(void)
{
#ifdef EMBER_AF_HAS_COORDINATOR_NETWORK
  EmberStatus status;
  EmberNetworkParameters networkParams;
  initNetworkParams(&networkParams);
  status = emberAfFormNetwork(&networkParams);
  emberAfAppPrintln("%p 0x%x", "form", status);
  emberAfAppFlush();
#else
  emberAfAppPrintln("only coordinators can form");
#endif
}

// network extpanid <8 BYTES>
void networkExtendedPanIdCommand(void)
{
  int8u extendedPanId[EXTENDED_PAN_ID_SIZE];
  emberAfCopyBigEndianEui64Argument(0, extendedPanId);
  emberAfSetFormAndJoinExtendedPanIdCallback(extendedPanId);
  emberAfAppPrint("ext. PAN ID: ");
  emberAfAppDebugExec(emberAfPrintBigEndianEui64(extendedPanId));
  emberAfAppPrintln("");
}

// network leave
void networkLeaveCommand(void)
{
  EmberStatus status;
  status = emberLeaveNetwork();
  emberAfAppPrintln("%p 0x%x", "leave",  status);
}

// network pjoin <time>
// network broad-pjoin <time>
void networkPermitJoinCommand(void)
{
  int8u duration = (int8u)emberUnsignedCommandArgument(0);
  emAfPermitJoin(duration,
                 ('b' 
                  == emberStringCommandArgument(-1,
                                                NULL)[0]));  // broadcast permit join?
}

void findJoinableNetworkCommand(void)
{
  emberAfStartSearchForJoinableNetwork();
}

void findUnusedPanIdCommand(void)
{
  emberAfFindUnusedPanIdAndForm();
}

void networkChangeChannelCommand(void)
{
  int8u channel = (int8u)emberUnsignedCommandArgument(0);
  EmberStatus status = emberChannelChangeRequest(channel);
  emberAfAppPrintln("Changing to channel %d: 0x%X",
                    channel,
                    status);
}

// TODO: put gating back in after we have a mechanism in place to do so
// for the generated CLI
//#if defined(EMBER_AF_TC_SWAP_OUT_TEST)
void networkInitCommand(void)
{
  EmberNetworkInitStruct networkInitStruct = { 
    EMBER_AF_CUSTOM_NETWORK_INIT_OPTIONS   // EmberNetworkInitBitmask value
  };
  EmberStatus status = emberNetworkInitExtended(&networkInitStruct);
  emberAfAppPrintln("Network Init returned: 0x%X", status);
}
//#endif

void networkSetCommand(void)
{
  int8u index = (int8u)emberUnsignedCommandArgument(0);
  if (EMBER_SUPPORTED_NETWORKS <= index) {
    emberAfCorePrintln("invalid network index");
    return;
  }
  emAfCliNetworkIndex = index;
}

void networkIdCommand(void)
{
  EmberEUI64 eui64;
  emberAfGetEui64(eui64);
  emberAfCorePrint("Short ID: 0x%2X, EUI64: ");
  emberAfPrintBigEndianEui64(eui64);
  emberAfCorePrintln(", Pan ID: 0x%2X", emberAfGetPanId());
}
