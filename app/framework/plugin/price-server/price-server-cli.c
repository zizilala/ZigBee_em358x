// *******************************************************************
// * price-server-cli.c
// *
// *
// * Copyright 2012 by Ember Corporation. All rights reserved.              *80*
// *******************************************************************

#include "app/framework/include/af.h"
#include "app/util/serial/command-interpreter2.h"
#include "app/framework/plugin/price-server/price-server.h"

//=============================================================================
// Functions

void emAfPriceServerCliClear(void);
void emAfPriceServerCliTClear(void);
void emAfPriceServerCliWho(void);
void emAfPriceServerCliTWho(void);
void emAfPriceServerCliWhat(void);
void emAfPriceServerCliTWhat(void);
void emAfPriceServerCliWhen(void);
void emAfPriceServerCliTWhen(void);
void emAfPriceServerCliPrice(void);
void emAfPriceServerCliTariff(void);
void emAfPriceServerCliAlternate(void);
void emAfPriceServerCliAck(void);
void emAfPriceServerCliValid(void);
void emAfPriceServerCliTStatus(void);
void emAfPriceServerCliGet(void);
void emAfPriceServerCliTGet(void);
void emAfPriceServerCliPrint(void);
void emAfPriceServerCliTPrint(void);
void emAfPriceServerCliSprint(void);
void emAfPriceServerCliTSPrint(void);
void emAfPriceServerCliPublish(void);
void emAfPriceServerCliPmPrint(void);
void emAfPriceServerCliPmSetMetadata(void);
void emAfPriceServerCliPmSetProvider(void);
void emAfPriceServerCliPmSetEvent(void);
void emAfPriceServerCliPmSetTariff(void);
void emAfPriceServerCliPmSetTime(void);
void emAfPriceServerCliPmSetStatus(void);
void emAfPriceServerCliPmGet(void);
void emAfPriceServerCliPmSet(void);
void emAfPriceServerCliPmPut(void);
void emAfPriceServerCliPmFillTier(void);
void emAfPriceServerCliPmFillBlock(void);

//=============================================================================
// Globals

static EmberAfScheduledPrice price;


#if !defined(EMBER_AF_GENERATE_CLI)
EmberCommandEntry emberAfPluginPriceServerCommands[] = {
  emberCommandEntryAction("clear",  emAfPriceServerCliClear, "u", ""),
  emberCommandEntryAction("who",  emAfPriceServerCliWho, "wbw", ""),
  emberCommandEntryAction("what",  emAfPriceServerCliWhat, "uvuuu", ""),
  emberCommandEntryAction("when",  emAfPriceServerCliWhen, "wv", ""),
  emberCommandEntryAction("price",  emAfPriceServerCliPrice, "wuwu", ""),
  emberCommandEntryAction("alternate",  emAfPriceServerCliAlternate, "wuu", ""),
  emberCommandEntryAction("ack",  emAfPriceServerCliAck, "u", ""),
  emberCommandEntryAction("valid",  emAfPriceServerCliValid, "uu", ""),
  emberCommandEntryAction("get",  emAfPriceServerCliGet, "uu", ""),
  emberCommandEntryAction("print",  emAfPriceServerCliPrint, "u", ""),
  emberCommandEntryAction("sprint",  emAfPriceServerCliSprint, "u", ""),
  emberCommandEntryAction("publish", emAfPriceServerCliPublish, "vuuu", ""),
  emberCommandEntryTerminator(),
};
#endif // EMBER_AF_GENERATE_CLI

//=============================================================================

// plugin price-server clear <endpoint:1>
void emAfPriceServerCliClear(void)
{
  emberAfPriceClearPriceTable(emberUnsignedCommandArgument(0));
}

// plugin price-server who <provId:4> <label:1-13> <eventId:4>
void emAfPriceServerCliWho(void)
{
  int8u length;
  price.providerId = emberUnsignedCommandArgument(0);
  length = emberCopyStringArgument(1,
                                   price.rateLabel + 1,
                                   ZCL_PRICE_CLUSTER_MAXIMUM_RATE_LABEL_LENGTH,
                                   FALSE);
  price.rateLabel[0] = length;
  price.issuerEventID = emberUnsignedCommandArgument(2);
}


// plugin price-server what <unitOfMeas:1> <curr:2> <ptd:1> <PTRT:1> <bt:1>
void emAfPriceServerCliWhat(void)
{
  price.unitOfMeasure = (int8u)emberUnsignedCommandArgument(0);
  price.currency = (int16u)emberUnsignedCommandArgument(1);
  price.priceTrailingDigitAndTier = (int8u)emberUnsignedCommandArgument(2);
  price.numberOfPriceTiersAndTier = (int8u)emberUnsignedCommandArgument(3);
  price.numberOfBlockThresholds = (int8u)emberUnsignedCommandArgument(4);
}


// plugin price-server when <startTime:4> <duration:2>
void emAfPriceServerCliWhen(void)
{
  price.startTime = emberUnsignedCommandArgument(0);
  price.duration = (int16u)emberUnsignedCommandArgument(1);
}

// plugin price-server price <price:4> <ratio:1> <genPrice:4> <genRatio:1>
void emAfPriceServerCliPrice(void)
{
  price.price = emberUnsignedCommandArgument(0);
  price.priceRatio = (int8u)emberUnsignedCommandArgument(1);
  price.generationPrice = emberUnsignedCommandArgument(2);
  price.generationPriceRatio = (int8u)emberUnsignedCommandArgument(3);
}


// plugin price-server alternate <alternateCostDelivered:4> <alternateCostUnit:1> <alternateCostTrailingDigit:1>
void emAfPriceServerCliAlternate(void)
{
  price.alternateCostDelivered = emberUnsignedCommandArgument(0);
  price.alternateCostUnit = (int8u)emberUnsignedCommandArgument(1);
  price.alternateCostTrailingDigit = (int8u)emberUnsignedCommandArgument(2);
}

// plugin price-server ack <req:1>
void emAfPriceServerCliAck(void)
{
  price.priceControl &= ~ZCL_PRICE_CLUSTER_PRICE_ACKNOWLEDGEMENT_MASK;
  if (emberUnsignedCommandArgument(0) == 1) {
    price.priceControl |= EMBER_ZCL_PRICE_CONTROL_ACKNOWLEDGEMENT_REQUIRED;
  }
}

// plugin price-server <valid | invalid | tinvalid> <endpoint:1> <index:1>
void emAfPriceServerCliValid(void)
{
  int8u endpoint = (int8u)emberUnsignedCommandArgument(0);
  int8u index = (int8u)emberUnsignedCommandArgument(1);
  if (emberCurrentCommand->name[0] == 't') {
    if (!emberAfPriceSetTariffTableEntry(endpoint,
                                         index,
                                         NULL)) {
      emberAfPriceClusterPrintln("tariff entry %d not present", index);
    }

    return;
  }

  if (!emberAfPriceSetPriceTableEntry(endpoint,
                                      index,
                                      (emberCurrentCommand->name[0] == 'v'
                                       ? &price
                                       : NULL))) {
    emberAfPriceClusterPrintln("price entry %d not present", index);
  }
}

// plugin price-server get <endpoint:1> <index:1>
void emAfPriceServerCliGet(void)
{
  int8u endpoint = (int8u)emberUnsignedCommandArgument(0);
  int8u index = (int8u)emberUnsignedCommandArgument(1);
  if (!emberAfPriceGetPriceTableEntry(endpoint, index, &price)) {
    emberAfPriceClusterPrintln("price entry %d not present", index);
  }
}

// plugin price-server print <endpoint:1>
void emAfPriceServerCliPrint(void) 
{
  int8u endpoint = (int8u)emberUnsignedCommandArgument(0);
  emberAfPricePrintTable(endpoint);
}

// plugin price-server sprint
void emAfPriceServerCliSprint(void)
{
  emberAfPricePrint(&price);
}

// plugin price-server publish <nodeId:2> <srcEndpoint:1> <dstEndpoint:1> <priceIndex:1>
void emAfPriceServerCliPublish(void)
{
  emberAfPluginPriceServerPublishPriceMessage((EmberNodeId)emberUnsignedCommandArgument(0),
                                              (int8u)emberUnsignedCommandArgument(1),
                                              (int8u)emberUnsignedCommandArgument(2),
                                              (int8u)emberUnsignedCommandArgument(3));
}

