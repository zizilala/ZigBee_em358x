// *****************************************************************************
// * concentrator-support.c
// *
// * Code common to SOC and host to handle periodically broadcasting
// * many-to-one route requests (MTORRs).
// *
// * Copyright 2012 by Ember Corporation. All rights reserved.              *80*
// *****************************************************************************

#include "app/framework/include/af.h"
#include "app/util/source-route-common.h"
#include "concentrator-support.h"
// *****************************************************************************
// Globals

static int8u routeErrorCount = 0;
static int8u deliveryFailureCount = 0;

#define MIN_QS (EMBER_AF_PLUGIN_CONCENTRATOR_MIN_TIME_BETWEEN_BROADCASTS_SECONDS << 2)
#define MAX_QS (EMBER_AF_PLUGIN_CONCENTRATOR_MAX_TIME_BETWEEN_BROADCASTS_SECONDS << 2)

#if (MIN_QS > MAX_QS)
  #error "Minimum broadcast time must be less than max (EMBER_PLUGIN_CONCENTRATOR_MIN_TIME_BETWEEN_BROADCASTS_SECONDS < EMBER_PLUGIN_CONCENTRATOR_MAX_TIME_BETWEEN_BROADCASTS_SECONDS)"
#endif

// Handy values to make the code more readable.
#define USE_MIN_TIME TRUE
#define USE_MAX_TIME FALSE

EmberEventControl emberAfPluginConcentratorUpdateEventControl;

// Use a shorter name to make the code more readable
#define myEvent emberAfPluginConcentratorUpdateEventControl

#ifndef EMBER_AF_HAS_ROUTER_NETWORK
  #error "Concentrator support only allowed on routers and coordinators."
#endif

//This is used to store the sourceRouteOverhead to our last sender
//It defaults to 0xFF if no valid sourceRoute is found. When available, it 
//is used once to prevent the overhead of calling ezspGetSourceRouteOverhead()
//and cleared subsequently. 
#if defined(EZSP_HOST) && defined(EMBER_AF_PLUGIN_CONCENTRATOR_NCP_SUPPORT)
  static EmberNodeId targetId          = EMBER_UNKNOWN_NODE_ID;
  static int8u sourceRouteOverhead     = EZSP_SOURCE_ROUTE_OVERHEAD_UNKNOWN;
#endif

// *****************************************************************************
// Functions

static int32u queueRouteDiscovery(boolean useMinTime)
{
  int32u timeLeftQS = (useMinTime ? MIN_QS : MAX_QS); 

  // NOTE:  Since timeToExecute is always in MS we must convert MIN_QS => MIN_MS
  if (useMinTime 
      && (myEvent.timeToExecute > 0)
      && (myEvent.timeToExecute < (MIN_QS * 250))) {

    // Do nothing.  Our queued event will fire shortly.
    // We don't want to reset its time and actually delay
    // when it will fire.
    timeLeftQS = myEvent.timeToExecute >> 2;

  } else {
    emberEventControlSetDelayQS(myEvent,
                                timeLeftQS);
  }
   
  // Tell the caller we have approximately 1 quarter second left
  // even though we actually have less than that.  This lets them plan their
  // for events that are waiting to fire based on the MTORR.
  return (timeLeftQS > 0
          ? timeLeftQS
          : 1);
}

int32u emberAfPluginConcentratorQueueDiscovery(void)
{
  return queueRouteDiscovery(USE_MIN_TIME);
}

void emberAfPluginConcentratorInitCallback(void)
{
  sourceRouteInit();
#if (!defined(EZSP_HOST) || !defined(EMBER_AF_PLUGIN_CONCENTRATOR_NCP_SUPPORT))
    queueRouteDiscovery(USE_MAX_TIME);
#endif
}

void emberAfPluginConcentratorNcpInitCallback(void)
{
#if defined(EZSP_HOST) && defined(EMBER_AF_PLUGIN_CONCENTRATOR_NCP_SUPPORT)
    ezspSetConcentrator(TRUE,
                        EMBER_AF_PLUGIN_CONCENTRATOR_CONCENTRATOR_TYPE,
                        EMBER_AF_PLUGIN_CONCENTRATOR_MIN_TIME_BETWEEN_BROADCASTS_SECONDS,
                        EMBER_AF_PLUGIN_CONCENTRATOR_MAX_TIME_BETWEEN_BROADCASTS_SECONDS,
                        EMBER_AF_PLUGIN_CONCENTRATOR_ROUTE_ERROR_THRESHOLD,
                        EMBER_AF_PLUGIN_CONCENTRATOR_DELIVERY_FAILURE_THRESHOLD,
                        EMBER_AF_PLUGIN_CONCENTRATOR_MAX_HOPS);
#endif
}

void emberAfPluginConcentratorStopDiscovery(void)
{
  emberEventControlSetInactive(myEvent);
  myEvent.timeToExecute = 0;
  emberAfCorePrintln("Concentrator advertisements stopped."); 
}

static void routeErrorCallback(EmberStatus status)
{
  if (status == EMBER_SOURCE_ROUTE_FAILURE 
      || status == EMBER_MANY_TO_ONE_ROUTE_FAILURE) {
    routeErrorCount += 1;
    if (routeErrorCount >= EMBER_AF_PLUGIN_CONCENTRATOR_ROUTE_ERROR_THRESHOLD) {
      emberAfPluginConcentratorQueueDiscovery();
    }
  }
}

void emberAfPluginConcentratorMessageSentCallback(EmberOutgoingMessageType type,
                                                  int16u indexOrDestination,
                                                  EmberApsFrame *apsFrame,
                                                  EmberStatus status,
                                                  int16u messageLength,
                                                  int8u *messageContents)
{
  if ((type == EMBER_OUTGOING_DIRECT
       || type == EMBER_OUTGOING_VIA_ADDRESS_TABLE
       || type == EMBER_OUTGOING_VIA_BINDING)
      && status == EMBER_DELIVERY_FAILED) {
    deliveryFailureCount += 1;
    if (deliveryFailureCount >= EMBER_AF_PLUGIN_CONCENTRATOR_DELIVERY_FAILURE_THRESHOLD) {
      emberAfPluginConcentratorQueueDiscovery();
    }
  }
}

// We only store one valid overhead for one destination. We don't want to overwrite that with
// an invalid source route to another destination. We do however want to invalidate an 
// overhead to our destination if it is now unknown.
void emberAfSetSourceRouteOverheadCallback(EmberNodeId destination, int8u overhead)
{
  #if defined(EZSP_HOST) && defined(EMBER_AF_PLUGIN_CONCENTRATOR_NCP_SUPPORT)
    if(!(destination != targetId && overhead == EZSP_SOURCE_ROUTE_OVERHEAD_UNKNOWN)) {
      targetId = destination;
      sourceRouteOverhead = overhead;
    }
  #endif
}

// In an effort to reduce the traffic between the host and NCP, for each incoming message, 
// the sourceRouteOverhead to that particular destination is sent from the NCP to the host 
// as a part of the incomingMessageHandler(). This information is cached and can be used 
// once to calculate the MaximumPayload() to that same destination. It is invalidated after
// one use.
int8u emberAfGetSourceRouteOverheadCallback(EmberNodeId destination)
{
#if defined(EZSP_HOST) 
  #if defined(EMBER_AF_PLUGIN_CONCENTRATOR_NCP_SUPPORT)
    if(targetId == destination && sourceRouteOverhead != EZSP_SOURCE_ROUTE_OVERHEAD_UNKNOWN){
      emberAfDebugPrintln("ValidSourceRouteFound %u ",sourceRouteOverhead);
      return sourceRouteOverhead;
    } else{
      return ezspGetSourceRouteOverhead(destination);
    }
  #else
    return ezspGetSourceRouteOverhead(destination);
  #endif
#else
  return emberGetSourceRouteOverhead(destination);
#endif
}

void emberAfPluginConcentratorUpdateEventHandler(void)
{
  routeErrorCount = 0;
  deliveryFailureCount = 0;
  if (EMBER_SUCCESS
      == emberSendManyToOneRouteRequest(EMBER_AF_PLUGIN_CONCENTRATOR_CONCENTRATOR_TYPE, 
                                        EMBER_AF_PLUGIN_CONCENTRATOR_MAX_HOPS)) {
    emberAfDebugPrintln("send MTORR");
    emberAfPluginConcentratorBroadcastSentCallback();
  }
  queueRouteDiscovery(USE_MAX_TIME);
}

void emberIncomingRouteErrorHandler(EmberStatus status, EmberNodeId target)
{
  routeErrorCallback(status);
}

void ezspIncomingRouteErrorHandler(EmberStatus status, EmberNodeId target)
{
  routeErrorCallback(status);
}

void emberAfPluginConcentratorStackStatusCallback(EmberStatus status)
{
  if (status == EMBER_NETWORK_DOWN
      && !emberStackIsPerformingRejoin()) {
    emberAfCorePrintln("Clearing source route table.");
    sourceRouteClearTable();    
  }
}

//------------------------------------------------------------------------------
// These functions are defined for legacy support for the software using the old
// app/util/concentrator/ names.
// I would really like to #define the old function names to the new ones,
// but creating a real function with the same name will cause a duplicate symbol 
// error to occur if both this plugin and the old code are included.  Users can then
// remove the app/util/concentrator code from their project to prevent confusion
// and redundancy.
int32u emberConcentratorQueueRouteDiscovery(void)
{
  return emberAfPluginConcentratorQueueDiscovery();
}

void emberConcentratorStartDiscovery(void)
{
  emberAfPluginConcentratorInitCallback();  
}

void emberConcentratorStopDiscovery(void)
{
  emberAfPluginConcentratorStopDiscovery();  
}

//------------------------------------------------------------------------------
