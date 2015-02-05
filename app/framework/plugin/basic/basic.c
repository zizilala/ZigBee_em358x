// *******************************************************************
// * basic.c
// *
// *
// * Copyright 2007 by Ember Corporation. All rights reserved.              *80*
// *******************************************************************

#include "../../include/af.h"

void emberAfResetAttributes(int8u endpoint);

boolean emberAfBasicClusterResetToFactoryDefaultsCallback(void)
{
  emberAfBasicClusterPrintln("RX: ResetToFactoryDefaultsCallback");
  emberAfResetAttributes(emberAfCurrentEndpoint());
  emberAfPluginBasicResetToFactoryDefaultsCallback(emberAfCurrentEndpoint());
  emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_SUCCESS);
  return TRUE;
}
