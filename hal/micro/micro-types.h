/** @file hal/micro/micro-types.h
 *
 * @brief
 * This file handles defines and enums related to all the micros.
 *
 * THIS IS A GENERATED FILE.  DO NOT EDIT.
 *
 * <!-- Copyright 2014 Silicon Laboratories, Inc.                        *80*-->
 */

#ifndef __MICRO_DEFINES_H__
#define __MICRO_DEFINES_H__

#ifndef DOXYGEN_SHOULD_SKIP_THIS

// Below is a list of PLATFORM and MICRO values that are used to define the
// chips our code runs on. These values are used in EBL headers, bootloader
// query response messages, and possibly other places in the code
// -----------------------------------------------------------------------------
// PLAT 1 was the AVR ATMega, no longer supported (still used for EMBER_TEST)
// for PLAT 1, MICRO 1 is the avr64
// for PLAT 1, MICRO 2 is the avr128
// for PLAT 1, MICRO 3 is the avr32

// PLAT 2 is the XAP2b
// for PLAT 2, MICRO 1 is the em250
// for PLAT 2, MICRO 2 is the em260

// PLAT 3 was the MSP 430, no longer supported

// PLAT 4 is the CortexM3
// for PLAT 4, MICRO 1 is the em350
// for PLAT 4, MICRO 2 is the em360
// for PLAT 4, MICRO 3 is the em357
// for PLAT 4, MICRO 4 is the em367
// for PLAT 4, MICRO 5 is the em351
// for PLAT 4, MICRO 6 is the em35x
// for PLAT 4, MICRO 7 is the stm32w108
// for PLAT 4, MICRO 8 is the em3588
// for PLAT 4, MICRO 9 is the em359
// for PLAT 4, MICRO 10 is the em3581
// for PLAT 4, MICRO 11 is the em3582
// for PLAT 4, MICRO 12 is the em3585
// for PLAT 4, MICRO 13 is the em3586
// for PLAT 4, MICRO 14 is the em3587
// for PLAT 4, MICRO 15 is the sky66107
// for PLAT 4, MICRO 16 is the em3597
// for PLAT 4, MICRO 17 is the em356
// for PLAT 4, MICRO 18 is the em3598
// for PLAT 4, MICRO 19 is the em3591
// for PLAT 4, MICRO 20 is the em3592
// for PLAT 4, MICRO 21 is the em3595
// for PLAT 4, MICRO 22 is the em3596

// PLAT 5 is the C8051
// for PLAT 5, MICRO 1 is the siF930
// for PLAT 5, MICRO 2 is the siF960
// for PLAT 5, MICRO 3 is the cobra
// for PLAT 5, MICRO 4 is the siF370
// for PLAT 5, MICRO 5 is the siF393

// -----------------------------------------------------------------------------

// Create an enum for the platform types
enum {
  EMBER_PLATFORM_UNKNOWN    = 0,
  EMBER_PLATFORM_AVR_ATMEGA = 1,
  EMBER_PLATFORM_XAP2B      = 2,
  EMBER_PLATFORM_MSP_430    = 3,
  EMBER_PLATFORM_CORTEXM3   = 4,
  EMBER_PLATFORM_C8051      = 5,
  EMBER_PLATFORM_MAX_VALUE
};
typedef int16u EmberPlatformEnum;

#define EMBER_PLATFORM_STRINGS \
  "Unknown",                   \
  "Test",                      \
  "XAP2b",                     \
  "MSP 430",                   \
  "CortexM3",                  \
  "C8051",                     \
  NULL,

// Create an enum for all of the different AVR ATMega micros we support
enum {
  EMBER_MICRO_AVR_ATMEGA_UNKNOWN   = 0,
  EMBER_MICRO_AVR_ATMEGA_64        = 1,
  EMBER_MICRO_AVR_ATMEGA_128       = 2,
  EMBER_MICRO_AVR_ATMEGA_32        = 3,
  EMBER_MICRO_AVR_ATMEGA_MAX_VALUE
};
typedef int16u EmberMicroAvrAtmegaEnum;

#define EMBER_MICRO_AVR_ATMEGA_STRINGS \
  "Unknown",                           \
  "avr64",                             \
  "avr128",                            \
  "avr32",                             \
  NULL,

// Create an enum for all of the different XAP2b micros we support
enum {
  EMBER_MICRO_XAP2B_UNKNOWN   = 0,
  EMBER_MICRO_XAP2B_EM250     = 1,
  EMBER_MICRO_XAP2B_EM260     = 2,
  EMBER_MICRO_XAP2B_MAX_VALUE
};
typedef int16u EmberMicroXap2bEnum;

#define EMBER_MICRO_XAP2B_STRINGS \
  "Unknown",                      \
  "em250",                        \
  "em260",                        \
  NULL,

// Create an enum for all of the different CortexM3 micros we support
enum {
  EMBER_MICRO_CORTEXM3_UNKNOWN   = 0,
  EMBER_MICRO_CORTEXM3_EM350     = 1,
  EMBER_MICRO_CORTEXM3_EM360     = 2,
  EMBER_MICRO_CORTEXM3_EM357     = 3,
  EMBER_MICRO_CORTEXM3_EM367     = 4,
  EMBER_MICRO_CORTEXM3_EM351     = 5,
  EMBER_MICRO_CORTEXM3_EM35X     = 6,
  EMBER_MICRO_CORTEXM3_STM32W108 = 7,
  EMBER_MICRO_CORTEXM3_EM3588    = 8,
  EMBER_MICRO_CORTEXM3_EM359     = 9,
  EMBER_MICRO_CORTEXM3_EM3581    = 10,
  EMBER_MICRO_CORTEXM3_EM3582    = 11,
  EMBER_MICRO_CORTEXM3_EM3585    = 12,
  EMBER_MICRO_CORTEXM3_EM3586    = 13,
  EMBER_MICRO_CORTEXM3_EM3587    = 14,
  EMBER_MICRO_CORTEXM3_SKY66107  = 15,
  EMBER_MICRO_CORTEXM3_EM3597    = 16,
  EMBER_MICRO_CORTEXM3_EM356     = 17,
  EMBER_MICRO_CORTEXM3_EM3598    = 18,
  EMBER_MICRO_CORTEXM3_EM3591    = 19,
  EMBER_MICRO_CORTEXM3_EM3592    = 20,
  EMBER_MICRO_CORTEXM3_EM3595    = 21,
  EMBER_MICRO_CORTEXM3_EM3596    = 22,
  EMBER_MICRO_CORTEXM3_MAX_VALUE
};
typedef int16u EmberMicroCortexM3Enum;

#define EMBER_MICRO_CORTEXM3_STRINGS \
  "Unknown",                         \
  "em350",                           \
  "em360",                           \
  "em357",                           \
  "em367",                           \
  "em351",                           \
  "em35x",                           \
  "stm32w108",                       \
  "em3588",                          \
  "em359",                           \
  "em3581",                          \
  "em3582",                          \
  "em3585",                          \
  "em3586",                          \
  "em3587",                          \
  "sky66107",                        \
  "em3597",                          \
  "em356",                           \
  "em3598",                          \
  "em3591",                          \
  "em3592",                          \
  "em3595",                          \
  "em3596",                          \
  NULL,

// Create an enum for all of the different C8051 micros we support
enum {
  EMBER_MICRO_C8051_UNKNOWN   = 0,
  EMBER_MICRO_C8051_SIF930    = 1,
  EMBER_MICRO_C8051_SIF960    = 2,
  EMBER_MICRO_C8051_COBRA     = 3,
  EMBER_MICRO_C8051_SIF370    = 4,
  EMBER_MICRO_C8051_SIF393    = 5,
  EMBER_MICRO_C8051_MAX_VALUE
};
typedef int16u EmberMicroC8051Enum;

#define EMBER_MICRO_C8051_STRINGS \
  "Unknown",                      \
  "siF930",                       \
  "siF960",                       \
  "cobra",                        \
  "siF370",                       \
  "siF393",                       \
  NULL,

// A dummy type declared to allow generically passing around this
// data type.  Micro specific enums (such as EmberMicroCortexM3Enum)
// are required to be the same size as this.
typedef int16u EmberMicroEnum;

// Determine what micro and platform that we're supposed to target using the
// defines passed in at build time. Then set the PLAT and MICRO defines based
// on what was passed in
#if ((! defined(EZSP_HOST)) && (! defined(UNIX_HOST)))

#if defined(EMBER_TEST)
  #define PLAT EMBER_PLATFORM_AVR_ATMEGA
  #define MICRO 2
#elif defined(AVR_ATMEGA)
  #define PLAT EMBER_PLATFORM_AVR_ATMEGA
  #if defined(AVR_ATMEGA_64)
    #define MICRO EMBER_MICRO_AVR_ATMEGA_64
  #elif defined(AVR_ATMEGA_128)
    #define MICRO EMBER_MICRO_AVR_ATMEGA_128
  #elif defined(AVR_ATMEGA_32)
    #define MICRO EMBER_MICRO_AVR_ATMEGA_32
  #endif
#elif defined(XAP2B)
  #define PLAT EMBER_PLATFORM_XAP2B
  #if defined(XAP2B_EM250)
    #define MICRO EMBER_MICRO_XAP2B_EM250
  #elif defined(XAP2B_EM260)
    #define MICRO EMBER_MICRO_XAP2B_EM260
  #endif
#elif defined(CORTEXM3)
  #define PLAT EMBER_PLATFORM_CORTEXM3
  #if defined(CORTEXM3_EM350)
    #define MICRO EMBER_MICRO_CORTEXM3_EM350
  #elif defined(CORTEXM3_EM360)
    #define MICRO EMBER_MICRO_CORTEXM3_EM360
  #elif defined(CORTEXM3_EM357)
    #define MICRO EMBER_MICRO_CORTEXM3_EM357
  #elif defined(CORTEXM3_EM367)
    #define MICRO EMBER_MICRO_CORTEXM3_EM367
  #elif defined(CORTEXM3_EM351)
    #define MICRO EMBER_MICRO_CORTEXM3_EM351
  #elif defined(CORTEXM3_EM35X)
    #define MICRO EMBER_MICRO_CORTEXM3_EM35X
  #elif defined(CORTEXM3_STM32W108)
    #define MICRO EMBER_MICRO_CORTEXM3_STM32W108
  #elif defined(CORTEXM3_EM3588)
    #define MICRO EMBER_MICRO_CORTEXM3_EM3588
  #elif defined(CORTEXM3_EM359)
    #define MICRO EMBER_MICRO_CORTEXM3_EM359
  #elif defined(CORTEXM3_EM3581)
    #define MICRO EMBER_MICRO_CORTEXM3_EM3581
  #elif defined(CORTEXM3_EM3582)
    #define MICRO EMBER_MICRO_CORTEXM3_EM3582
  #elif defined(CORTEXM3_EM3585)
    #define MICRO EMBER_MICRO_CORTEXM3_EM3585
  #elif defined(CORTEXM3_EM3586)
    #define MICRO EMBER_MICRO_CORTEXM3_EM3586
  #elif defined(CORTEXM3_EM3587)
    #define MICRO EMBER_MICRO_CORTEXM3_EM3587
  #elif defined(CORTEXM3_SKY66107)
    #define MICRO EMBER_MICRO_CORTEXM3_SKY66107
  #elif defined(CORTEXM3_EM3597)
    #define MICRO EMBER_MICRO_CORTEXM3_EM3597
  #elif defined(CORTEXM3_EM356)
    #define MICRO EMBER_MICRO_CORTEXM3_EM356
  #elif defined(CORTEXM3_EM3598)
    #define MICRO EMBER_MICRO_CORTEXM3_EM3598
  #elif defined(CORTEXM3_EM3591)
    #define MICRO EMBER_MICRO_CORTEXM3_EM3591
  #elif defined(CORTEXM3_EM3592)
    #define MICRO EMBER_MICRO_CORTEXM3_EM3592
  #elif defined(CORTEXM3_EM3595)
    #define MICRO EMBER_MICRO_CORTEXM3_EM3595
  #elif defined(CORTEXM3_EM3596)
    #define MICRO EMBER_MICRO_CORTEXM3_EM3596
  #endif
#elif defined(C8051)
  #define PLAT EMBER_PLATFORM_C8051
  #if defined(C8051_SIF930)
    #define MICRO EMBER_MICRO_C8051_SIF930
  #elif defined(C8051_SIF960)
    #define MICRO EMBER_MICRO_C8051_SIF960
  #elif defined(C8051_COBRA)
    #define MICRO EMBER_MICRO_C8051_COBRA
  #elif defined(C8051_SIF370)
    #define MICRO EMBER_MICRO_C8051_SIF370
  #elif defined(C8051_SIF393)
    #define MICRO EMBER_MICRO_C8051_SIF393
  #endif
#endif

#endif // ((! defined(EZSP_HOST)) && (! defined(UNIX_HOST)))

// Define distinct literal values for each phy. These values are used in the
// bootloader query response message.
// PHY 0 is NULL
// PHY 1 is em2420 (no longer supported)
// PHY 2 is em250
// PHY 3 is em3XX
// PHY 4 is si446x
// PHY 5 is cobra
// PHY 6 is PRO2+
// PHY 7 is si4455

enum {
  EMBER_PHY_NULL      = 0,
  EMBER_PHY_EM2420    = 1,
  EMBER_PHY_EM250     = 2,
  EMBER_PHY_EM3XX     = 3,
  EMBER_PHY_SI446X    = 4,
  EMBER_PHY_COBRA     = 5,
  EMBER_PHY_PRO2PLUS  = 6,
  EMBER_PHY_SI4455    = 7,
  EMBER_PHY_MAX_VALUE
};
typedef int16u EmberPhyEnum;

#define EMBER_PHY_STRINGS \
  "NULL",                 \
  "em2420",               \
  "em250",                \
  "em3XX",                \
  "si446x",               \
  "cobra",                \
  "PRO2+",                \
  "si4455",               \
  NULL,

#if defined(PHY_NULL)
  #define PHY EMBER_PHY_NULL
#elif defined(PHY_EM2420) || defined(PHY_EM2420B)
  #error em2420 PHY is no longer supported
#elif defined(PHY_EM250) || defined(PHY_EM250B)
  #define PHY EMBER_PHY_EM250
#elif defined(PHY_EM3XX)
  #define PHY EMBER_PHY_EM3XX
#elif defined(PHY_SI446X_US) || defined(PHY_SI446X_EU) || defined(PHY_SI446X_CN) || defined(PHY_SI446X_JP)
  #define PHY EMBER_PHY_SI446X
#elif defined(PHY_COBRA)
  #define PHY EMBER_PHY_COBRA
#elif defined(PHY_PRO2PLUS)
  #define PHY EMBER_PHY_PRO2PLUS
#elif defined(PHY_SI4455)
  #define PHY EMBER_PHY_SI4455
#endif

typedef struct {
  EmberPlatformEnum platform;
  EmberMicroEnum    micro;
  EmberPhyEnum      phy;
} EmberChipTypeStruct;

// load up any chip-specific feature defines
#if defined(PLAT) && defined(MICRO) && PLAT == EMBER_PLATFORM_CORTEXM3
  #include "cortexm3/micro-features.h"
#endif

#endif // DOXYGEN_SHOULD_SKIP_THIS

#endif // __MICRO_DEFINES_H__
