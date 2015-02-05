// *****************************************************************************
// * address-table-cli.c
// *
// * This code provides support for managing the address table.
// *
// * Copyright 2012 by Ember Corporation. All rights reserved.              *80*
// *****************************************************************************

#include "app/framework/include/af.h"
#include "app/util/serial/command-interpreter2.h"

#include "address-table-management.h"

void emberAfPluginAddressTableAddCommand(void);
void emberAfPluginAddressTableRemoveCommand(void);
void emberAfPluginAddressTableLookupCommand(void);

#if !defined(EMBER_AF_GENERATE_CLI)
EmberCommandEntry emberAfPluginAddressTableCommands[] = {
  emberCommandEntryAction("add", emberAfPluginAddressTableAddCommand,  "b", "Add an entry to the address table."),
  emberCommandEntryAction("remove", emberAfPluginAddressTableRemoveCommand,  "b", "Remove an entry from the address table."),
  emberCommandEntryAction("lookup", emberAfPluginAddressTableLookupCommand,  "b", "Search for an entry in the address table."),
  emberCommandEntryTerminator(),
};
#endif // EMBER_AF_GENERATE_CLI

void emberAfPluginAddressTableAddCommand(void)
{
  int8u index;
  EmberEUI64 entry;
  emberCopyEui64Argument(0, entry);

  index = emberAfPluginAddressTableAddEntry(entry);

  if (index == EMBER_NULL_ADDRESS_TABLE_INDEX) {
    emberAfCorePrintln("Table full, entry not added");
  } else {
    emberAfCorePrintln("Entry added at position 0x%x", index);
  }
}

void emberAfPluginAddressTableRemoveCommand(void)
{
  EmberStatus status;
  EmberEUI64 entry;
  emberCopyEui64Argument(0, entry);

  status = emberAfPluginAddressTableRemoveEntry(entry);

  if (status == EMBER_SUCCESS) {
    emberAfCorePrintln("Entry removed");
  } else {
    emberAfCorePrintln("Entry removal failed");
  }
}

void emberAfPluginAddressTableLookupCommand(void)
{
  int8u index;
  EmberEUI64 entry;
  emberCopyEui64Argument(0, entry);
  index = emberAfPluginAddressTableLookupByEui64(entry);

  if (index == EMBER_NULL_ADDRESS_TABLE_INDEX)
    emberAfCorePrintln("Entry not found");
  else
    emberAfCorePrintln("Found entry at position 0x%x", index);
}

