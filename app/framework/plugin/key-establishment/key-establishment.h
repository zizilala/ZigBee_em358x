// *******************************************************************
// * key-establishment.h
// *
// *
// * Copyright 2008 by Ember Corporation. All rights reserved.              *80*
// *******************************************************************


// Init - bytes: suite (2), key gen time (1), derive secret time (1), cert (48)
#define EM_AF_KE_INITIATE_SIZE        (2 + 1 + 1 + EMBER_CERTIFICATE_SIZE)
#define EM_AF_KE_INITIATE_SIZE_283K1  (2 + 1 + 1 + EMBER_CERTIFICATE_283K1_SIZE)
#define EM_AF_KE_EPHEMERAL_SIZE       EMBER_PUBLIC_KEY_SIZE
#define EM_AF_KE_EPHEMERAL_SIZE_283K1 EMBER_PUBLIC_KEY_283K1_SIZE
#define EM_AF_KE_SMAC_SIZE            EMBER_SMAC_SIZE      

  // Terminate - bytes: status (1), wait time (1), suite (2)
#define EM_AF_KE_TERMINATE_SIZE (1 + 1 + 2)

#define APS_ACK_TIMEOUT_SECONDS 1

// These values reported to the remote device as to how long the local
// device takes to execute these operations.
#define DEFAULT_EPHEMERAL_DATA_GENERATE_TIME_SECONDS   (10 + APS_ACK_TIMEOUT_SECONDS)
#define DEFAULT_GENERATE_SHARED_SECRET_TIME_SECONDS    (15 + APS_ACK_TIMEOUT_SECONDS)

extern PGM int8u emAfKeyEstablishMessageToDataSize[];

#ifdef EZSP_HOST
  #define emAfPluginKeyEstablishmentGenerateCbkeKeysHandler          ezspGenerateCbkeKeysHandler
  #define emAfPluginKeyEstablishmentCalculateSmacsHandler            ezspCalculateSmacsHandler
  #define emAfPluginKeyEstablishmentGenerateCbkeKeysHandler283k1    ezspGenerateCbkeKeysHandler283k1
  #define emAfPluginKeyEstablishmentCalculateSmacsHandler283k1       ezspCalculateSmacsHandler283k1
#else
  #define emAfPluginKeyEstablishmentGenerateCbkeKeysHandler          emberGenerateCbkeKeysHandler
  #define emAfPluginKeyEstablishmentCalculateSmacsHandler            emberCalculateSmacsHandler
  #define emAfPluginKeyEstablishmentGenerateCbkeKeysHandler283k1    emberGenerateCbkeKeysHandler283k1
  #define emAfPluginKeyEstablishmentCalculateSmacsHandler283k1       emberCalculateSmacsHandler283k1
#endif

#define TERMINATE_STATUS_STRINGS { \
    "Success",                     \
    "Unknown Issuer",              \
    "Bad Key Confirm",             \
    "Bad Message",                 \
    "No resources",                \
    "Unsupported suite",           \
    "Bad Key Usage byte",          \
    "???",                         \
   }
#define UNKNOWN_TERMINATE_STATUS 7

typedef enum {
  NO_KEY_ESTABLISHMENT_EVENT     = 0,
  //Initiator only event
  CHECK_SUPPORTED_CURVES         = 1,
  BEGIN_KEY_ESTABLISHMENT        = 2,
  GENERATE_KEYS                  = 3,
  SEND_EPHEMERAL_DATA_MESSAGE    = 4,
  GENERATE_SHARED_SECRET         = 5,
  SEND_CONFIRM_KEY_MESSAGE       = 6,

  // Initiator only event
  INITIATOR_RECEIVED_CONFIRM_KEY = 7,

} KeyEstablishEvent;  

typedef int8u KeyEstablishMessage;

extern EmberAfCbkeKeyEstablishmentSuite emAfAvailableCbkeSuite;
extern EmberAfCbkeKeyEstablishmentSuite emAfCurrentCbkeSuite;

# define isCbkeKeyEstablishmentSuiteValid() \
(emAfCurrentCbkeSuite <= EMBER_AF_CBKE_KEY_ESTABLISHMENT_SUITE_283K1)

# define isCbkeKeyEstablishmentSuite163k1() \
(emAfCurrentCbkeSuite \
  == EMBER_AF_CBKE_KEY_ESTABLISHMENT_SUITE_163K1)

# define isCbkeKeyEstablishmentSuite283k1() \
(emAfCurrentCbkeSuite \
  == EMBER_AF_CBKE_KEY_ESTABLISHMENT_SUITE_283K1)

#define cleanupAndStop(message) cleanupAndStopWithDelay((message), 0)

boolean checkIssuer(int8u *issuer);
void cleanupAndStopWithDelay(EmberAfKeyEstablishmentNotifyMessage message,
                             int8u delayInSec);
EmberAfKeyEstablishmentNotifyMessage sendCertificate(void);
void sendNextKeyEstablishMessage(KeyEstablishMessage message,
                                        int8u *data);
EmberStatus emGenerateCbkeKeysForCurve(void);
void emAfKeyEstablishmentSelectCurve(EmberAfCbkeKeyEstablishmentSuite suite);
EmberStatus emCalculateSmacsForCurve(boolean amInitiator,
                                     EmberCertificate283k1Data* partnerCert,
                                     EmberPublicKey283k1Data* partnerEphemeralPublicKey);
EmberStatus emberClearTemporaryDataMaybeStoreLinkKeyForCurve(boolean storeLinkKey);
void emberAfPluginKeyEstablishmentReadAttributesCallback(EmberAfCbkeKeyEstablishmentSuite suite);
void emAfSkipCheckSupportedCurves(EmberAfCbkeKeyEstablishmentSuite suite);
void emAfSetAvailableCurves(EmberAfCbkeKeyEstablishmentSuite suite);
