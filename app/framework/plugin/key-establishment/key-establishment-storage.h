// *******************************************************************
// * key-establishment-storage.h
// *
// * API for the storage of public temporary partner data.
// * - Partner Certificate
// * - Partner Ephemeral Public Key
// * - A single SMAC
// *
// * Copyright 2008 by Ember Corporation. All rights reserved.              *80*
// *******************************************************************

// If isCertificate is FALSE, data is a public key.
boolean storePublicPartnerData(boolean isCertificate,
                               int8u* data);
boolean storePublicPartnerData163k1(boolean isCertificate,
                                    int8u* data);
boolean storePublicPartnerData283k1(boolean isCertificate,
                                    int8u* data);
boolean retrieveAndClearPublicPartnerData163k1(EmberCertificateData* partnerCertificate, 
                                               EmberPublicKeyData* partnerEphemeralPublicKey);

boolean retrieveAndClearPublicPartnerData(EmberCertificate283k1Data* partnerCertificate, 
                                          EmberPublicKey283k1Data* partnerEphemeralPublicKey);
boolean retrieveAndClearPublicPartnerData283k1(EmberCertificate283k1Data* partnerCertificate,
                                               EmberPublicKey283k1Data* partnerEphemeralPublicKey);
boolean storeSmac(EmberSmacData* smac);
boolean getSmacPointer(EmberSmacData** smacPtr);

void clearAllTemporaryPublicData(void);

