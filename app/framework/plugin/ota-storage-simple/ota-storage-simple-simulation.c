// *****************************************************************************
// * ota-storage-simple-simulation.c
// *
// * This code will load a file from disk into the 'Simple Storage' plugin.
// * 
// * Copyright 2012 by Ember Corporation. All rights reserved.              *80*
// *****************************************************************************

#include "app/framework/include/af.h"
#include "app/framework/plugin/ota-common/ota.h"

#if defined(EMBER_TEST)

#include "app/util/serial/command-interpreter2.h"

#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

//------------------------------------------------------------------------------
// Globals

#define MAX_READ_SIZE 512
#define MAX_PATH_SIZE 512

//------------------------------------------------------------------------------
// Forward Declarations

//------------------------------------------------------------------------------

static boolean loadFileIntoOtaStorage(const char* file)
{
  FILE* fh = fopen(file, "rb");
  char cwd[MAX_PATH_SIZE];
    
  assert(NULL != getcwd(cwd, MAX_PATH_SIZE));

  emberAfOtaBootloadClusterFlush();
  otaPrintln("Current directory: '%s'", cwd);
  emberAfOtaBootloadClusterFlush();

  if (fh == NULL) {
    otaPrintln("Failed to open file: %p",
               strerror(errno));
    return FALSE;
  }

  struct stat buffer;
  if (0 != stat(file, &buffer)) {
    otaPrintln("Failed to stat() file: %p",
               strerror(errno));
    return FALSE;
  }

  EmberAfOtaStorageStatus status = emberAfOtaStorageClearTempDataCallback();
  if (status != EMBER_AF_OTA_STORAGE_SUCCESS) {
    otaPrintln("Failed to delete existing OTA file.");
    return FALSE;
  }
  
  off_t offset = 0;
  while (offset < buffer.st_size) {
    int8u data[MAX_READ_SIZE];
    off_t readSize = (buffer.st_size - offset > MAX_READ_SIZE
                      ? MAX_READ_SIZE
                      : buffer.st_size - offset);
    size_t readAmount = fread(data, 1, readSize, fh);
    if (readAmount != readSize) {
      otaPrintln("Failed to read %d bytes from file at offset 0x%4X",
                 readSize,
                 offset);
      status = EMBER_AF_OTA_STORAGE_ERROR;
      goto loadStorageDone;
    }

    status = emberAfOtaStorageWriteTempDataCallback(offset,
                                                    readAmount,
                                                    data);
    if (status != EMBER_AF_OTA_STORAGE_SUCCESS) {
      otaPrintln("Failed to load data into temp storage.");
      goto loadStorageDone;
    }

    offset += readAmount;
  }

  status = emberAfOtaStorageFinishDownloadCallback(offset);

  if (status == EMBER_AF_OTA_STORAGE_SUCCESS) {
    int32u totalSize;
    int32u offset;
    EmberAfOtaImageId id;
    status = emberAfOtaStorageCheckTempDataCallback(&offset,
                                                    &totalSize,
                                                    &id);

    if (status != EMBER_AF_OTA_STORAGE_SUCCESS) {
      otaPrintln("Failed to validate OTA file.");
      goto loadStorageDone;
    }

    otaPrintln("Loaded image successfully.");
  }

 loadStorageDone:
  fclose(fh);
  return status;
}

#endif // EMBER_TEST

// TODO: this should be gated once we set up a gating mechanism for the 
// generated CLI
void emAfOtaLoadFileCommand(void)
{
  char filename[250];
  int8u length = emberCopyStringArgument(0,
                                         filename,
                                         250,
                                         FALSE);
  filename[length] = '\0';
  otaPrintln("Loading from file: '%s'", filename);
  loadFileIntoOtaStorage(filename);  
}


