// *****************************************************************************
// * standalone-bootloader-client-soc.c
// *
// * This file defines the SOC specific client behavior for the Ember
// * proprietary bootloader protocol.
// * 
// * Copyright 2012 by Ember Corporation. All rights reserved.              *80*
// *****************************************************************************

#include "app/framework/include/af.h"
#include "app/framework/plugin/standalone-bootloader-common/bootloader-protocol.h"

#define MIN_BOOTLOADER_VERSION_WITH_EXTENDED_TYPE_API 0x4700

void emAfStandaloneBootloaderClientGetInfo(int16u* bootloaderVersion,
                                           int8u* platformId,
                                           int8u* microId,
                                           int8u* phyId)
{
  *bootloaderVersion = halGetStandaloneBootloaderVersion();

  *platformId = PLAT;
  *microId = MICRO;
  *phyId = PHY;
}

EmberStatus emAfStandaloneBootloaderClientLaunch(void)
{
  return halLaunchStandaloneBootloader(STANDALONE_BOOTLOADER_NORMAL_MODE);
}

void emAfStandaloneBootloaderClientGetMfgInfo(int16u* mfgIdReturnValue,
                                              int8u* boardNameReturnValue)
{
  halCommonGetToken(mfgIdReturnValue, TOKEN_MFG_MANUF_ID);
  halCommonGetToken(boardNameReturnValue, TOKEN_MFG_BOARD_NAME);
}

int32u emAfStandaloneBootloaderClientGetRandomNumber(void)
{
  return halStackGetInt32uSymbolTick();
}

#if !defined(EMBER_TEST)

#if !defined(SERIAL_UART_BTL)
  #error Wrong Bootloader specified for configuration.  Must specify a standalone bootloader.
#endif

boolean emAfPluginStandaloneBootloaderClientCheckBootloader(void)
{
  BlExtendedType blExtendedType = halBootloaderGetInstalledType();

  if (blExtendedType != BL_EXT_TYPE_SERIAL_UART_OTA) {
    // Actual bootloader on-chip is wrong.
    bootloadPrintln("Error:  Loaded bootloader type 0x%X != required type 0x%X (serial-uart-ota)",
                    blExtendedType,
                    BL_EXT_TYPE_SERIAL_UART_OTA);
    return FALSE;
  }

  return TRUE;
}

void emAfStandaloneBootloaderClientGetKey(int8u* returnData)
{
  halCommonGetToken(returnData, TOKEN_MFG_BOOTLOAD_AES_KEY);
}

#else

boolean emAfPluginStandaloneBootloaderClientCheckBootloader(void)
{
  // In order to test the high level app messages in simulation, we lie
  // and say the bootloader is the "correct" one.  We can't actually bootload
  // in simulation.
  return TRUE;
}

#endif
