// *******************************************************************
// * reporting-cli.c
// *
// *
// * Copyright 2011 by Ember Corporation. All rights reserved.              *80*
// *******************************************************************

#include "app/framework/include/af.h"
#include "app/util/serial/command-interpreter2.h"
#include "reporting.h"

void emAfPluginReportingCliPrint(void);
void emAfPluginReportingCliClear(void);
void emAfPluginReportingCliRemove(void); // "remov" to avoid a conflict with "remove" in stdio
void emAfPluginReportingCliAdd(void);

#if !defined(EMBER_AF_GENERATE_CLI)
EmberCommandEntry emberAfPluginReportingCommands[] = {
  emberCommandEntryAction("print",  emAfPluginReportingCliPrint, "",        "Print the reporting table"),
  emberCommandEntryAction("clear",  emAfPluginReportingCliClear, "",        "Clear the reporting tabel"),
  emberCommandEntryAction("remove", emAfPluginReportingCliRemove, "u",       "Remove an entry from the reporting table"),
  emberCommandEntryAction("add",    emAfPluginReportingCliAdd,   "uvvuvvw", "Add an entry to the reporting table"),
  emberCommandEntryTerminator(),
};
#endif // EMBER_AF_GENERATE_CLI

// plugin reporting print
void emAfPluginReportingCliPrint(void)
{
  int8u i;
  for (i = 0; i < EMBER_AF_PLUGIN_REPORTING_TABLE_SIZE ; i++) {
    EmberAfPluginReportingEntry entry;
    emAfPluginReportingGetEntry(i, &entry);
    emberAfReportingPrint("%x:", i);
    if (entry.endpoint != EMBER_AF_PLUGIN_REPORTING_UNUSED_ENDPOINT_ID) {
      emberAfReportingPrint("ep %x clus %2x attr %2x svr %c",
                            entry.endpoint,
                            entry.clusterId,
                            entry.attributeId,
                            (entry.mask == CLUSTER_MASK_SERVER ? 'y' : 'n'));
      if (entry.manufacturerCode != EMBER_AF_NULL_MANUFACTURER_CODE) {
        emberAfReportingPrint(" mfg %x", entry.manufacturerCode);
      }
      if (entry.direction == EMBER_ZCL_REPORTING_DIRECTION_REPORTED) {
        emberAfReportingPrint(" report min %2x max %2x rpt-chg %4x",
                              entry.data.reported.minInterval,
                              entry.data.reported.maxInterval,
                              entry.data.reported.reportableChange);
        emberAfReportingFlush();
      } else {
        emberAfReportingPrint(" receive from %2x ep %x timeout %2x",
                              entry.data.received.source,
                              entry.data.received.endpoint,
                              entry.data.received.timeout);
      }
    }
    emberAfReportingPrintln("");
    emberAfReportingFlush();
  }
}

// plugin reporting clear
void emAfPluginReportingCliClear(void)
{
  EmberStatus status = emberAfClearReportTableCallback();
  emberAfReportingPrintln("%p 0x%x", "clear", status);
}

// plugin reporting remove <index:1>
void emAfPluginReportingCliRemove(void)
{
  EmberStatus status = emAfPluginReportingRemoveEntry((int8u)emberUnsignedCommandArgument(0));
  emberAfReportingPrintln("%p 0x%x", "remove", status);
}

// plugin reporting add <endpoint:1> <cluster id:2> <attribute id:2> ...
// ... <mask:1> <min interval:2> <max interval:2> <reportable change:4>
void emAfPluginReportingCliAdd(void)
{
  EmberAfStatus status = EMBER_ZCL_STATUS_SUCCESS;
  EmberAfPluginReportingEntry entry;
  entry.endpoint = (int8u)emberUnsignedCommandArgument(0);
  entry.clusterId = (EmberAfClusterId)emberUnsignedCommandArgument(1);
  entry.attributeId = (EmberAfAttributeId)emberUnsignedCommandArgument(2);
  entry.mask = (int8u)(emberUnsignedCommandArgument(3) == 0
                       ? CLUSTER_MASK_CLIENT
                       : CLUSTER_MASK_SERVER);
  entry.manufacturerCode = EMBER_AF_NULL_MANUFACTURER_CODE;
  entry.data.reported.minInterval = (int16u)emberUnsignedCommandArgument(4);
  entry.data.reported.maxInterval = (int16u)emberUnsignedCommandArgument(5);
  entry.data.reported.reportableChange = emberUnsignedCommandArgument(6);
  status = emberAfPluginReportingConfigureReportedAttribute(&entry);

  emberAfReportingPrintln("%p 0x%x", "add", status);
}
