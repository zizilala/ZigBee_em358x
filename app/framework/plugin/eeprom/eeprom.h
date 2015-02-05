// *****************************************************************************
// * eeprom.h
// *
// * Header file for eeprom plugin API.
// *
// * Copyright 2012 by Silicon Labs.      All rights reserved.              *80*
// *****************************************************************************


void emberAfPluginEepromInit(void);

void emberAfPluginEepromNoteInitializedState(boolean state);

int8u emberAfPluginEepromGetWordSize(void);
const HalEepromInformationType* emberAfPluginEepromInfo(void);

int8u emberAfPluginEepromWrite(int32u address, const int8u *data, int16u totalLength);

int8u emberAfPluginEepromRead(int32u address, int8u *data, int16u totalLength);

// Erase has a 32-bit argument, since it's possible to erase more than int16u chunk.
// Read and write have only int16u for length, because you don't have enough RAM
// for the data buffer
int8u emberAfPluginEepromErase(int32u address, int32u totalLength);

boolean emberAfPluginEepromBusy(void);

boolean emberAfPluginEepromShutdown(void);

int8u emberAfPluginEepromFlushSavedPartialWrites(void);

#if defined(EMBER_TEST)
void emAfPluginEepromFakeEepromCallback(void);
#endif


// Currently there are no EEPROM/flash parts that we support that have a word size
// of 4.  The local storage bootloader has a 2-byte word size and that is the main
// thing we are optimizing for.
#define EM_AF_EEPROM_MAX_WORD_SIZE 2

typedef struct {
  int32u address;
  int8u data;
} EmAfPartialWriteStruct;

extern EmAfPartialWriteStruct emAfEepromSavedPartialWrites[];
boolean emAfIsEepromInitialized(void);