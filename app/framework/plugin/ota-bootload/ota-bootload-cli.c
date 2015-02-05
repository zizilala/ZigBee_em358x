// *******************************************************************
// * ota-bootload-cli.c
// *
// * Bootload specific commands
// * 
// * Copyright 2009 by Ember Corporation. All rights reserved.              *80*
// *******************************************************************

#include "app/framework/include/af.h"
#include "app/util/serial/command-interpreter2.h"
#include "app/framework/plugin/ota-common/ota.h"

#include "hal/micro/bootloader-interface.h"

//------------------------------------------------------------------------------
// Forward Declarations

void printBootloaderInfoCommand(void);

#ifndef EMBER_AF_GENERATE_CLI
EmberCommandEntry emberAfPluginOtaBootloadCommands[] = {
  emberCommandEntryAction("info", printBootloaderInfoCommand, "", 
                          "Print information about the installed bootloader."),
  emberCommandEntryTerminator(),
};
#endif // EMBER_AF_GENERATE_CLI

//------------------------------------------------------------------------------


void printBootloaderInfoCommand(void)
{
#if !defined(EZSP_HOST)
  BlExtendedType blExtendedType = halBootloaderGetInstalledType();
  int32u emberVersion;
  int32u customVersion;
  int8u keyData[EMBER_ENCRYPTION_KEY_SIZE];
  halGetExtendedBootloaderVersion(&emberVersion, &customVersion);
  otaPrintln("Installed Type (Base):      0x%X",  halBootloaderGetType());
  otaPrintln("Installed Type (Extended):  0x%2X", blExtendedType);
  otaPrintln("Bootloader Version:         0x%2X", halGetBootloaderVersion());
  otaPrintln("Bootloader Version, Ember:  0x%4X", emberVersion);
  otaPrintln("Bootloader Version, Custom: 0x%4X", customVersion);

#if defined(EMBER_TEST)
  MEMSET(keyData, 0xFF, EMBER_ENCRYPTION_KEY_SIZE);
#else
  halCommonGetToken(keyData, TOKEN_MFG_SECURE_BOOTLOADER_KEY);
#endif

  otaPrint  ("Secure Bootloader Key:      ");
  emberAfPrintZigbeeKey(keyData);
  otaPrintln("");

#else
  otaPrintln("Unsupported on EZSP Host");
#endif

}
