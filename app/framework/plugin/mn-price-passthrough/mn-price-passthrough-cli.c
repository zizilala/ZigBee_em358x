// *******************************************************************
// * mn-price-passthrough-cli.c
// *
// *
// * Copyright 2012 by Ember Corporation. All rights reserved.              *80*
// *******************************************************************

#include "app/framework/include/af.h"
#include "app/util/serial/command-interpreter2.h"
#include "app/framework/plugin/mn-price-passthrough/mn-price-passthrough.h"

void start(void);
void stop(void);
void setRouting(void);
void print(void);

#ifndef EMBER_AF_GENERATE_CLI
EmberCommandEntry emberAfPluginMnPricePassthroughCommands[] = {
  emberCommandEntryAction("start", start, "", ""),
  emberCommandEntryAction("stop", stop, "", ""),
  emberCommandEntryAction("set-routing", setRouting, "vuu", ""),
  emberCommandEntryAction("print", print, "", ""),
  emberCommandEntryTerminator(),
};
#endif // EMBER_AF_GENERATE_CLI

// plugin mn-price-passthrough start
void start(void)
{
  emAfPluginMnPricePassthroughStartPollAndForward();
}

// plugin mn-price-passthrough start
void stop(void)
{
  emAfPluginMnPricePassthroughStopPollAndForward();
}

// plugin mn-price-passthrough setRouting <forwardingId:2> <forwardingEndpoint:1> <proxyEsiEndpoint:1>
void setRouting(void)
{
  EmberNodeId fwdId = (EmberNodeId)emberUnsignedCommandArgument(0);
  int8u fwdEndpoint = (int8u)emberUnsignedCommandArgument(1);
  int8u esiEndpoint = (int8u)emberUnsignedCommandArgument(2);
  emAfPluginMnPricePassthroughRoutingSetup(fwdId,
                                           fwdEndpoint,
                                           esiEndpoint);
}

// plugin mn-price-passthrough print
void print(void)
{
  emAfPluginMnPricePassthroughPrintCurrentPrice();
}
