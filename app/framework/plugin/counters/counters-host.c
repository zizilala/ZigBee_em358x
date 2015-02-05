// File: counters-host.c
//
// Description: Provides the host utility functions for counters.
//
// Copyright 2013 by Ember Corporation. All rights reserved.                *80*

#include "app/framework/include/af.h"
#include "app/util/serial/command-interpreter2.h"
#include "app/util/common/common.h"
#include "counters.h"
#include "counters-ota.h"

int16u emberCounters[EMBER_COUNTER_TYPE_COUNT];
int16u emberCountersThresholds[EMBER_COUNTER_TYPE_COUNT];

void emberAfPluginCountersInitCallback(void)
{
  emberAfPluginCountersClear();
  emberAfPluginCountersResetThresholds();
}

void ezspCounterRolloverHandler(EmberCounterType type)
{
  emberAfPluginCountersRolloverCallback(type);
}

void emberAfPluginCountersClear(void)
{
  EzspStatus status;
  status = ezspSetValue(EZSP_VALUE_CLEAR_COUNTERS,0,NULL);
  if(status!=EZSP_SUCCESS)
    emberAfCorePrintln("ezsp counters failed %u",status);    
}

void emberAfPluginCountersResetThresholds(void)
{
  //Clear local copy of thresholds.

  EzspStatus status;
  status = ezspSetValue(EZSP_VALUE_RESET_COUNTER_THRESHOLDS,0,NULL);
  if(status!=EZSP_SUCCESS)
    emberAfCorePrintln("ezsp Reset Threshold error %u",status);
  else
    MEMSET(emberCountersThresholds,0xFF,sizeof(emberCountersThresholds));  
}

void emberAfPluginCountersSetThreshold(EmberCounterType type, int16u threshold)
{
  EzspStatus status;
  int8u ezspThreshold[3];
  ezspThreshold[0] = type;
  ezspThreshold[1] = LOW_BYTE(threshold);
  ezspThreshold[2] = HIGH_BYTE(threshold);
  status = ezspSetValue(EZSP_VALUE_SET_COUNTER_THRESHOLD,sizeof(ezspThreshold),ezspThreshold);
  if(status!=EZSP_SUCCESS)
    emberAfCorePrintln("ezsp Set Threshold error %u",status);
  else
    emberCountersThresholds[type] = threshold;
}

