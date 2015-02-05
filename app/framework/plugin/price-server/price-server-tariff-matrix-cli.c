// ****************************************************************************
// * price-server-tariff-matrix-cli.c
// *
// *
// * Copyright 2014 by Silicon Labs. All rights reserved.                  *80*
// ****************************************************************************

#include "app/framework/include/af.h"
#include "app/util/serial/command-interpreter2.h"
#include "app/framework/plugin/price-server/price-server.h"

#if defined(EMBER_AF_PLUGIN_PRICE_SERVER_TARIFF_MATRIX_SUPPORT)

//=============================================================================
// Globals

static EmberAfScheduledTariff tariff;
static EmberAfScheduledPriceMatrix pm;

//=============================================================================
// Functions


// plugin price-server tclear <endpoint:1>
void emAfPriceServerCliTClear(void)
{
  emberAfPriceClearTariffTable(emberUnsignedCommandArgument(0));
}

// plugin price-server pmclear <endpoint:1>
void emAfPriceServerCliPmClear(void) 
{
  emberAfPriceClearPriceMatrixTable(emberUnsignedCommandArgument(0));
}

// plugin price-server twho <provId:4> <label:1-13> <eventId:4> <tariffId:4>
void emAfPriceServerCliTWho(void)
{
  int8u length;
  tariff.providerId = emberUnsignedCommandArgument(0);
  length = emberCopyStringArgument(1,
                                   tariff.tariffLabel + 1,
                                   ZCL_PRICE_CLUSTER_MAXIMUM_RATE_LABEL_LENGTH,
                                   FALSE);
  tariff.tariffLabel[0] = length;
  tariff.issuerEventId = emberUnsignedCommandArgument(2);
  tariff.issuerTariffId = emberUnsignedCommandArgument(3);
}

// plugin price-server twhat <type:1> <unitOfMeas:1> <curr:2> <ptd:1> <prt:1> <btu:1> <blockMode:1>
void emAfPriceServerCliTWhat(void)
{
  tariff.tariffTypeChargingScheme = (int8u)emberUnsignedCommandArgument(0);
  tariff.unitOfMeasure = (int8u)emberUnsignedCommandArgument(1);
  tariff.currency = (int16u)emberUnsignedCommandArgument(2);
  tariff.priceTrailingDigit = (int8u)emberUnsignedCommandArgument(3);
  tariff.numberOfPriceTiersInUse = (int8u)emberUnsignedCommandArgument(4);
  tariff.numberOfBlockThresholdsInUse = (int8u)emberUnsignedCommandArgument(5);
  tariff.tierBlockMode = (int8u)emberUnsignedCommandArgument(6);
}

// plugin price-server twhen <startTime:4>
void emAfPriceServerCliTWhen(void)
{
  tariff.startTime = emberUnsignedCommandArgument(0);
}

// plugin price-server tariff <standingCharge:4> <btm:4> <btd:4>
void emAfPriceServerCliTariff(void)
{
  tariff.standingCharge = emberUnsignedCommandArgument(0);
  tariff.blockThresholdMultiplier = emberUnsignedCommandArgument(1);
  tariff.blockThresholdDivisor = emberUnsignedCommandArgument(2);
}


// plugin price-server tstatus <endpoint:1> <index:1> <status:1>
void emAfPriceServerCliTStatus(void)
{
  int8u endpoint = (int8u)emberUnsignedCommandArgument(0);
  int8u index = (int8u)emberUnsignedCommandArgument(1);
  int8u status = (int8u)emberUnsignedCommandArgument(2);

  if (status == 0) {
    tariff.status |= CURRENT;
  } else {
    tariff.status |= FUTURE;
  }

  if (!emberAfPriceSetTariffTableEntry(endpoint,
                                       index,
                                       &tariff)) {
    emberAfPriceClusterPrintln("tariff entry %d not present", index);
  }
}

// plugin price-server tget <endpoint:1> <index:1>
void emAfPriceServerCliTGet(void)
{
  int8u endpoint = (int8u)emberUnsignedCommandArgument(0);
  int8u index = (int8u)emberUnsignedCommandArgument(1);
  if (!emberAfPriceGetTariffTableEntry(endpoint, index, &tariff)) {
    emberAfPriceClusterPrintln("tariff entry %d not present", index);
  }
}

// plugin price-server tprint <endpoint:1>
void emAfPriceServerCliTPrint(void)
{
  int8u endpoint = (int8u)emberUnsignedCommandArgument(0);
  emberAfPricePrintTariffTable(endpoint);
}

// plugin price-server tsprint
void emAfPriceServerCliTSPrint(void)
{
  emberAfPricePrintTariff(&tariff);
}

// plugin price-server pmprint <endpoint:1>
void emAfPriceServerCliPmPrint(void)
{
  int8u endpoint = (int8u)emberUnsignedCommandArgument(0);
  emberAfPricePrintPriceMatrixTable(endpoint);
}

// plugin price-server pm set-metadata <endpoint:1> <providerId:4> <issuerEventId:4> 
//                                     <issuerTariffId:4> <startTime:4> <status:1>
void emAfPriceServerCliPmSetMetadata(void)
{
  int8u endpoint = (int8u)emberUnsignedCommandArgument(0);
  int32u tariffId = (int32u)emberUnsignedCommandArgument(3);
  EmberAfScheduledTariff t;

  if (!(emberAfPriceGetTariffByIssuerTariffId(endpoint, tariffId, &t)
        || tariff.issuerTariffId == tariffId)) {
    emberAfPriceClusterPrint("Invalid issuer tariff ID; no corresponding tariff found.\n");
    return;
  }

  pm.providerId = (int32u)emberUnsignedCommandArgument(1);
  pm.issuerEventId = (int32u)emberUnsignedCommandArgument(2);
  pm.issuerTariffId = tariffId;
  pm.startTime = (int32u)emberUnsignedCommandArgument(4);
  pm.status = (int8u)emberUnsignedCommandArgument(5);
}

// plugin price-server pm set-provider <providerId:4>
void emAfPriceServerCliPmSetProvider(void)
{
  pm.providerId = (int32u)emberUnsignedCommandArgument(0);
}

// plugin price-server pm set-event <issuerEventId:4>
void emAfPriceServerCliPmSetEvent(void)
{
  pm.issuerEventId = (int32u)emberUnsignedCommandArgument(0);
}

// plugin price-server pm set-tariff <issuerTariffId:4>
void emAfPriceServerCliPmSetTariff(void)
{
  pm.issuerTariffId = (int32u)emberUnsignedCommandArgument(0);
}

// plugin price-server pm set-time <startTime:4>
void emAfPriceServerCliPmSetTime(void)
{
  pm.startTime = (int32u)emberUnsignedCommandArgument(0);
}

// plugin price-server pm set-status <status:1>
void emAfPriceServerCliPmSetStatus(void)
{
  pm.status = (int8u)emberUnsignedCommandArgument(0);
}

// plugin price-server pm get <endpoint:1> <index:1>
void emAfPriceServerCliPmGet(void)
{
  int8u endpoint = (int8u)emberUnsignedCommandArgument(0);
  int8u index = (int8u)emberUnsignedCommandArgument(1);
  emberAfPriceGetPriceMatrixTableEntry(endpoint, index, &pm);
}

// plugin price-server pm set <endpoint:1> <index:1>
void emAfPriceServerCliPmSet(void)
{
  int8u endpoint = (int8u)emberUnsignedCommandArgument(0);
  int8u index = (int8u)emberUnsignedCommandArgument(1);
  EmberAfScheduledTariff t;

  if (!(emberAfPriceGetTariffByIssuerTariffId(endpoint, pm.issuerTariffId, &t)
        || tariff.issuerTariffId == pm.issuerTariffId)) {
    emberAfPriceClusterPrint("Invalid issuer tariff ID; no corresponding tariff found.\n");
    return;
  }

  emberAfPriceSetPriceMatrixTableEntry(endpoint, index, &pm);
}

// plugin price-server pm put <endpoint:1> <tier:1> <block:1> <price:4>
void emAfPriceServerCliPmPut(void)
{
  int8u endpoint = (int8u)emberUnsignedCommandArgument(0);
  int8u tier = (int8u)emberUnsignedCommandArgument(1);
  int8u block = (int8u)emberUnsignedCommandArgument(2);
  int32u price = (int32u)emberUnsignedCommandArgument(3);
  int8u chargingScheme;
  EmberAfScheduledTariff t;

  if (pm.issuerTariffId == tariff.issuerTariffId) {
    t = tariff;
    chargingScheme = tariff.tariffTypeChargingScheme;
  } else {
    boolean found = emberAfPriceGetTariffByIssuerTariffId(endpoint, pm.issuerTariffId, &t);
    if (!found) {
      emberAfPriceClusterPrint("Invalid issuer tariff ID in price matrix; no corresponding tariff found.\n"); 
      return;
    } else {
      chargingScheme = t.tariffTypeChargingScheme;
    }
  }

  if (tier >= t.numberOfPriceTiersInUse 
      || block > t.numberOfBlockThresholdsInUse) {
    emberAfPriceClusterPrint("Invalid index into price matrix. Value not set.\n");
    return;
  }

  switch (chargingScheme >> 4) {
    case 0: // TOU only
      pm.matrix.tier[tier] = price;
      break;
    case 1: // Block only
      pm.matrix.blockAndTier[tier][0] = price;
      break;
    case 2:
    case 3: // TOU and Block
      pm.matrix.blockAndTier[tier][block] = price;
      break;
    default:
      emberAfDebugPrintln("Invalid tariff type / charging scheme.");
      break;
  }
}

// plugin price-server pm fill-tier <endpoint:1> <tier:1> <price:4>
void emAfPriceServerCliPmFillTier(void)
{
  int8u endpoint = (int8u)emberUnsignedCommandArgument(0);
  int8u tier = (int8u)emberUnsignedCommandArgument(1);
  int32u price = (int32u)emberUnsignedCommandArgument(2);
  int8u chargingScheme, i;
  EmberAfScheduledTariff t;

  if (pm.issuerTariffId == tariff.issuerTariffId) {
    t = tariff;
    chargingScheme = tariff.tariffTypeChargingScheme;
  } else {
    boolean found = emberAfPriceGetTariffByIssuerTariffId(endpoint, pm.issuerTariffId, &t);
    if (!found) {
      emberAfPriceClusterPrint("Invalid issuer tariff ID in price matrix; no corresponding tariff found.\n"); 
      return;
    } else {
      chargingScheme = t.tariffTypeChargingScheme;
    }
  }

  if (tier >= t.numberOfPriceTiersInUse) {
    emberAfPriceClusterPrint("Tier exceeds number of price tiers in use for this tariff. Values not set.\n");
    return;
  }

  switch (chargingScheme >> 4) {
    case 0: // TOU only
      pm.matrix.tier[tier] = price;
      break;
    case 1: // Block only
      pm.matrix.blockAndTier[tier][0] = price;
      break;
    case 2:
    case 3: // TOU and Block
      for (i = 0; i < t.numberOfBlockThresholdsInUse + 1; i++) {
        pm.matrix.blockAndTier[tier][i] = price;
      }
      break;
    default:
      emberAfDebugPrintln("Invalid tariff type / charging scheme.");
      break;
  }
}

// plugin price-server pm fill-tier <endpoint:1> <block:1> <price:4>
void emAfPriceServerCliPmFillBlock(void)
{
  int8u endpoint = (int8u)emberUnsignedCommandArgument(0);
  int8u block = (int8u)emberUnsignedCommandArgument(1);
  int32u price = (int32u)emberUnsignedCommandArgument(2);
  int8u chargingScheme, i;
  EmberAfScheduledTariff t;

  if (pm.issuerTariffId == tariff.issuerTariffId) {
    t = tariff;
    chargingScheme = tariff.tariffTypeChargingScheme;
  } else {
    boolean found = emberAfPriceGetTariffByIssuerTariffId(endpoint, pm.issuerTariffId, &t);
    if (!found) {
      emberAfPriceClusterPrint("Invalid issuer tariff ID in price matrix; no corresponding tariff found.\n"); 
      return;
    } else {
      chargingScheme = t.tariffTypeChargingScheme;
    }
  }

  if ( block > t.numberOfBlockThresholdsInUse) {
    emberAfPriceClusterPrint("Block exceeds number of block thresholds in use for this tariff. Values not set.\n");
    return;
  }

  switch (chargingScheme >> 4) {
    case 0: // TOU only
      for (i = 0; i < t.numberOfPriceTiersInUse; i++) {
        pm.matrix.tier[i] = price;
      }
      break;
    case 1: // Block only
      for (i = 0; i < t.numberOfPriceTiersInUse; i++) {
        pm.matrix.blockAndTier[i][0] = price;
      }
      break;
    case 2:
    case 3: // TOU and Block
      for (i = 0; i < t.numberOfPriceTiersInUse; i++) {
        pm.matrix.blockAndTier[i][block] = price;
      }
      break;
    default:
      emberAfDebugPrintln("Invalid tariff type / charging scheme.");
      break;
  }
}

#else // !defined(EMBER_AF_PLUGIN_PRICE_SERVER_TARIFF_MATRIX_SUPPORT)



#endif // defined(EMBER_AF_PLUGIN_PRICE_SERVER_TARIFF_MATRIX_SUPPORT)
