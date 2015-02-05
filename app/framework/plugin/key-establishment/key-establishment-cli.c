// *******************************************************************
// * key-establishment-cli.c
// *
// *
// * Copyright 2013 by Ember Corporation. All rights reserved.              *80*
// *******************************************************************

#include "app/framework/include/af.h"
#include "app/util/serial/command-interpreter2.h"

#include "app/framework/util/af-main.h"  // for emberAfCopyBigEndianEui64Argument()

//-----------------------------------------------------------------------------
// Forward Declarations

void emAfKeyEstablishmentStartCommand(void);
void emAfKeyEstablishmentInterpanCommand(void);

//-----------------------------------------------------------------------------
// Globals Declarations

#if !defined(EMBER_AF_GENERATE_CLI)

static PGM_P cbkeStartCommandArguments[] = {
  "Target node ID",
  "Target node endpoint",
  NULL,
};

static PGM_P cbkeInterPanCommandArguments[] = {
  "Target PAN ID",
  "Target EUI64 (big endian)",
  NULL,
};

EmberCommandEntry emberAfPluginKeyEstablishmentCommands[] = {
  emberCommandEntryActionWithDetails("start", 
                                     emAfKeyEstablishmentStartCommand, 
                                     "vu",
                                     "Initiate key establishment with the target.",
                                     cbkeStartCommandArguments),
  emberCommandEntryActionWithDetails("interpan",
                                     emAfKeyEstablishmentInterpanCommand,
                                     "vb",
                                     "Initiate interpan key establishment with the target",
                                     cbkeInterPanCommandArguments),
  emberCommandEntryTerminator(),
};

#endif // EMBER_AF_GENERATE_CLI

//-----------------------------------------------------------------------------
// Functions

void emAfKeyEstablishmentStartCommand(void)
{
  EmberStatus status;
  EmberNodeId newPartnerId = (EmberNodeId)emberUnsignedCommandArgument(0);
  int8u endpoint = (int8u)emberUnsignedCommandArgument(1);
  emberAfCorePrintln("Starting %pment w/ 0x%2x, EP: 0x%x",\
                     "Key Establish",
                     newPartnerId,
                     endpoint);
  emberAfCoreFlush();
  emberSerialBufferTick();

  status = emberAfInitiateKeyEstablishment(newPartnerId, endpoint);
  emberAfCorePrintln("%p", (status == EMBER_SUCCESS ? "Success" : "Error"));
}

void emAfKeyEstablishmentInterpanCommand(void)
{
  EmberEUI64 eui64;
  EmberPanId panId = (EmberPanId)emberUnsignedCommandArgument(0);
  EmberStatus status;
  emberAfCopyBigEndianEui64Argument(1, eui64);

  emberAfCorePrint("Starting %pment w/ ", "Key Establish");
  emberAfCoreDebugExec(emberAfPrintBigEndianEui64(eui64));
  emberAfCorePrintln("");
  emberAfCoreFlush();
  emberSerialBufferTick();

  status = emberAfInitiateInterPanKeyEstablishment(panId, eui64);
  emberAfCorePrintln("%p", (status == EMBER_SUCCESS ? "Success" : "Error"));
}
