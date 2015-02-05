// File: counters-soc.c
//
// Description: implements emberCounterHandler() and keeps a tally
// of the events reported by the stack.  The application must define
// EMBER_APPLICATION_HAS_COUNTER_HANDLER in its configuration header
// to use this file.
//
// Copyright 2013 by Ember Corporation. All rights reserved.                *80*

#include "app/framework/include/af.h"
#include "app/util/serial/command-interpreter2.h"
#include "app/util/common/common.h"
#include "counters.h"
#include "counters-ota.h"

int16u emberCounters[EMBER_COUNTER_TYPE_COUNT];
int16u emberCountersThresholds[EMBER_COUNTER_TYPE_COUNT];
int16u emberMultiNetworkCounters[EMBER_SUPPORTED_NETWORKS]
                                [MULTI_NETWORK_COUNTER_TYPE_COUNT];
static int8u getMultiNetworkCounterIndex(EmberCounterType type);
static void multiNetworkCounterHandler(EmberCounterType type, int8u data);

void emberAfPluginCountersInitCallback(void)
{
  emberAfPluginCountersClear();
  emberAfPluginCountersResetThresholds();
}
 
// Implement the stack callback by simply tallying up the counts.
void emberCounterHandler(EmberCounterType type, int8u data)
{
  //To ensure that we call the counter rollover handler exactly once.
  boolean hasCounterExceededThreshold;

  if(emberCounters[type] < emberCountersThresholds[type])
    hasCounterExceededThreshold = FALSE;
  else
    hasCounterExceededThreshold = TRUE;

  if (emberCounters[type] < 0xFFFF)
    emberCounters[type] += 1;

  if (EMBER_COUNTER_MAC_TX_UNICAST_SUCCESS == type
      || EMBER_COUNTER_MAC_TX_UNICAST_FAILED == type){
    if((emberCounters[EMBER_COUNTER_MAC_TX_UNICAST_RETRY] + data)
      < emberCountersThresholds[EMBER_COUNTER_MAC_TX_UNICAST_RETRY])
      emberCounters[EMBER_COUNTER_MAC_TX_UNICAST_RETRY] += data;
    else
      emberCounters[EMBER_COUNTER_MAC_TX_UNICAST_RETRY] = 
        emberCountersThresholds[EMBER_COUNTER_MAC_TX_UNICAST_RETRY];
  }
  else if (EMBER_COUNTER_APS_DATA_TX_UNICAST_SUCCESS == type
           || EMBER_COUNTER_APS_DATA_TX_UNICAST_FAILED == type){
    if((emberCounters[EMBER_COUNTER_APS_DATA_TX_UNICAST_RETRY] + data) 
      < emberCountersThresholds[EMBER_COUNTER_APS_DATA_TX_UNICAST_RETRY])
      emberCounters[EMBER_COUNTER_APS_DATA_TX_UNICAST_RETRY] += data;
    else
      emberCounters[EMBER_COUNTER_APS_DATA_TX_UNICAST_RETRY] = 
        emberCountersThresholds[EMBER_COUNTER_APS_DATA_TX_UNICAST_RETRY];
  }    
  else if (EMBER_COUNTER_PHY_TO_MAC_QUEUE_LIMIT_REACHED == type) {
    if((emberCounters[EMBER_COUNTER_PHY_TO_MAC_QUEUE_LIMIT_REACHED] + data) 
       < emberCountersThresholds[EMBER_COUNTER_PHY_TO_MAC_QUEUE_LIMIT_REACHED])
      emberCounters[type] += data;
    else
      emberCounters[EMBER_COUNTER_APS_DATA_TX_UNICAST_RETRY] =
        emberCountersThresholds[EMBER_COUNTER_APS_DATA_TX_UNICAST_RETRY];
  }

  if(emberCounters[type]>=emberCountersThresholds[type]){
    if(!hasCounterExceededThreshold){
      emberAfPluginCountersRolloverCallback(type);
      hasCounterExceededThreshold = TRUE;
    }
  }

  multiNetworkCounterHandler(type, data);
}

void emberAfPluginCountersClear(void)
{
  MEMSET(emberCounters, 0, sizeof(emberCounters));
}

void emberAfPluginCountersResetThresholds(void)
{
  MEMSET(emberCountersThresholds,0xFF,sizeof(emberCountersThresholds));  
}

void emberAfPluginCountersSetThreshold(EmberCounterType type, int16u threshold)
{
  emberCountersThresholds[type] = threshold;
}

/*******************************************************************************
 * Multi-network counters support
 *
 * Some of the counters are per-network. Some of them are implicitly not
 * per-network because of the limited multi-network support. i.e., a node can be
 * coordinator/router/end device on just one network. The per-network counters
 * are defined in a table. The per-network counters are stored in a separate
 * two-dimensional array. We keep writing the multi-network counters also in the
 * usual single-network counters array.
 ******************************************************************************/
extern int8u emSupportedNetworks;

static PGM EmberCounterType multiNetworkCounterTable[] = {
    EMBER_COUNTER_MAC_RX_BROADCAST,
    EMBER_COUNTER_MAC_TX_BROADCAST,
    EMBER_COUNTER_MAC_RX_UNICAST,
    EMBER_COUNTER_MAC_TX_UNICAST_SUCCESS,
    EMBER_COUNTER_MAC_TX_UNICAST_RETRY,
    EMBER_COUNTER_MAC_TX_UNICAST_FAILED,
    EMBER_COUNTER_APS_DATA_RX_BROADCAST,
    EMBER_COUNTER_APS_DATA_TX_BROADCAST,
    EMBER_COUNTER_APS_DATA_RX_UNICAST,
    EMBER_COUNTER_APS_DATA_TX_UNICAST_SUCCESS,
    EMBER_COUNTER_APS_DATA_TX_UNICAST_RETRY,
    EMBER_COUNTER_APS_DATA_TX_UNICAST_FAILED,
    EMBER_COUNTER_NWK_FRAME_COUNTER_FAILURE,
    EMBER_COUNTER_APS_FRAME_COUNTER_FAILURE,
    EMBER_COUNTER_APS_LINK_KEY_NOT_AUTHORIZED,
    EMBER_COUNTER_NWK_DECRYPTION_FAILURE,
    EMBER_COUNTER_APS_DECRYPTION_FAILURE,
};

static int8u getMultiNetworkCounterIndex(EmberCounterType type)
{
  int8u i;
  for(i=0; i<MULTI_NETWORK_COUNTER_TYPE_COUNT; i++) {
    if (multiNetworkCounterTable[i] == type)
      return i;
  }
  return 0xFF;
}

static void multiNetworkCounterHandler(EmberCounterType type, int8u data)
{
  int8u counterIndex = getMultiNetworkCounterIndex(type);

  // This function is always called in a callback context emberCounterHandler().
  // Not a multi-network counter, nothing to do.
  if (counterIndex == 0xFF)
    return;

  int8u nwkIndex = emberGetCallbackNetwork();
  assert(nwkIndex < emSupportedNetworks);

  if (emberMultiNetworkCounters[nwkIndex][counterIndex] < 0xFFFF)
    emberMultiNetworkCounters[nwkIndex][counterIndex] += 1;

  if (EMBER_COUNTER_MAC_TX_UNICAST_SUCCESS == type
       || EMBER_COUNTER_MAC_TX_UNICAST_FAILED == type)
    emberMultiNetworkCounters[nwkIndex][EMBER_COUNTER_MAC_TX_UNICAST_RETRY]
                                        += data;
  else if (EMBER_COUNTER_APS_DATA_TX_UNICAST_SUCCESS == type
      || EMBER_COUNTER_APS_DATA_TX_UNICAST_FAILED == type) {
    emberMultiNetworkCounters[nwkIndex][EMBER_COUNTER_APS_DATA_TX_UNICAST_RETRY]
                                        += data;
  }
}

