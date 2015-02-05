/** 
 * @file: counters-cli.h
 *
 * Used for testing the counters library via a command line interface.
 * For documentation on the counters library see counters.h.
 *
 * Copyright 2007 by Ember Corporation. All rights reserved.                *80*
 */

void emberAfPluginCountersPrintCommand(void);
void emberAfPluginCountersSimplePrintCommand(void);
void emberAfPluginCountersPrintThresholdsCommand(void);
void emberAfPluginCountersSetThresholdCommand(void);
void emberAfPluginCountersSendRequestCommand(void);
void emberAfPluginCounterPrintCountersResponse(EmberMessageBuffer message);

/** Args: destination id, clearCounters (boolean) */
void sendCountersRequestCommand(void);

/** Utility function for printing out the OTA counters response. */
void printCountersResponse(EmberMessageBuffer message);

