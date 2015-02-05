// File: gateway-support-cli.c
//
// Description:  Gateway specific CLI behavior for a host application.
//   In this case we assume our application is running on
//   a PC with Unix library support, connected to an NCP via serial uart.
//
// Author(s): Rob Alexander <rob.alexander@silabs.com>
//
// Copyright 2013 by Ember Corporation.  All rights reserved.               *80*
//
//------------------------------------------------------------------------------


extern EmberCommandEntry emberAfPluginGatewayCommands[];

#if defined(GATEWAY_APP)
  #define EMBER_AF_PLUGIN_GATEWAY_COMMANDS \
    emberCommandEntrySubMenu("gateway", \
                             emberAfPluginGatewayCommands, \
                             "Commands for the Linux host application"),

#else
  #define EMBER_AF_PLUGIN_GATEWAY_COMMANDS

#endif