// *****************************************************************************
// * test-harness.c
// *
// *  Test harness code for validating the behavior of the key establishment
// *  cluster and modifying the normal behavior of App. Framework.  
// *
// * Copyright 2011 by Ember Corporation. All rights reserved.              *80*
// *****************************************************************************

// this file contains all the common includes for clusters in the zcl-util
#include "app/framework/include/af.h"
#include "app/framework/util/common.h"
#include "app/framework/util/util.h"
#include "app/util/serial/command-interpreter2.h"

#include "app/framework/plugin/key-establishment/key-establishment.h"
#include "test-harness.h"
#include "test-harness-cli.h"
#include "app/framework/plugin/price-server/price-server.h"
#include "app/framework/plugin/trust-center-keepalive/trust-center-keepalive.h"

#include "app/framework/plugin/ota-storage-simple-ram/ota-storage-ram.h"

#include "app/util/concentrator/concentrator.h"

#if !defined(EZSP_HOST)
  #include "stack/include/cbke-crypto-engine.h"
#endif

#include "app/framework/plugin/trust-center-nwk-key-update-periodic/trust-center-nwk-key-update-periodic.h"
#include "app/framework/plugin/trust-center-nwk-key-update-broadcast/trust-center-nwk-key-update-broadcast.h"
#include "app/framework/plugin/trust-center-nwk-key-update-unicast/trust-center-nwk-key-update-unicast.h"

#if !defined(EZSP_HOST)
  #if defined(EMBER_TEST) || defined(EMBER_STACK_TEST_HARNESS)
    #define STACK_TEST_HARNESS
  #endif
#endif

//------------------------------------------------------------------------------
// Globals

typedef int8u TestHarnessMode;

#define MODE_NORMAL           0
#define MODE_CERT_MANGLE      1
#define MODE_OUT_OF_SEQUENCE  2
#define MODE_NO_RESOURCES     3
#define MODE_TIMEOUT          4
#define MODE_DELAY_CBKE       5
#define MODE_DEFAULT_RESPONSE 6
#define MODE_KEY_MANGLE       7

static PGM_P modeText[] = {
  "Normal",
  "Cert Mangle",
  "Out of Sequence",
  "No Resources",
  "Cause Timeout",
  "Delay CBKE operation",
  "Default Response",
  "Key Mangle"
};

static TestHarnessMode testHarnessMode = MODE_NORMAL;
static int8u respondToCommandWithOutOfSequence = ZCL_INITIATE_KEY_ESTABLISHMENT_REQUEST_COMMAND_ID;
static int8s certLengthMod = 0;
static int8s keyLengthMod  = 0;
#define CERT_MANGLE_NONE    0
#define CERT_MANGLE_LENGTH  1
#define CERT_MANGLE_ISSUER  2
#define CERT_MANGLE_CORRUPT 3
#define CERT_MANGLE_SUBJECT 4
#define CERT_MANGLE_VALUE   5

static PGM_P certMangleText[] = {
  "None",
  "Mangle Length",
  "Rewrite Issuer",
  "Corrupt Cert",
  "Change byte",
};

#if defined(EMBER_AF_HAS_SECURITY_PROFILE_SE_TEST)
  #define DEFAULT_POLICY TRUE
#else
  #define DEFAULT_POLICY FALSE
#endif
boolean emKeyEstablishmentPolicyAllowNewKeyEntries = DEFAULT_POLICY;
boolean emAfTestHarnessSupportForNewPriceFields = TRUE;

typedef int8u CertMangleType;

static CertMangleType certMangleType = CERT_MANGLE_NONE;

// Offset from start of ZCL header
//   - ZCL overhead (3 bytes)
//   - KE suite (2 bytes)
//   - Ephemeral Data Generate Time (1 byte)
//   - Confirm Key Generate Time (1 byte)
#define CERT_OFFSET_IN_INITIATE_KEY_ESTABLISHMENT_MESSAGE (3 + 2 + 1 + 1)

// Public Key Reconstruction Data (22 bytes)
// Subject (8 bytes)
#define SUBJECT_OFFSET_IN_CERT  22
#define ISSUER_OFFSET_IN_CERT (SUBJECT_OFFSET_IN_CERT + 8)

static int8u invalidEui64[] = {
  0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08
};

static void delayEphemeral(void);

void emberAfPluginTestHarnessChannelMaskAddOrRemoveCommand(void);

extern int32u testHarnessChannelMask;
extern const int32u testHarnessOriginalChannelMask;


void emberAfPluginTestHarnessKeyEstablishmentSetModeCommand(void);
void emberAfPluginTestHarnessCertMangleCommand(void);
void emberAfPluginTestHarnessKeyEstablishmentKeyMangleCommand(void);
void emberAfPluginTestHarnessStatusCommand(void);
void emberAfPluginTestHarnessSetRegistrationCommand(void);
void emberAfPluginTestHarnessSetApsSecurityForClusterCommand(void);
void emberAfPluginTestHarnessPriceSendNewFieldsCommand(void);
void emberAfPluginTestHarnessTcKeepaliveSendCommand(void);
void emberAfPluginTestHarnessTcKeepaliveStartStopCommand(void);
void emberAfPluginTestHarnessOtaImageMangleCommand(void);
void emberAfPluginTestHarnessRadioOnOffCommand(void);
void emberAfPluginTestHarnessSelectSuiteCommand(void);

#if defined(EMBER_AF_PLUGIN_CONCENTRATOR)
void emberAfPluginTestHarnessConcentratorStartStopCommand(void);
#endif

#if defined(EMBER_AF_PLUGIN_NETWORK_FIND)
void emberAfPluginTestHarnessChannelMaskResetClearAllCommand(void);
#endif

void emberAfPluginTestHarnessEnableDisableEndpointCommand(void);
void emberAfPluginTestHarnessEndpointStatusCommand(void);

extern EmberCommandEntry emAfReadWriteAttributeTestCommands[];

#if defined(EMBER_AF_PLUGIN_TRUST_CENTER_NWK_KEY_UPDATE_PERIODIC)
void emberAfPluginTestHarnessKeyUpdateCommand(void);
#endif

// TODO: this should be modified once we've upgraded the generated CLI
// mechanism to conditionally generate CLI based on macros such as these
#if defined(STACK_TEST_HARNESS) || defined(EMBER_AF_GENERATE_CLI)
void emberAfPluginTestHarnessLimitBeaconsOnOffCommand(void);
#endif

#if !defined(EMBER_AF_GENERATE_CLI)

static EmberCommandEntry certMangleCommands[] = {
  emberCommandEntryAction( "length",      emberAfPluginTestHarnessCertMangleCommand, "s",  "Mangles the length of the certificate"),
  emberCommandEntryAction( "issuer",      emberAfPluginTestHarnessCertMangleCommand, "b",  "Changes the issuer in the certificate"),
  emberCommandEntryAction( "corrupt",     emberAfPluginTestHarnessCertMangleCommand, "u",  "Corrupts a single byte in the cert"),
  emberCommandEntryAction( "subject",     emberAfPluginTestHarnessCertMangleCommand, "b",  "Changes the subject (IEEE) of the cert"),
  emberCommandEntryAction( "change-byte", emberAfPluginTestHarnessCertMangleCommand, "uu", "Changes the value of a single byte of the cert"),

  emberCommandEntryTerminator(),
};

static EmberCommandEntry registrationCommands[] = {
  emberCommandEntryAction( "on",  emberAfPluginTestHarnessSetRegistrationCommand, "", "Turns automatic SE registration on."),
  emberCommandEntryAction( "off", emberAfPluginTestHarnessSetRegistrationCommand, "", "Turns automatic SE registration off."),
  emberCommandEntryTerminator(),
};

static EmberCommandEntry apsSecurityForClusterCommands[] = {
  emberCommandEntryAction( "on",  emberAfPluginTestHarnessSetApsSecurityForClusterCommand, "v", "Turns on automatic APS security based on cluster." ),
  emberCommandEntryAction( "off", emberAfPluginTestHarnessSetApsSecurityForClusterCommand, "",  "Turns off automatic APS security based on cluster." ),
  emberCommandEntryTerminator(), 
};

static EmberCommandEntry keyEstablishmentCommands[] = {
  emberCommandEntryAction( "normal-mode",     emberAfPluginTestHarnessKeyEstablishmentSetModeCommand, "",   "Sets key establishment to normal, compliant mode."),
  emberCommandEntryAction( "no-resources",    emberAfPluginTestHarnessKeyEstablishmentSetModeCommand, "",   "All received KE requests will be responded with 'no resources'"),
  emberCommandEntryAction( "out-of-sequence", emberAfPluginTestHarnessKeyEstablishmentSetModeCommand, "u",  "Sends an out-of-sequence KE message based on the passed command ID."),
  emberCommandEntryAction( "timeout",         emberAfPluginTestHarnessKeyEstablishmentSetModeCommand, "" ,  "Artificially creates a timeout by delaying an outgoing message."),
  emberCommandEntryAction( "delay-cbke",      emberAfPluginTestHarnessKeyEstablishmentSetModeCommand, "vv", "Changes the advertised delays by the local device for CBKE."),
  emberCommandEntrySubMenu( "cert-mangle",    certMangleCommands, "Commands to mangle the certificate sent by the local device."),
  emberCommandEntryAction( "default-resp",    emberAfPluginTestHarnessKeyEstablishmentSetModeCommand, "",   "Sends a default response error message in response to initate KE."),
  emberCommandEntryAction( "new-key-policy",  emberAfPluginTestHarnessKeyEstablishmentSetModeCommand, "u",  "Sets the policy of whether the TC allows new KE requests."),
  emberCommandEntryAction( "reset-aps-fc",    emberAfPluginTestHarnessKeyEstablishmentSetModeCommand, "",   "Forces the local device to reset its outgoing APS FC."),
  emberCommandEntryAction( "adv-aps-fc",      emberAfPluginTestHarnessKeyEstablishmentSetModeCommand, "",   "Advances the local device's outgoing APS FC by 4096."),
  emberCommandEntryTerminator(),
};

static EmberCommandEntry priceCommands[] = {
  emberCommandEntryAction( "send-new-fields", emberAfPluginTestHarnessPriceSendNewFieldsCommand, "u", "Controls whether the new SE 1.1 price fields are included."),
  emberCommandEntryTerminator(),
};

static EmberCommandEntry tcKeepaliveCommands[] = {
  emberCommandEntryAction( "send",  emberAfPluginTestHarnessTcKeepaliveSendCommand,      "", "Sends a new TC keepalive." ),
  emberCommandEntryAction( "start", emberAfPluginTestHarnessTcKeepaliveStartStopCommand, "", "Starts the TC keepalive state machine."),
  emberCommandEntryAction( "stop",  emberAfPluginTestHarnessTcKeepaliveStartStopCommand, "", "Stops the TC keepalive state machine."),
  emberCommandEntryTerminator(),
};

static EmberCommandEntry otaCommands[] = {
  emberCommandEntryAction( "image-mangle", 
                          emberAfPluginTestHarnessOtaImageMangleCommand, 
                          "v", 
                          "Mangles the Simple Storage RAM OTA image."),
  emberCommandEntryTerminator(),
};

#if defined(EMBER_AF_PLUGIN_TRUST_CENTER_NWK_KEY_UPDATE_PERIODIC)
static EmberCommandEntry keyUpdateCommands[] = {
  emberCommandEntryAction( "unicast",   emberAfPluginTestHarnessKeyUpdateCommand, "", "Changes TC NWK key update mechanism to unicast with APS security."),
  emberCommandEntryAction( "broadcast", emberAfPluginTestHarnessKeyUpdateCommand, "", "Changes TC NWK key update mechanism to broadcast."),
  emberCommandEntryAction( "now",       emberAfPluginTestHarnessKeyUpdateCommand, "", "Starts a TC NWK key update now"),
  emberCommandEntryTerminator(),
};
#endif

#if defined(EMBER_AF_PLUGIN_CONCENTRATOR)
static EmberCommandEntry concentratorCommands[] = {
  emberCommandEntryAction( "start",
                          emberAfPluginTestHarnessConcentratorStartStopCommand, 
                          "", 
                          "Starts the concentrator's periodic broadcasts." ),
  emberCommandEntryAction("stop",
                          emberAfPluginTestHarnessConcentratorStartStopCommand, 
                          "",
                          "Stops the concentrator's periodic broadcasts." ),
  emberCommandEntryTerminator(),
};
#endif

#if defined(STACK_TEST_HARNESS)

static EmberCommandEntry limitBeaconsCommands[] = {
  emberCommandEntryAction("on", 
                          emberAfPluginTestHarnessLimitBeaconsOnOffCommand, 
                          "", 
                          "Enables a limit to the max number of outgoing beacons."),
  emberCommandEntryAction("off",
                          emberAfPluginTestHarnessLimitBeaconsOnOffCommand, 
                          "", 
                          "Disables a limit to the max number of outgoing beacons."),
  emberCommandEntryTerminator(),
};

static EmberCommandEntry stackTestHarnessCommands[] = {
  emberCommandEntrySubMenu("limit-beacons", 
                           limitBeaconsCommands,
                           "Commands to limit the outgoing beacons." ),
  emberCommandEntryTerminator(),
};
#endif


#if defined(EMBER_AF_PLUGIN_NETWORK_FIND)
static EmberCommandEntry channelMaskCommands[] = {
  emberCommandEntryAction("clear",
                          emberAfPluginTestHarnessChannelMaskResetClearAllCommand, 
                          "",
                          "Clears the channel mask used by network find."),

  emberCommandEntryAction("reset",
                          emberAfPluginTestHarnessChannelMaskResetClearAllCommand,
                          "", 
                          "Resets the channel mask back to the app default"),

  emberCommandEntryAction("all",
                          emberAfPluginTestHarnessChannelMaskResetClearAllCommand,
                          "",
                          "Sets the channel mask to all channels"),

  emberCommandEntryAction("add",
                          emberAfPluginTestHarnessChannelMaskAddOrRemoveCommand,
                          "u",  
                          "Add a channel to the mask"),

  emberCommandEntryAction("remove",
                          emberAfPluginTestHarnessChannelMaskAddOrRemoveCommand,
                          "u",
                          "Removes a channel from the mask"),

  emberCommandEntryTerminator(),
};
#endif

static EmberCommandEntry endpointCommands[] = {
  emberCommandEntryAction("enable",
                          emberAfPluginTestHarnessEnableDisableEndpointCommand,
                          "u", 
                          "Enables the endpont to receive messages and be discovered" ),

  emberCommandEntryAction("disable",
                          emberAfPluginTestHarnessEnableDisableEndpointCommand, 
                          "u", 
                          "Disables the endpont to receive messages and be discovered" ),

  emberCommandEntryAction("status",
                          emberAfPluginTestHarnessEndpointStatusCommand, 
                          "",  
                          "Prints the status of all endpoints"),

  emberCommandEntryTerminator(),
};

static EmberCommandEntry radioCommands[] = {
  emberCommandEntryAction("on", emberAfPluginTestHarnessRadioOnOffCommand, "", 
                          "Turns on the radio if it was previously turned off"),
  emberCommandEntryAction("off", emberAfPluginTestHarnessRadioOnOffCommand, "",
                          "Turns off the radio so that no messages are sent."),
  emberCommandEntryTerminator(),
};

EmberCommandEntry emberAfPluginTestHarnessCommands[] = {
  emberCommandEntryAction( "status",            
                          emberAfPluginTestHarnessStatusCommand,
                          "",
                          "Prints the status of the test harness plugin"),

  emberCommandEntrySubMenu("aps-sec-for-clust",
                           apsSecurityForClusterCommands,
                           "APS Security on clusters"),
  emberCommandEntrySubMenu("registration",
                           registrationCommands,
                           "Enables/disables auto SE registration"),

#if defined(EMBER_AF_PLUGIN_KEY_ESTABLISHMENT)
  emberCommandEntrySubMenu("ke",
                           keyEstablishmentCommands,
                           "Key establishment commands"),
#endif

#if defined(EMBER_AF_PLUGIN_PRICE_SERVER)
  emberCommandEntrySubMenu("price",
                           priceCommands,
                           "Commands for manipulating price server"),
#endif

#if defined(EMBER_AF_PLUGIN_TRUST_CENTER_KEEPALIVE)
  emberCommandEntrySubMenu("tc-keepalive",
                           tcKeepaliveCommands,
                           "Commands for Keepalive messages to TC"),
#endif

#if defined(EMBER_AF_PLUGIN_TRUST_CENTER_NWK_KEY_UPDATE_PERIODIC)
  emberCommandEntrySubMenu("key-update",        
                           keyUpdateCommands,
                           "Trust Center Key update commands"),
#endif

  emberCommandEntrySubMenu("ota",
                           otaCommands,
                           "OTA cluster commands"),
  
#if defined(EMBER_AF_PLUGIN_CONCENTRATOR)
  emberCommandEntrySubMenu("concentrator",
                           concentratorCommands,
                           "Concentrator commands"),
#endif

#if defined(STACK_TEST_HARNESS)
  emberCommandEntrySubMenu("stack",
                           stackTestHarnessCommands,
                           "Stack test harness commands"),
#endif

#if defined(EMBER_AF_PLUGIN_NETWORK_FIND)
  emberCommandEntrySubMenu("channel-mask",
                           channelMaskCommands,
                           "Manipulates the Network Find channel mask"),
#endif

  emberCommandEntrySubMenu("endpoint",
                           endpointCommands,
                           "Commands for manipulating endpoint messaging/discovery."),
  emberCommandEntrySubMenu("radio",
                           radioCommands,
                           "Commands to turn on/off the radio"),
  emberCommandEntrySubMenu("attributes",
                           emAfReadWriteAttributeTestCommands,
                           "Commands to test attributes"),

  emberCommandEntryAction("hash-the-flash",
                          emAfTestHarnessStartImageStampCalculation, 
                          "",
                          "Calculates the image stamp (hash) of all flash pages"),

  emberCommandEntryTerminator(),
};

#endif // !defined(EMBER_AF_GENERATE_CLI)

static int8u certIndexToCorrupt = 0;
static int8u certIndexToChange  = 0;
static int8u certNewByteValue   = 0;
#define TEST_HARNESS_BACK_OFF_TIME_SECONDS 30

#define CBKE_KEY_ESTABLISHMENT_SUITE         0x0001   // per the spec

#define TEST_HARNESS_PRINT_PREFIX "TEST HARNESS: "

// Holds either the ephemeral public key or the 2 SMAC values.
// The SMAC values are the biggest piece of data.
static int8u* delayedData[EMBER_SMAC_SIZE * 2];
static boolean stopRecursion = FALSE;
static int16u cbkeDelaySeconds = 0;
static int8u delayedCbkeOperation = CBKE_OPERATION_GENERATE_KEYS;

EmberEventControl emAfKeyEstablishmentTestHarnessEventControl;

// This is what is reported to the partner of key establishment.
int16u emAfKeyEstablishmentTestHarnessGenerateKeyTime = DEFAULT_EPHEMERAL_DATA_GENERATE_TIME_SECONDS;
int16u emAfKeyEstablishmentTestHarnessConfirmKeyTime = DEFAULT_GENERATE_SHARED_SECRET_TIME_SECONDS;
int16u emAfKeyEstablishmentTestHarnessAdvertisedGenerateKeyTime = DEFAULT_EPHEMERAL_DATA_GENERATE_TIME_SECONDS;

#define NULL_CLUSTER_ID 0xFFFF
static EmberAfClusterId clusterIdRequiringApsSecurity = NULL_CLUSTER_ID;

#if defined(EMBER_AF_PLUGIN_TEST_HARNESS_AUTO_REGISTRATION_START)
  #define AUTO_REG_START TRUE
#else
  #define AUTO_REG_START FALSE
#endif

boolean emAfTestHarnessAllowRegistration = AUTO_REG_START;

static boolean unicastKeyUpdate = FALSE;

//------------------------------------------------------------------------------
// Misc. forward declarations

void emberAfPluginTrustCenterNwkKeyUpdatePeriodicMyEventHandler(void);

#if defined(STACK_TEST_HARNESS)
void emTestHarnessBeaconSuppressSet(boolean enable);
int8u emTestHarnessBeaconSuppressGet(void);
#endif

//------------------------------------------------------------------------------

static void testHarnessPrintVarArgs(PGM_P format,
                                    va_list vargs)
{
  emberAfCoreFlush();
  emberAfCorePrint(TEST_HARNESS_PRINT_PREFIX);
  emberSerialPrintfVarArg(EMBER_AF_PRINT_OUTPUT, format, vargs);
  emberAfCoreFlush();
}

static void testHarnessPrint(PGM_P format,
                               ...)
{
  va_list vargs;
  va_start(vargs, format);
  testHarnessPrintVarArgs(format, vargs);
  va_end(vargs);
}


static void testHarnessPrintln(PGM_P format,
                               ...)
{
  va_list vargs;
  va_start(vargs, format);
  testHarnessPrintVarArgs(format, vargs);
  va_end(vargs);
  emberSerialPrintCarriageReturn(EMBER_AF_PRINT_OUTPUT);
}

static void resetTimeouts(void)
{
  emAfKeyEstablishmentTestHarnessGenerateKeyTime = DEFAULT_EPHEMERAL_DATA_GENERATE_TIME_SECONDS;
  emAfKeyEstablishmentTestHarnessConfirmKeyTime = DEFAULT_GENERATE_SHARED_SECRET_TIME_SECONDS;
  emAfKeyEstablishmentTestHarnessAdvertisedGenerateKeyTime = DEFAULT_EPHEMERAL_DATA_GENERATE_TIME_SECONDS;
}

void emberAfPluginTestHarnessCertMangleCommand(void)
{
  int8u commandChar = emberCurrentCommand->name[0];
  int8u secondChar  = emberCurrentCommand->name[1];
  testHarnessMode = MODE_CERT_MANGLE;

  if (commandChar == 'l') {
    certMangleType = CERT_MANGLE_LENGTH;
    certLengthMod = (int8s)emberSignedCommandArgument(0);
  } else if (commandChar == 'i') {
    certMangleType = CERT_MANGLE_ISSUER;
    emberCopyEui64Argument(0, invalidEui64);
  } else if (commandChar == 'c' ) {
    if(secondChar == 'o'){
      certMangleType = CERT_MANGLE_CORRUPT;
      certIndexToCorrupt = (int8u)emberUnsignedCommandArgument(0);
    } else{
      certMangleType = CERT_MANGLE_VALUE;
      certIndexToChange  = (int8u)emberUnsignedCommandArgument(0);
      certNewByteValue   = (int8u)emberUnsignedCommandArgument(1);
    }
  } else if (commandChar == 's' ) {
    certMangleType = CERT_MANGLE_SUBJECT;
    emberCopyEui64Argument(0, invalidEui64);
  } else {
    testHarnessPrintln("Error: Unknown command.");
    return;
  }

  // Reset this value back to its normal value in case 
  // it was set.
  resetTimeouts();
}

void emberAfPluginTestHarnessKeyEstablishmentSetModeCommand(void)
{
  int8u commandChar0 = emberCurrentCommand->name[0];
  int8u commandChar2 = emberCurrentCommand->name[2];

  if (commandChar0 == 'o') {
    testHarnessMode = MODE_OUT_OF_SEQUENCE;
    respondToCommandWithOutOfSequence = (int8u)emberUnsignedCommandArgument(0);

  } else if (commandChar0 == 'n') {
    if (commandChar2 == '-') {
      testHarnessMode = MODE_NO_RESOURCES;
    } else if (commandChar2 == 'r') {
      testHarnessMode = MODE_NORMAL;
    } else if (commandChar2 == 'w') {
      testHarnessMode = MODE_NORMAL;
      emKeyEstablishmentPolicyAllowNewKeyEntries = 
        (int8u)emberUnsignedCommandArgument(0);
    }

  } else if (commandChar0 == 't') {
    testHarnessMode = MODE_TIMEOUT;

  } else if (commandChar2 == 'l' ) {  // delay-cbke
    int16u delay = (int16u)emberUnsignedCommandArgument(0);
    int16u advertisedDelay = (int16u)emberUnsignedCommandArgument(1);
    testHarnessMode = MODE_DELAY_CBKE;
    cbkeDelaySeconds = delay;
    emAfKeyEstablishmentTestHarnessGenerateKeyTime = delay;
    emAfKeyEstablishmentTestHarnessConfirmKeyTime  = advertisedDelay;
    emAfKeyEstablishmentTestHarnessAdvertisedGenerateKeyTime = advertisedDelay;
    return;

  } else if (commandChar2 == 'f') { // default-resp
    testHarnessMode = MODE_DEFAULT_RESPONSE;
    return;

  } else if (commandChar0 == 'r') {
    testHarnessMode = MODE_NORMAL;
    emAfTestHarnessResetApsFrameCounter();  
  } else if (commandChar0 == 'a') {
    testHarnessMode = MODE_NORMAL;
    emAfTestHarnessAdvanceApsFrameCounter();
  } else {
    testHarnessPrintln("Error: Unknown command.");
    return;
  }

  resetTimeouts();
}

void emberAfPluginTestHarnessSetRegistrationCommand(void)
{
  int8u commandChar1 = emberCurrentCommand->name[1];
  if (commandChar1 == 'n') {
    emAfTestHarnessAllowRegistration = TRUE;
  } else if (commandChar1 == 'f') {
    emAfTestHarnessAllowRegistration = FALSE;
  } else {
    testHarnessPrintln("Error: Unknown command.");
  }
}

void emberAfPluginTestHarnessSetApsSecurityForClusterCommand(void)
{
  int8u commandChar1 = emberCurrentCommand->name[1];
  if (commandChar1 == 'n') {
    clusterIdRequiringApsSecurity = (int16u)emberUnsignedCommandArgument(0);
  } else if (commandChar1 == 'f') {
    clusterIdRequiringApsSecurity = NULL_CLUSTER_ID;
  } else {
    testHarnessPrintln("Error: Unknown command.");
  }

}

#if defined(EMBER_AF_PLUGIN_KEY_ESTABLISHMENT)
void emberAfPluginTestHarnessKeyEstablishmentKeyMangleCommand(void)
{
  testHarnessMode = MODE_KEY_MANGLE;
  keyLengthMod = (int8s)emberSignedCommandArgument(0);
}

boolean emAfKeyEstablishmentTestHarnessMessageSendCallback(int8u message)
{
  int8u i;
  int8u* ptr = (appResponseData
                + CERT_OFFSET_IN_INITIATE_KEY_ESTABLISHMENT_MESSAGE);
  int8u direction = (*appResponseData & ZCL_FRAME_CONTROL_DIRECTION_MASK);

  if (MODE_NORMAL == testHarnessMode) {
    return TRUE;
  }

  if (testHarnessMode == MODE_CERT_MANGLE) {
    if (message == ZCL_INITIATE_KEY_ESTABLISHMENT_REQUEST_COMMAND_ID) {
      if (certMangleType == CERT_MANGLE_LENGTH) {
        if (certLengthMod > 0) {
          ptr = appResponseData + appResponseLength;
          for (i = 0; i < certLengthMod; i++) {
            *ptr = i;
            ptr++;
          }
        }
        appResponseLength += certLengthMod;

        testHarnessPrintln("Mangling certificate length by %p%d bytes",
                           (certLengthMod > 0
                            ? "+"
                            : ""),
                           certLengthMod);

      } else if (certMangleType == CERT_MANGLE_ISSUER
                 || certMangleType == CERT_MANGLE_SUBJECT) {
        ptr += (certMangleType == CERT_MANGLE_ISSUER
                ? ISSUER_OFFSET_IN_CERT
                : SUBJECT_OFFSET_IN_CERT);

        MEMCOPY(ptr, invalidEui64, EUI64_SIZE);
        testHarnessPrintln("Mangling certificate %p to be (>)%X%X%X%X%X%X%X%X",
                           (certMangleType == CERT_MANGLE_ISSUER
                            ? "issuer"
                            : "subject"),
                           invalidEui64[0],
                           invalidEui64[1],
                           invalidEui64[2],
                           invalidEui64[3],
                           invalidEui64[4],
                           invalidEui64[5],
                           invalidEui64[6],
                           invalidEui64[7]);
      
      } else if (certMangleType == CERT_MANGLE_CORRUPT) {
        // We bit flip the byte to make sure it is different than 
        // its real value.
        ptr += certIndexToCorrupt;
        *ptr = ~ (*ptr);
        testHarnessPrintln("Mangling byte at index %d in certificate.", 
                           certIndexToCorrupt);
      } else if (certMangleType == CERT_MANGLE_VALUE) {
        ptr += certIndexToChange;
        *ptr = certNewByteValue;
        testHarnessPrintln("Changing byte at index %u to %u in certificate.", 
                           certIndexToChange,certNewByteValue);        
      }
    } else if (message == ZCL_TERMINATE_KEY_ESTABLISHMENT_COMMAND_ID) {
      if (certMangleType == CERT_MANGLE_CORRUPT) {
        // To simulate that the test harness has NOT cancelled
        // key establishment due to a problem with calculating
        // or comparing the SMAC (acting as KE server), we
        // send a confirm key message instead.  The local server
        // has really cancelled KE but we want to test that the other
        // side will cancel it and send its own terminate message.
        ptr = appResponseData + 2; // jump over frame control and sequence num
        *ptr++ = ZCL_CONFIRM_KEY_DATA_RESPONSE_COMMAND_ID;
        for (i = 0; i < EMBER_SMAC_SIZE; i++) {
          *ptr++ = i;
        }
        appResponseLength = (EMBER_AF_ZCL_OVERHEAD
                             + EMBER_SMAC_SIZE);
        testHarnessPrintln("Rewriting Terminate Message as Confirm Key Message");
      }
    }

  } else if (testHarnessMode == MODE_OUT_OF_SEQUENCE) {
    if (message == respondToCommandWithOutOfSequence) {
      int8u *ptr = appResponseData + 2;  // ZCL Frame control and sequence number

      testHarnessPrintln("Sending out-of-sequence message");

      ptr[0] = (message == ZCL_CONFIRM_KEY_DATA_REQUEST_COMMAND_ID
                ? ZCL_INITIATE_KEY_ESTABLISHMENT_REQUEST_COMMAND_ID
                : (message + 1));

      // Setting the outgoing message to the right length without really
      // filling the message with valid data means there would
      // probably be garbage or bad data.  However the receiving device should
      // check for an invalid command ID first before parsing the specific
      // fields in the message.
      appResponseLength = EMBER_AF_ZCL_OVERHEAD + emAfKeyEstablishMessageToDataSize[message];
    }

  } else if (testHarnessMode == MODE_NO_RESOURCES) {

    // If we are the client then we have to wait until after the first
    // message to send the 'no resources' message.
    if (!(direction == ZCL_FRAME_CONTROL_CLIENT_TO_SERVER
          && message == ZCL_INITIATE_KEY_ESTABLISHMENT_REQUEST_COMMAND_ID)) {
      int8u *ptr = appResponseData + 2;  // ZCL Frame control and sequence number
      *ptr++ = ZCL_TERMINATE_KEY_ESTABLISHMENT_COMMAND_ID;
      *ptr++ = EMBER_ZCL_AMI_KEY_ESTABLISHMENT_STATUS_NO_RESOURCES;
      *ptr++ = TEST_HARNESS_BACK_OFF_TIME_SECONDS;
      *ptr++ = LOW_BYTE(emAfCurrentCbkeSuite);
      *ptr++ = HIGH_BYTE(emAfCurrentCbkeSuite);

      appResponseLength = ptr - appResponseData;
      
      testHarnessPrintln("Sending Terminate: NO_RESOURCES");
    }
    
  } else if (testHarnessMode == MODE_DEFAULT_RESPONSE) {

    // If we are the client then we have to wait until after the first
    // message to send the Default Response message.
    if (!(direction == ZCL_FRAME_CONTROL_CLIENT_TO_SERVER
          && message == ZCL_INITIATE_KEY_ESTABLISHMENT_REQUEST_COMMAND_ID)) {
      int8u *ptr = appResponseData;  
      int8u oldCommandId;
      *ptr++ = direction;
      ptr++; // skip sequence number, it was already written correctly.

      oldCommandId = *ptr;
      *ptr++ = ZCL_DEFAULT_RESPONSE_COMMAND_ID;

      // If we are the client, we send a response to the previous command ID.
      // If we are the server, we send a response to the current command ID.
      // This works because the client -> server and server -> client commands
      // are identical.  The client code will be sending the NEXT command ID,
      // which we are rewriting into a default response.  The server is sending
      // a command ID for the previous client command (a response ID to the
      // request), which has the same command ID as we are rewriting.
      *ptr++ = (direction == ZCL_FRAME_CONTROL_CLIENT_TO_SERVER
                ? oldCommandId - 1
                : oldCommandId);
      *ptr++ = EMBER_ZCL_STATUS_FAILURE;
      
      appResponseLength = ptr - appResponseData;
      
      testHarnessPrintln("Sending Default Response: FAILURE");
    }
    
  } else if (testHarnessMode == MODE_TIMEOUT) {

    // If we are the client then we have to wait until after the first
    // message to induce a timeout.
    if (!(direction == ZCL_FRAME_CONTROL_CLIENT_TO_SERVER
          && message == ZCL_INITIATE_KEY_ESTABLISHMENT_REQUEST_COMMAND_ID)) {
      testHarnessPrintln("Suppressing message to induce timeout.");
      return FALSE;
    }
  } else if (testHarnessMode == MODE_KEY_MANGLE) {
    //Change the length of the emphemeral Key.
    if (message == ZCL_EPHEMERAL_DATA_REQUEST_COMMAND_ID) {
      if (keyLengthMod > 0) {
        ptr = appResponseData + appResponseLength;
        for (i = 0; i < keyLengthMod; i++) {
          *ptr = i;
          ptr++;
        }
      }
      appResponseLength += keyLengthMod;

      testHarnessPrintln("Mangling key length by %p%d bytes",
                         (keyLengthMod > 0
                          ? "+"
                          : ""),
                         keyLengthMod);
    }
  }

  // Send the message
  return TRUE;
}

void emberAfPluginTestHarnessKeyEstablishmentSelectSuiteCommand(void)
{
  int16u suite = (int16u)emberUnsignedCommandArgument(0);
  emAfSkipCheckSupportedCurves(suite);
}

void emberAfPluginTestHarnessKeyEstablishmentSetAvailableSuiteCommand(void)
{
  int16u suite = (int16u)emberUnsignedCommandArgument(0);
  emAfSetAvailableCurves(suite);
}
#endif

void emAfKeyEstablishementTestHarnessEventHandler(void)
{
  emberEventControlSetInactive(emAfKeyEstablishmentTestHarnessEventControl);
  testHarnessPrintln("Test Harness Event Handler fired, Tick: 0x%4X",
                     halCommonGetInt32uMillisecondTick());

  stopRecursion = TRUE;
#if defined(EMBER_AF_PLUGIN_KEY_ESTABLISHMENT)
  testHarnessPrintln("Generating %p callback.",
                     ((delayedCbkeOperation
                       == CBKE_OPERATION_GENERATE_KEYS)
                      ? "emberGenerateCbkeKeysHandler()"
                      : "emberCalculateSmacsHandler()"));
  
  if (delayedCbkeOperation == CBKE_OPERATION_GENERATE_KEYS) {
    emAfPluginKeyEstablishmentGenerateCbkeKeysHandler(EMBER_SUCCESS,
                                                      (EmberPublicKeyData*)delayedData);
  } else {
    emAfPluginKeyEstablishmentCalculateSmacsHandler(EMBER_SUCCESS,
                                                    (EmberSmacData*)delayedData,
                                                    (EmberSmacData*)(delayedData + EMBER_SMAC_SIZE));
  }
#endif
  
  stopRecursion = FALSE;
}

boolean emAfKeyEstablishmentTestHarnessCbkeCallback(int8u cbkeOperation,
                                                    int8u* data1,
                                                    int8u* data2)
{
  if (testHarnessMode != MODE_DELAY_CBKE) {
    return FALSE;
  }

  if (stopRecursion) {
    return FALSE;
  }

  testHarnessPrintln("Delaying %p key callback for %d seconds", 
                     (cbkeOperation == CBKE_OPERATION_GENERATE_KEYS
                      ? "ephemeral"
                      : "confirm"),
                     cbkeDelaySeconds);

  MEMCOPY(delayedData,
          data1, 
          (cbkeOperation == CBKE_OPERATION_GENERATE_KEYS
           ? sizeof(EmberPublicKeyData)
           : sizeof(EmberSmacData)));

  if (data2 != NULL) {
    MEMCOPY(delayedData + sizeof(EmberSmacData),
            data2,
            sizeof(EmberSmacData));
  }

  delayedCbkeOperation = cbkeOperation;
  
  testHarnessPrintln("Test Harness Tick: 0x%4X",
                     halCommonGetInt32uMillisecondTick());
  emberAfEventControlSetDelay(&emAfKeyEstablishmentTestHarnessEventControl, 
                              (((int32u)(cbkeDelaySeconds)) 
                               * MILLISECOND_TICKS_PER_SECOND));  // convert to MS
  return TRUE;
}

void emberAfPluginTestHarnessStatusCommand(void)
{
  emberAfKeyEstablishmentClusterPrint("Test Harness Mode: %p", modeText[testHarnessMode]);  
  if (testHarnessMode == MODE_CERT_MANGLE) {
    emberAfKeyEstablishmentClusterPrintln("");
    emberAfKeyEstablishmentClusterPrint("Cert Mangling Type: %p", certMangleText[certMangleType]);
    if (certMangleType == CERT_MANGLE_LENGTH) {
      emberAfKeyEstablishmentClusterPrint(" (%p%d bytes)",
                                          ((certLengthMod > 0)
                                           ? "+"
                                           : ""),
                                          certLengthMod);
    } else if (certMangleType == CERT_MANGLE_CORRUPT) {
      emberAfKeyEstablishmentClusterPrint(" (index: %d)",
                                          certIndexToCorrupt);
    }
  } else if (testHarnessMode == MODE_DELAY_CBKE) {
    emberAfKeyEstablishmentClusterPrint(" (by %d seconds",
                                        cbkeDelaySeconds);
  }
  emberAfKeyEstablishmentClusterPrintln("");

  emberAfKeyEstablishmentClusterPrintln("Auto SE Registration: %p",
                                        (emAfTestHarnessAllowRegistration
                                         ? "On"
                                         : "Off"));
  emberAfKeyEstablishmentClusterPrint("Additional Cluster Security: ");
  if (clusterIdRequiringApsSecurity == NULL_CLUSTER_ID) {
    emberAfKeyEstablishmentClusterPrintln("off");
  } else {
    emberAfKeyEstablishmentClusterPrintln("0x%2X", 
                                          clusterIdRequiringApsSecurity);
  }

  emberAfKeyEstablishmentClusterPrintln("Publish Price includes SE 1.1 fields: %p",
                                        (emAfTestHarnessSupportForNewPriceFields
                                         ? "yes"
                                         : "no"));
  emberAfKeyEstablishmentClusterFlush();

#if defined(STACK_TEST_HARNESS)  
  {
    int8u beaconsLeft = emTestHarnessBeaconSuppressGet();
    emberAfKeyEstablishmentClusterPrint("Beacon Suppress: %p",
                                        (beaconsLeft == 255
                                         ? "Disabled "
                                         : "Enabled "));
    if (beaconsLeft != 255) {
      emberAfKeyEstablishmentClusterPrint(" (%d left to be sent)",
                                          beaconsLeft);
    }
    emberAfKeyEstablishmentClusterFlush();
    emberAfKeyEstablishmentClusterPrintln("");
  }
#endif

#if defined(EMBER_AF_PLUGIN_NETWORK_FIND)
  {
    emberAfKeyEstablishmentClusterPrint("Channel Mask: ");
    emberAfPrintChannelListFromMask(testHarnessChannelMask);
    emberAfKeyEstablishmentClusterPrintln("");
  }
#endif
}

boolean emberAfClusterSecurityCustomCallback(EmberAfProfileId profileId,
                                             EmberAfClusterId clusterId,
                                             boolean incoming, 
                                             int8u commandId)
{
  return (clusterIdRequiringApsSecurity != NULL_CLUSTER_ID
          && clusterId == clusterIdRequiringApsSecurity);
}


void emberAfPluginTestHarnessPriceSendNewFieldsCommand(void)
{
#if defined(EMBER_AF_PLUGIN_PRICE_SERVER)
  emAfTestHarnessSupportForNewPriceFields = (boolean)emberUnsignedCommandArgument(0);
#else
  testHarnessPrintln("No Price server plugin included.");
#endif
}

void emberAfPluginTestHarnessTcKeepaliveSendCommand(void)
{
#if defined(EMBER_AF_PLUGIN_TRUST_CENTER_KEEPALIVE)
  emAfSendKeepaliveSignal();
#else
  testHarnessPrintln("No TC keepalive plugin included.");
#endif
}

void emberAfPluginTestHarnessTcKeepaliveStartStopCommand(void)
{
#if defined(EMBER_AF_PLUGIN_TRUST_CENTER_KEEPALIVE)

  int8u commandChar2 = emberCurrentCommand->name[2];
  if (commandChar2 == 'o') {  // stop
    emberAfTrustCenterKeepaliveAbortCallback();

  } else if (commandChar2 == 'a') { // start
    emberAfTrustCenterKeepaliveUpdateCallback(TRUE); // registration complete?  
                                                     // assume this is only called when device is done with that

  } else {
    testHarnessPrintln("Unknown keepalive command.");
  }

#else
  testHarnessPrintln("No TC keepalive plugin included.");
#endif
  
}

void emberAfPluginTestHarnessOtaImageMangleCommand(void)
{
#if defined (EMBER_AF_PLUGIN_OTA_STORAGE_SIMPLE_RAM)
  int16u index = (int16u)emberUnsignedCommandArgument(0);
  if (index >= emAfOtaStorageDriveGetImageSize()) {
    testHarnessPrintln("Error: Index %d > image size of %d",
                       index,
                       emAfOtaStorageDriveGetImageSize());
  } else {
    emAfOtaStorageDriverCorruptImage(index);
  }
#else
  testHarnessPrintln("No OTA Storage Simple RAM plugin included");
#endif
}

#if defined(EMBER_AF_PLUGIN_TRUST_CENTER_NWK_KEY_UPDATE_PERIODIC)
void emberAfPluginTestHarnessKeyUpdateCommand(void)
{
  int8u commandChar0 = emberCurrentCommand->name[0];

  if (commandChar0 == 'u') {
    unicastKeyUpdate = TRUE;
  } else if (commandChar0 == 'b') {
    unicastKeyUpdate = FALSE;
  } else if (commandChar0 == 'n') {
    emberAfPluginTrustCenterNwkKeyUpdatePeriodicMyEventHandler();
  }
}

EmberStatus emberAfTrustCenterStartNetworkKeyUpdate(void)
{
  testHarnessPrintln("Using %p key update method",
                     (unicastKeyUpdate
                      ? "unicast"
                      : "broadcast"));
  return (unicastKeyUpdate
          ? emberAfTrustCenterStartUnicastNetworkKeyUpdate()
          : emberAfTrustCenterStartBroadcastNetworkKeyUpdate());
}
#else

void emberAfPluginTestHarnessKeyUpdateCommand(void)
{
  testHarnessPrintln("NWK Key Update Plugin not enabled.");
}

#endif // EMBER_AF_PLUGIN_TRUST_CENTER_NWK_KEY_UPDATE_PERIODIC

#if defined(EMBER_AF_PLUGIN_CONCENTRATOR)
void emberAfPluginTestHarnessConcentratorStartStopCommand(void)
{
  int8u commandChar2 = emberCurrentCommand->name[2];
  if (commandChar2 == 'o') {
    emberConcentratorStopDiscovery();
  } else if (commandChar2 == 'a') {
    emberConcentratorStartDiscovery();
  } else {
    testHarnessPrintln("Error: Unknown command.");
  }
}
#endif // EMBER_AF_PLUGIN_CONCENTRATOR


#if defined(STACK_TEST_HARNESS)
void emberAfPluginTestHarnessLimitBeaconsOnOffCommand(void)
{
  int8u commandChar1 = emberCurrentCommand->name[1];
  if (commandChar1 == 'f') {
    emTestHarnessBeaconSuppressSet(255);
  } else if (commandChar1 == 'n') {
    emTestHarnessBeaconSuppressSet(1);
  } else {
    testHarnessPrintln("Error: Unknown command.");
  }
}
// TODO: this should be modified once we've upgraded the generated CLI
// mechanism to conditionally generate CLI based on macros such as these
#elif defined(EMBER_AF_GENERATE_CLI)
void emberAfPluginTestHarnessLimitBeaconsOnOffCommand(void)
{
}
#endif

#if defined(EMBER_AF_PLUGIN_NETWORK_FIND)

void emberAfPluginTestHarnessChannelMaskAddOrRemoveCommand(void)
{
  int8u commandChar0 = emberCurrentCommand->name[0];
  int8u channel = (int8u)emberUnsignedCommandArgument(0);

  if (channel < 11 || channel > 26) {
    testHarnessPrintln("Error: Invalid channel '%d'.", channel);
    return;
  }
  if (commandChar0 == 'a') {
    testHarnessChannelMask |= (1 << channel);
  } else if (commandChar0 == 'r') {
    testHarnessChannelMask &= ~(1 << channel);
  } else {
    testHarnessPrintln("Error: Unknown command.");
  }
}

void emberAfPluginTestHarnessChannelMaskResetClearAllCommand(void)
{
  int8u commandChar0 = emberCurrentCommand->name[0];

  if (commandChar0 == 'c') {
    testHarnessChannelMask = 0;
  } else if (commandChar0 == 'r') {
    testHarnessChannelMask = testHarnessOriginalChannelMask;
  } else if (commandChar0 == 'a') {
    testHarnessChannelMask = EMBER_ALL_802_15_4_CHANNELS_MASK;
  } else {
    testHarnessPrintln("Error: Unknown command.");
  }
}

#endif

void emberAfPluginTestHarnessEnableDisableEndpointCommand(void)
{
  int8u endpoint = (int8u)emberUnsignedCommandArgument(0);
  int8u commandChar0 = emberCurrentCommand->name[0];

#if defined(EZSP_HOST)
  boolean disable = commandChar0 == 'd';

  ezspSetEndpointFlags(endpoint,
                       (disable
                        ? EZSP_ENDPOINT_DISABLED
                        : EZSP_ENDPOINT_ENABLED));
#else

  testHarnessPrintln("Unsupported on SOC.");

#endif
}

void emberAfPluginTestHarnessEndpointStatusCommand(void)
{
#if defined(EZSP_HOST)
  int8u i;

  for (i = 0; i < emberAfEndpointCount(); i++) {
    int8u endpoint = emberAfEndpointFromIndex(i);
    EzspEndpointFlags flags;
    EzspStatus status = ezspGetEndpointFlags(endpoint,
                                             &flags);
    testHarnessPrintln("EP %d, Flags 0x%2X [%p]",
                       endpoint,
                       flags,
                       ((flags & EZSP_ENDPOINT_ENABLED)
                        ? "Enabled"
                        : "Disabled"));
  }

#else

  testHarnessPrintln("Unsupported on SOC");
  
#endif
}

#define RADIO_OFF_SCAN (EMBER_ACTIVE_SCAN + 1 + 4)  // EM_START_RADIO_OFF_SCAN (from ember-stack.h)

void emberAfPluginTestHarnessRadioOnOffCommand(void)
{
  boolean radioOff = (emberCurrentCommand->name[1] == 'f');
  EmberStatus status;
  if (radioOff) {
    status = emberStartScan(RADIO_OFF_SCAN,
                            0,   // channels (doesn't matter)
                            0);  // duration (doesn't matter)
  } else {
    status = emberStopScan();
  }
  emberAfCorePrintln("Radio %p status: 0x%X", 
                     (radioOff ? "OFF": "ON"),
                     status);
}


//------------------------------------------------------------------------------

