// ****************************************************************************
// * price-server-tariff-matrix.c
// *
// *
// * Copyright 2014 by Silicon Labs. All rights reserved.                  *80*
// ****************************************************************************

#include "app/framework/include/af.h"
#include "app/framework/util/common.h"
#include "price-server.h"

#if defined(EMBER_AF_PLUGIN_PRICE_SERVER_TARIFF_MATRIX_SUPPORT)

//=============================================================================
// Globals

static EmberAfScheduledTariff tariffTable[EMBER_AF_PRICE_CLUSTER_SERVER_ENDPOINT_COUNT][EMBER_AF_PLUGIN_PRICE_SERVER_TARIFF_TABLE_SIZE];
static EmberAfScheduledPriceMatrix pmTable[EMBER_AF_PRICE_CLUSTER_SERVER_ENDPOINT_COUNT][EMBER_AF_PLUGIN_PRICE_SERVER_TARIFF_TABLE_SIZE];

//=============================================================================
// Functions

void emberAfPriceClearTariffTable(int8u endpoint)
{
  int8u i; 
  int8u ep = emberAfFindClusterServerEndpointIndex(endpoint, ZCL_PRICE_CLUSTER_ID);

  if (ep == 0xFF) {
    return;
  }

  for (i = 0; i < EMBER_AF_PLUGIN_PRICE_SERVER_TARIFF_TABLE_SIZE; i++) {
    tariffTable[ep][i].providerId = 0;
    tariffTable[ep][i].issuerEventId = 0;
    tariffTable[ep][i].issuerTariffId = 0;
    tariffTable[ep][i].startTime = 0;
    tariffTable[ep][i].tariffTypeChargingScheme = 0;
    tariffTable[ep][i].status = 0;
  }
}

void emberAfPriceClearPriceMatrixTable(int8u endpoint)
{
  int8u i;
  int8u ep = emberAfFindClusterServerEndpointIndex(endpoint, ZCL_PRICE_CLUSTER_ID);

  if (ep == 0xFF) {
    return;
  }

  for (i = 0; i < EMBER_AF_PLUGIN_PRICE_SERVER_TARIFF_TABLE_SIZE; i++) {
    pmTable[ep][i].providerId = 0;
    pmTable[ep][i].issuerEventId = 0;
    pmTable[ep][i].issuerTariffId = 0;
    pmTable[ep][i].startTime = 0;
    pmTable[ep][i].status = 0;
  }
}


// Retrieves the tariff at the index. Returns FALSE if the index is invalid.
boolean emberAfPriceGetTariffTableEntry(int8u endpoint,
                                        int8u index,
                                        EmberAfScheduledTariff *tariff)
{
  int8u ep = emberAfFindClusterServerEndpointIndex(endpoint, ZCL_PRICE_CLUSTER_ID);
  if (ep == 0xFF || index == 0xFF) {
    return FALSE;
  }

  if (index < EMBER_AF_PLUGIN_PRICE_SERVER_TARIFF_TABLE_SIZE) {
    emberAfDebugPrintln("emberAfPriceGetTariffTableEntry: retrieving tariff at index %d, %d", ep, index);
    MEMCOPY(tariff, &tariffTable[ep][index], sizeof(EmberAfScheduledTariff));
    return TRUE;
  }

  return FALSE;
}

// Retrieves the tariff at the index. Returns FALSE if the index is invalid.
boolean emberAfPriceGetPriceMatrixTableEntry(int8u endpoint,
                                             int8u index,
                                             EmberAfScheduledPriceMatrix *pm)
{
  int8u ep = emberAfFindClusterServerEndpointIndex(endpoint, ZCL_PRICE_CLUSTER_ID);
  if (ep == 0xFF || index == 0xFF) {
    return FALSE;
  }

  if (index < EMBER_AF_PLUGIN_PRICE_SERVER_TARIFF_TABLE_SIZE) {
    emberAfDebugPrintln("emberAfPriceGetTariffTableEntry: retrieving tariff at index %d, %d", ep, index);
    MEMCOPY(pm, &pmTable[ep][index], sizeof(EmberAfScheduledPriceMatrix));
    return TRUE;
  }

  return FALSE;
}

// Retrieves the tariff with the corresponding issuer tariff ID. Returns FALSE
// if the tariff is not found or is invalid.
boolean emberAfPriceGetTariffByIssuerTariffId(int8u endpoint,
                                              int32u issuerTariffId,
                                              EmberAfScheduledTariff *tariff)
{
  int8u i;
  int8u ep = emberAfFindClusterServerEndpointIndex(endpoint, ZCL_PRICE_CLUSTER_ID);

  if (ep == 0xFF) {
    return FALSE;
  }

  for (i = 0; i < EMBER_AF_PLUGIN_PRICE_SERVER_TARIFF_TABLE_SIZE; i++) {
    EmberAfScheduledTariff *lookup = &tariffTable[ep][i];
    if (lookup->status != 0 
        && lookup->issuerTariffId == issuerTariffId) {
      MEMCOPY(tariff, lookup, sizeof(EmberAfScheduledTariff));
      return TRUE;
    }
  }

  return FALSE;
}

// Retrieves the price matrix with the corresponding issuer tariff ID. Returns FALSE
// if the price matrix is not found or is invalid.
boolean emberAfPriceGetPriceMatrixByIssuerTariffId(int8u endpoint,
                                                   int32u issuerTariffId,
                                                   EmberAfScheduledPriceMatrix *pm)
{
  int8u i;
  int8u ep = emberAfFindClusterServerEndpointIndex(endpoint, ZCL_PRICE_CLUSTER_ID);

  if (ep == 0xFF) {
    return FALSE;
  }

  for (i = 0; i < EMBER_AF_PLUGIN_PRICE_SERVER_TARIFF_TABLE_SIZE; i++) {
    EmberAfScheduledPriceMatrix *lookup = &pmTable[ep][i];
    if (lookup->status != 0 
        && lookup->issuerTariffId == issuerTariffId) {
      MEMCOPY(pm, lookup, sizeof(EmberAfScheduledPriceMatrix));
      return TRUE;
    }
  }

  return FALSE;
}

// Query the tariff table for tariffs matching the selection requirements dictated
// by the GetTariffInformation command. Returns the number of matching tariffs found.
static int8u findTariffs(int8u endpoint,
                         int32u startTime,
                         int32u minIssuerId,
                         int8u numTariffs,
                         int8u tariffType,
                         EmberAfScheduledTariff *tariffs)
{
  int8u i, tariffsFound = 0;
  boolean found;
  EmberAfScheduledTariff tariff;

  emberAfDebugPrintln("findTariffs: selection criteria");
  emberAfDebugPrintln("Start time: %4x", startTime);
  emberAfDebugPrintln("Minimum issuer ID: %4x", minIssuerId);
  emberAfDebugPrintln("Number of tariffs requested: %d", numTariffs);
  emberAfDebugPrintln("Tariff type: %x", tariffType);

  for (i = 0; i < EMBER_AF_PLUGIN_PRICE_SERVER_TARIFF_TABLE_SIZE; i++) {
    if (numTariffs != 0 && tariffsFound >= numTariffs) {
      emberAfDebugPrintln("findTariffs: found %d tariffs", tariffsFound);
      break;
    }

    found = emberAfPriceGetTariffTableEntry(endpoint, i, &tariff);
    if (found) {
      emberAfDebugPrintln(" -- Tariff table entry: endpoint %x index %d -- ", endpoint, i);
      emberAfDebugPrintln("Start time: %4x", tariff.startTime);
      emberAfDebugPrintln("Minimum issuer ID: %4x", tariff.issuerEventId);
      emberAfDebugPrintln("Tariff type: %x", tariff.tariffTypeChargingScheme);
      emberAfDebugPrintln("Tariff status: %x", tariff.status);
      if (tariff.startTime >= startTime
          && (tariff.issuerEventId >= minIssuerId
              || minIssuerId == 0xFFFFFFFF)
          && (tariff.tariffTypeChargingScheme & 0x0F) == (tariffType & 0x0F)
          && (tariffIsCurrent(&tariff) || tariffIsFuture(&tariff))) {
        emberAfDebugPrintln("findTariffs: found matching tariff at index %d", i);
        MEMCOPY(&tariffs[tariffsFound], &tariff, sizeof(EmberAfScheduledTariff));
        tariffsFound++;
        emberAfDebugPrintln("findTariffs: %d tariffs found so far", tariffsFound);
      }
    }
  }

  return tariffsFound;
}

// Sets the tariff at the index. Returns FALSE if the index is invalid.
boolean emberAfPriceSetTariffTableEntry(int8u endpoint,
                                        int8u index,
                                        const EmberAfScheduledTariff *tariff)
{
  int8u ep = emberAfFindClusterServerEndpointIndex(endpoint, ZCL_PRICE_CLUSTER_ID);

  if (ep == 0xFF) {
    return FALSE;
  }
  
  if (index < EMBER_AF_PLUGIN_PRICE_SERVER_TARIFF_TABLE_SIZE) {
    if (tariff == NULL) {
      tariffTable[ep][index].status = 0;
      return TRUE;
    }

    MEMCOPY(&tariffTable[ep][index], tariff, sizeof(EmberAfScheduledTariff));

    if (tariffTable[ep][index].status == 0) {
      tariffTable[ep][index].status |= FUTURE;
    }

    return TRUE;
  }

  return FALSE;
}

// Sets the price matrix at the index. Returns FALSE if the index is invalid.
boolean emberAfPriceSetPriceMatrixTableEntry(int8u endpoint,
                                             int8u index,
                                             const EmberAfScheduledPriceMatrix *pm)
{
  int8u ep = emberAfFindClusterServerEndpointIndex(endpoint, ZCL_PRICE_CLUSTER_ID);

  if (ep == 0xFF) {
    return FALSE;
  }
  
  if (index < EMBER_AF_PLUGIN_PRICE_SERVER_TARIFF_TABLE_SIZE) {
    if (pm == NULL) {
      pmTable[ep][index].status = 0;
      return TRUE;
    }

    MEMCOPY(&pmTable[ep][index], pm, sizeof(EmberAfScheduledPriceMatrix));

    if (pmTable[ep][index].status == 0) {
      pmTable[ep][index].status |= FUTURE;
    }

    return TRUE;
  }

  return FALSE;
}

void emberAfPricePrintTariff(const EmberAfScheduledTariff *tariff)
{
  emberAfPriceClusterPrint("  label: ");
  emberAfPriceClusterPrintString(tariff->tariffLabel);
  emberAfPriceClusterPrint(" (%x)\r\n uom/cur: %x/%2x"
                           "\r\n pid/eid/etid: %4x/%4x/%4x"
                           "\r\n st/tt: %4x/%x",
                           emberAfStringLength(tariff->tariffLabel),
                           tariff->unitOfMeasure,
                           tariff->currency,
                           tariff->providerId,
                           tariff->issuerEventId,
                           tariff->issuerTariffId,
                           tariff->startTime,
                           tariff->tariffTypeChargingScheme);
  emberAfPriceClusterFlush();
  emberAfPriceClusterPrint("\r\n ptu/btu: %x/%x"
                           "\r\n ptd/sc/tbm: %x/%4x/%x"
                           "\r\n btm/btd: %3x/%3x",
                           tariff->numberOfPriceTiersInUse,
                           tariff->numberOfBlockThresholdsInUse,
                           tariff->priceTrailingDigit,
                           tariff->standingCharge,
                           tariff->tierBlockMode,
                           tariff->blockThresholdMultiplier,
                           tariff->blockThresholdDivisor);
  emberAfPriceClusterFlush();
  emberAfPriceClusterPrint("  tariff is ");
  if (tariffIsCurrent(tariff)) {
    emberAfPriceClusterPrintln("current");
  } else if (tariffIsFuture(tariff)) {
    emberAfPriceClusterPrintln("future");
  } else {
    emberAfPriceClusterPrintln("invalid");
  }
  emberAfPriceClusterFlush();
}
    
void emberAfPricePrintPriceMatrix(int8u endpoint,
                                  const EmberAfScheduledPriceMatrix *pm)
{
  EmberAfScheduledTariff t;
  boolean found;
  int8u i, j, chargingScheme;

  found = emberAfPriceGetTariffByIssuerTariffId(endpoint,
                                                pm->issuerTariffId,
                                                &t);

  if (!found) {
    emberAfPriceClusterPrint("  No corresponding valid tariff found; price matrix cannot be printed.\n");
    emberAfPriceClusterPrint("  (NOTE: If printing from command line, be sure the tariff has been pushed to the tariff table.)\n");
    return;
  }

  chargingScheme = t.tariffTypeChargingScheme;

  emberAfPriceClusterPrint("  provider id: %4x\r\n", pm->providerId);
  emberAfPriceClusterPrint("  issuer event id: %4x\r\n", pm->issuerEventId);
  emberAfPriceClusterPrint("  issuer tariff id: %4x\r\n", pm->issuerTariffId);
  emberAfPriceClusterPrint("  start time: %4x\r\n", pm->startTime);

  emberAfPriceClusterFlush();

  emberAfPriceClusterPrint("  == matrix contents == \r\n");
  switch (chargingScheme >> 4) {
    case 0: // TOU only
      for (i = 0; i < t.numberOfPriceTiersInUse; i++) {
        emberAfPriceClusterPrint("  tier %d: %4x\r\n", i, pm->matrix.tier[i]);
      }
      break;
    case 1: // Block only
      for (i = 0; i < t.numberOfPriceTiersInUse; i++) {
        emberAfPriceClusterPrint("  tier %d (block 1): %4x\r\n", i, pm->matrix.blockAndTier[i][0]);
      }
      break;
    case 2: // TOU and Block
    case 3:
      for (i = 0; i < t.numberOfPriceTiersInUse; i++) {
        for (j = 0; j < t.numberOfBlockThresholdsInUse + 1; j++) {
          emberAfPriceClusterPrint("  tier %d block %d: %4x\r\n", i, j + 1, pm->matrix.blockAndTier[i][j]);
        }
      }
      break;
    default:
      emberAfPriceClusterPrint("  Invalid pricing scheme; no contents. \r\n");
      break;
  }

  emberAfPriceClusterPrint("  == end matrix contents == \r\n");
  emberAfPriceClusterFlush();
}

void emberAfPricePrintTariffTable(int8u endpoint) 
{
#if defined(EMBER_AF_PRINT_ENABLE) && defined(EMBER_AF_PRINT_PRICE_CLUSTER)
  int8u i;
  int8u ep = emberAfFindClusterServerEndpointIndex(endpoint, ZCL_PRICE_CLUSTER_ID);
  
  if (ep == 0xFF) {
    return;
  }

  emberAfPriceClusterFlush();
  emberAfPriceClusterPrintln("Tariff Table Contents: ");
  emberAfPriceClusterFlush();
  emberAfPriceClusterPrintln("  Note: ALL values given in HEX\r\n");
  emberAfPriceClusterFlush();
  for (i = 0; i < EMBER_AF_PLUGIN_PRICE_SERVER_TARIFF_TABLE_SIZE; i++) {
    emberAfPriceClusterPrintln("=TARIFF %x =",
                               i);
    emberAfPricePrintTariff(&tariffTable[ep][i]);
  }
#endif // defined(EMBER_AF_PRINT_ENABLE) && defined(EMBER_AF_PRINT_PRICE_CLUSTER)
}

void emberAfPricePrintPriceMatrixTable(int8u endpoint) 
{
#if defined(EMBER_AF_PRINT_ENABLE) && defined(EMBER_AF_PRINT_PRICE_CLUSTER)
  int8u i;
  int8u ep = emberAfFindClusterServerEndpointIndex(endpoint, ZCL_PRICE_CLUSTER_ID);
  
  if (ep == 0xFF) {
    return;
  }

  emberAfPriceClusterFlush();
  emberAfPriceClusterPrintln("Price Matrix Table Contents: ");
  emberAfPriceClusterFlush();
  emberAfPriceClusterPrintln("  Note: ALL values given in HEX (except indices)\r\n");
  emberAfPriceClusterFlush();
  for (i = 0; i < EMBER_AF_PLUGIN_PRICE_SERVER_TARIFF_TABLE_SIZE; i++) {
    emberAfPriceClusterPrintln("=PRICE MATRIX %x =",
                               i);
    emberAfPricePrintPriceMatrix(endpoint, &pmTable[ep][i]);
  }

#endif // defined(EMBER_AF_PRINT_ENABLE) && defined(EMBER_AF_PRINT_PRICE_CLUSTER)
}



boolean emberAfPriceClusterGetTariffInformationCallback(int32u earliestStartTime,
                                                        int32u minIssuerEventId,
                                                        int8u numberOfCommands,
                                                        int8u tariffType) 
{
  EmberAfScheduledTariff tariffs[EMBER_AF_PLUGIN_PRICE_SERVER_TARIFF_TABLE_SIZE];
  EmberAfScheduledTariff tariff;
  int8u endpoint = emberAfCurrentEndpoint(), tariffsFound, i;

  emberAfDebugPrintln("GetTariffInformation: about to query for tariffs");

  tariffsFound = findTariffs(endpoint,
                             earliestStartTime,
                             minIssuerEventId,
                             numberOfCommands,
                             tariffType,
                             (EmberAfScheduledTariff *) tariffs);

  emberAfDebugPrintln("Tariffs found: %d", tariffsFound);
  if (tariffsFound == 0) {
    emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_NOT_FOUND);
    return TRUE;
  }

  for (i = 0; i < tariffsFound; i++) {
    emberAfDebugPrintln("GetTariffInformation: publishing matching tariff %d of %d", i + 1, tariffsFound);
    tariff = tariffs[i];
    emberAfFillCommandPriceClusterPublishTariffInformation(tariff.providerId,
                                                           tariff.issuerEventId,
                                                           tariff.issuerTariffId,
                                                           tariff.startTime,
                                                           tariff.tariffTypeChargingScheme,
                                                           tariff.tariffLabel,
                                                           tariff.numberOfPriceTiersInUse,
                                                           tariff.numberOfBlockThresholdsInUse,
                                                           tariff.unitOfMeasure,
                                                           tariff.currency,
                                                           tariff.priceTrailingDigit,
                                                           tariff.standingCharge,
                                                           tariff.tierBlockMode,
                                                           tariff.blockThresholdMultiplier,
                                                           tariff.blockThresholdDivisor);
    emberAfSendResponse();
  }

  return TRUE;
}

boolean emberAfPriceClusterGetPriceMatrixCallback(int32u issuerTariffId)
{
  EmberAfScheduledTariff tariff;
  EmberAfScheduledPriceMatrix pm;
  boolean found;
  int8u endpoint = emberAfCurrentEndpoint(), i, j, payloadControl;
  int16u size = 0;
  // Allocate for the largest possible size, unfortunately
  int8u subPayload[ZCL_PRICE_CLUSTER_MAX_TOU_BLOCKS 
                   * ZCL_PRICE_CLUSTER_MAX_TOU_BLOCK_TIERS
                   * ZCL_PRICE_CLUSTER_PRICE_MATRIX_SUB_PAYLOAD_ENTRY_SIZE];

  // A price matrix must have an associated tariff, otherwise it is meaningless
  found = emberAfPriceGetTariffByIssuerTariffId(endpoint,
                                                issuerTariffId,
                                                &tariff);

  if (!found) {
    emberAfDebugPrintln("GetPriceMatrix: no corresponding tariff for id 0x%4x found", 
                        issuerTariffId);
    emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_NOT_FOUND);
    return TRUE;
  }

  // Grab the actual price matrix
  found = emberAfPriceGetPriceMatrixByIssuerTariffId(endpoint,
                                                     issuerTariffId,
                                                     &pm);

  if (!found) {
    emberAfDebugPrintln("GetPriceMatrix: no corresponding price matrix for id 0x%4x found", 
                        issuerTariffId);
    emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_NOT_FOUND);
    return TRUE;
  }

  // The structure of the price matrix will vary depending on the type of the tariff
  switch (tariff.tariffTypeChargingScheme >> 4) {
    case 0: // TOU only
      payloadControl = 1;
      for (i = 0; i < tariff.numberOfPriceTiersInUse; i++) {
        subPayload[i * ZCL_PRICE_CLUSTER_PRICE_MATRIX_SUB_PAYLOAD_ENTRY_SIZE] = i;
        emberAfCopyInt32u(subPayload,
                          i * ZCL_PRICE_CLUSTER_PRICE_MATRIX_SUB_PAYLOAD_ENTRY_SIZE + 1,
                          pm.matrix.tier[i]);
        size += ZCL_PRICE_CLUSTER_PRICE_MATRIX_SUB_PAYLOAD_ENTRY_SIZE;
      }
      break;
    case 1: // Block only
      payloadControl = 0;
      for (i = 0; i < tariff.numberOfPriceTiersInUse; i++) {
        subPayload[i * ZCL_PRICE_CLUSTER_PRICE_MATRIX_SUB_PAYLOAD_ENTRY_SIZE] = i << 4;
        emberAfCopyInt32u(subPayload,
                          i * ZCL_PRICE_CLUSTER_PRICE_MATRIX_SUB_PAYLOAD_ENTRY_SIZE + 1,
                          pm.matrix.blockAndTier[i][0]);
        size += ZCL_PRICE_CLUSTER_PRICE_MATRIX_SUB_PAYLOAD_ENTRY_SIZE;
      }
      break;
    case 2:
    case 3: // TOU / Block combined
      payloadControl = 0;
      for (i = 0; i < tariff.numberOfPriceTiersInUse; i++) {
        for (j = 0; j < tariff.numberOfBlockThresholdsInUse + 1; j++) { 
          subPayload[size] = (i << 4) | j;
          emberAfCopyInt32u(subPayload,
                            size + 1,
                            pm.matrix.blockAndTier[i][j]);
          size += ZCL_PRICE_CLUSTER_PRICE_MATRIX_SUB_PAYLOAD_ENTRY_SIZE;
        }
      }
      break;
    default:
      emberAfDebugPrintln("GetPriceMatrix: invalid tariff type / charging scheme");
      emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_INVALID_VALUE);
      return TRUE;
  }

  // Populate and send the PublishPriceMatrix command
  emberAfFillCommandPriceClusterPublishPriceMatrix(pm.providerId,
                                                   pm.issuerEventId,
                                                   pm.startTime,
                                                   pm.issuerTariffId,
                                                   0,
                                                   1,
                                                   payloadControl,
                                                   subPayload,
                                                   size);
  emberAfSendResponse();

  return TRUE;
}

#else // !defined(EMBER_AF_PLUGIN_PRICE_SERVER_TARIFF_MATRIX_SUPPORT)

void emberAfPriceClearTariffTable(int8u endpoint)
{
}

void emberAfPriceClearPriceMatrixTable(int8u endpoint)
{
}

boolean emberAfPriceSetTariffTableEntry(int8u endpoint,
                                        int8u index,
                                        const EmberAfScheduledTariff *tariff)
{
  return FALSE;
}

boolean emberAfPriceClusterGetTariffInformationCallback(int32u earliestStartTime,
                                                        int32u minIssuerEventId,
                                                        int8u numberOfCommands,
                                                        int8u tariffType) 
{
  return FALSE;
}

boolean emberAfPriceClusterGetPriceMatrixCallback(int32u issuerTariffId)
{
  return FALSE;
}

#endif // EMBER_AF_PLUGIN_PRICE_SERVER_TARIFF_MATRIX_SUPPORT