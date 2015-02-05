// *******************************************************************
// * key-establishment-storage-static.c
// *
// * This file implements the routines for storing temporary data that
// * is needed for key establishment.  This is data is completely
// * public and is sent over-the-air and thus not required to be 
// * closely protected.
// *
// * This version uses static memory buffers that are not dynamically
// * allocated.
// *
// * Copyright 2008 by Ember Corporation. All rights reserved.              *80*
// *******************************************************************


// this file contains all the common includes for clusters in the zcl-util
#include "../../util/common.h"

#include "key-establishment-storage.h"

#ifndef EZSP_HOST
  #include "stack/include/cbke-crypto-engine.h"
#endif

//------------------------------------------------------------------------------
// Globals

static EmberCertificateData partnerCert;
static EmberPublicKeyData partnerPublicKey;
static EmberCertificate283k1Data partnerCert283k1;
static EmberPublicKey283k1Data partnerPublicKey283k1;
static EmberSmacData storedSmac;

//------------------------------------------------------------------------------

boolean storePublicPartnerData163k1(boolean isCertificate,
                                    int8u* data)
{
  int8u* ptr = (isCertificate
                ? emberCertificateContents(&partnerCert)
                : emberPublicKeyContents(&partnerPublicKey));
  int8u size = (isCertificate 
                ? EMBER_CERTIFICATE_SIZE
                : EMBER_PUBLIC_KEY_SIZE);
  MEMCOPY(ptr, data, size);
  return TRUE;
}

boolean retrieveAndClearPublicPartnerData163k1(EmberCertificateData* partnerCertificate,
                                               EmberPublicKeyData* partnerEphemeralPublicKey)
{
  if ( partnerCertificate != NULL ) {
    MEMCOPY(partnerCertificate,
            &partnerCert,
            EMBER_CERTIFICATE_SIZE);
  }
  if ( partnerEphemeralPublicKey != NULL ) {
    MEMCOPY(partnerEphemeralPublicKey,
            &partnerPublicKey,
            EMBER_PUBLIC_KEY_SIZE);
  }
  MEMSET(&partnerCert, 0, EMBER_CERTIFICATE_SIZE);
  MEMSET(&partnerPublicKey, 0, EMBER_PUBLIC_KEY_SIZE);
  return TRUE;
}

boolean storePublicPartnerData283k1(boolean isCertificate,
                                    int8u* data)
{
  int8u* ptr = (isCertificate
                ? emberCertificate283k1Contents(&partnerCert283k1)
                : emberPublicKey283k1Contents(&partnerPublicKey283k1));
  int8u size = (isCertificate 
                ? EMBER_CERTIFICATE_283K1_SIZE
                : EMBER_PUBLIC_KEY_283K1_SIZE);
  MEMCOPY(ptr, data, size);
  return TRUE;
}

boolean retrieveAndClearPublicPartnerData283k1(EmberCertificate283k1Data* partnerCertificate,
                                               EmberPublicKey283k1Data* partnerEphemeralPublicKey)
{
  if ( partnerCertificate != NULL ) {
    MEMCOPY(partnerCertificate,
            &partnerCert283k1,
            EMBER_CERTIFICATE_283K1_SIZE);
  }
  if ( partnerEphemeralPublicKey != NULL ) {
    MEMCOPY(partnerEphemeralPublicKey,
            &partnerPublicKey283k1,
            EMBER_PUBLIC_KEY_283K1_SIZE);
  }
  MEMSET(&partnerCert283k1, 0, EMBER_CERTIFICATE_283K1_SIZE);
  MEMSET(&partnerPublicKey283k1, 0, EMBER_PUBLIC_KEY_283K1_SIZE);
  return TRUE;
}

boolean storeSmac(EmberSmacData* smac)
{
  MEMCOPY(&storedSmac, smac, EMBER_SMAC_SIZE);
  return TRUE;
}

boolean getSmacPointer(EmberSmacData** smacPtr)
{
  *smacPtr = &storedSmac;
  return TRUE;
}

void clearAllTemporaryPublicData(void)
{
  MEMSET(&storedSmac, 0, EMBER_SMAC_SIZE);
  retrieveAndClearPublicPartnerData(NULL, NULL);
  retrieveAndClearPublicPartnerData283k1(NULL, NULL);
}
