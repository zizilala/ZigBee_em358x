// *******************************************************************
// * key-establishment-curve-support.c
// * 
// * This file calls ECC curve specific functions from the cbke library
// * based on whether the 163k1 ECC curve or the 283k1 ECC curve is
// * being used for key establishment.
// *
// * Copyright 2013 by Silicon Labs. All rights reserved.                      *80*
// *******************************************************************

// this file contains all the common includes for clusters in the zcl-util
#include "../../util/common.h"

#ifndef EZSP_HOST
  #include "stack/include/cbke-crypto-engine.h"
#endif
#include "key-establishment.h"
#include "key-establishment-storage.h"

// The offset within the 163k1 curve certificate struct where the issuer field
// lives.  22-bytes for Public Key Reconstruction data, and 8-bytes for subject.
#define CERT_SUBJECT_OFFSET 22
#define CERT_ISSUER_OFFSET  (CERT_SUBJECT_OFFSET + 8)
#define CERT_ISSUER_SIZE    8

// The offset within the 283k1 curve certificate struct where the issuer field
// lives.  22-bytes for Public Key Reconstruction data, and 8-bytes for subject.
#define CERT_SUBJECT_OFFSET_283K1 28
#define CERT_ISSUER_OFFSET_283K1  11



boolean checkIssuer(int8u *issuer)
{
	int8u *ourIssuer;

	if(isCbkeKeyEstablishmentSuite163k1()){
		EmberCertificateData localCert;
	  if (emberGetCertificate(&localCert) != EMBER_SUCCESS) {
	    emberAfKeyEstablishmentClusterPrintln("Could not retrieve certificate");
	    cleanupAndStop(NO_LOCAL_RESOURCES);
	    return FALSE;
  	}
  	ourIssuer = &(localCert.contents[CERT_ISSUER_OFFSET]);
	} else if (isCbkeKeyEstablishmentSuite283k1()) {
		EmberCertificate283k1Data localCert;
		if(emberGetCertificate283k1(&localCert) != EMBER_SUCCESS) {
	    emberAfKeyEstablishmentClusterPrintln("Could not retrieve 283k1 certificate");
	    cleanupAndStop(NO_LOCAL_RESOURCES);
	    return FALSE;
  	}
  	 ourIssuer = &(localCert.contents[CERT_ISSUER_OFFSET_283K1]);
	} else {
		//No valid Suite.
		return FALSE;
	}

  if (MEMCOMPARE(issuer, ourIssuer, CERT_ISSUER_SIZE) == 0) {
  emberAfKeyEstablishmentClusterPrintln("Good Certificate Issuer");
    return TRUE;
  }
  emberAfKeyEstablishmentClusterPrintln("Bad Certificate Issuer");
  cleanupAndStop(BAD_CERTIFICATE_ISSUER);
  return FALSE;
}

boolean storePublicPartnerData(boolean isCertificate,
                              int8u* data)
{
	if(isCbkeKeyEstablishmentSuite163k1()){
		return storePublicPartnerData163k1(isCertificate,data);
	} else if(isCbkeKeyEstablishmentSuite283k1()){
		return storePublicPartnerData283k1(isCertificate,data);
	} else{
		//No valid suite
		return FALSE;
	}
}

boolean retrieveAndClearPublicPartnerData(EmberCertificate283k1Data* partnerCertificate,
                                          EmberPublicKey283k1Data* partnerEphemeralPublicKey)
{
	if(isCbkeKeyEstablishmentSuite163k1()) {
		return retrieveAndClearPublicPartnerData163k1((EmberCertificateData *)emberCertificate283k1Contents(partnerCertificate),
													  (EmberPublicKeyData *)emberPublicKey283k1Contents(partnerEphemeralPublicKey));
	} else if (isCbkeKeyEstablishmentSuite283k1()) {
		return retrieveAndClearPublicPartnerData283k1(partnerCertificate,partnerEphemeralPublicKey);
	}	else{
		//No valid suite
		return FALSE;
	}
}

EmberAfKeyEstablishmentNotifyMessage sendCertificate(void)
{
	if(isCbkeKeyEstablishmentSuite163k1()) {
	  EmberCertificateData localCert;
	  if (emberGetCertificate(&localCert) != EMBER_SUCCESS) {
	    emberAfKeyEstablishmentClusterPrintln("Failed to retrieve certificate.");
	    return NO_LOCAL_RESOURCES;
	  }
	  sendNextKeyEstablishMessage(ZCL_INITIATE_KEY_ESTABLISHMENT_REQUEST_COMMAND_ID,
                              emberCertificateContents(&localCert));  
	} else if (isCbkeKeyEstablishmentSuite283k1()) {
		EmberCertificate283k1Data localCert;
	  if (emberGetCertificate283k1(&localCert) != EMBER_SUCCESS) {
	    emberAfKeyEstablishmentClusterPrintln("Failed to retrieve 283k1 certificate.");
	    return NO_LOCAL_RESOURCES;
	  }
	  sendNextKeyEstablishMessage(ZCL_INITIATE_KEY_ESTABLISHMENT_REQUEST_COMMAND_ID,
                              emberCertificate283k1Contents(&localCert));
	} else {
		//Closest error message
		return NO_LOCAL_RESOURCES;
	}

  return NO_APP_MESSAGE;
}

EmberStatus emGenerateCbkeKeysForCurve(void)
{
	EmberStatus status;
	if(isCbkeKeyEstablishmentSuite163k1()) {
		status = emberGenerateCbkeKeys();
	} else if (isCbkeKeyEstablishmentSuite283k1()) {
		status = emberGenerateCbkeKeys283k1();
	} else {
		status = EMBER_ERR_FATAL;
	}
	return status;
}

EmberStatus emCalculateSmacsForCurve(boolean amInitiator,
                                     EmberCertificate283k1Data* partnerCert,
                                     EmberPublicKey283k1Data* partnerEphemeralPublicKey)
{
	EmberStatus status;
	emberAfKeyEstablishmentClusterPrintln("calculateSmacs()");
	if(isCbkeKeyEstablishmentSuite163k1()) {
		status = emberCalculateSmacs(amInitiator,
									 (EmberCertificateData *)emberCertificate283k1Contents(partnerCert),
									 (EmberPublicKeyData *)emberPublicKey283k1Contents(partnerEphemeralPublicKey));
	} else if(isCbkeKeyEstablishmentSuite283k1()) {
		status = emberCalculateSmacs283k1(amInitiator,
										  partnerCert,
										  partnerEphemeralPublicKey);
	} else {
		status = EMBER_ERR_FATAL;
	}
	return status;
}

EmberStatus emberClearTemporaryDataMaybeStoreLinkKeyForCurve(boolean storeLinkKey)
{
	EmberStatus status;
	if( isCbkeKeyEstablishmentSuite163k1() ){
		status = emberClearTemporaryDataMaybeStoreLinkKey(storeLinkKey);
	} else if( isCbkeKeyEstablishmentSuite283k1()) {
		status = emberClearTemporaryDataMaybeStoreLinkKey283k1(storeLinkKey);
	} else {
		status = EMBER_ERR_FATAL;
	}
	return status;
}

