// *******************************************************************
// * device-maangement-server-cli.c
// *
// *
// * Copyright 2014 by Silicon Laboratories. All rights reserved.           *80*
// *******************************************************************

#include "app/framework/include/af.h"
#include "device-management-server.h"

#ifndef EMBER_AF_GENERATE_CLI
  #error The Device Management Server plugin is not compatible with the legacy CLI.
#endif

void emAfDeviceManagementServerCliPrint(void);
void emAfDeviceManagementServerCliTenancy(void);
void emAfDeviceManagementServerCliSupplier(void);
void emAfDeviceManagementServerCliSupply(void);
void emAfDeviceManagementServerCliSiteId(void);
void emAfDeviceManagementServerCliSupplyStatus(void);
void emAfDeviceManagementServerCliPassword(void);
void emAfDeviceManagementServerCliUncontrolledFlowThreshold(void);

void emAfDeviceManagementServerCliPrint(void)
{
  emberAfDeviceManagementServerPrint();
}

void emAfDeviceManagementServerCliTenancy(void)
{
  EmberAfDeviceManagementTenancy tenancy;
  tenancy.implementationDateTime = (int32u) emberUnsignedCommandArgument(0);
  tenancy.tenancy = (int32u) emberUnsignedCommandArgument(1);

  emberAfPluginDeviceManagementSetTenancy(&tenancy);
}

void emAfDeviceManagementServerCliSupplier(void)
{
  int8u length;
  EmberAfDeviceManagementSupplier supplier;

  length = emberCopyStringArgument(0,
                                   supplier.providerName + 1,
                                   EMBER_AF_DEVICE_MANAGEMENT_MAXIMUM_PROVIDER_NAME_LENGTH,
                                   FALSE);
  supplier.providerName[0] = length;
  supplier.proposedProviderId = (int32u) emberUnsignedCommandArgument(1); 
  supplier.implementationDateTime = (int32u) emberUnsignedCommandArgument(2);
  supplier.providerChangeControl = (int32u) emberUnsignedCommandArgument(3); 

  emberAfPluginDeviceManagementSetSupplier(&supplier);
}

void emAfDeviceManagementServerCliSupply(void)
{
  EmberAfDeviceManagementSupply supply;
  supply.requestDateTime = (int32u) emberUnsignedCommandArgument(0);
  supply.implementationDateTime = (int32u) emberUnsignedCommandArgument(1);
  supply.supplyStatus = (int8u) emberUnsignedCommandArgument(2);
  supply.originatorIdSupplyControlBits = (int8u) emberUnsignedCommandArgument(3);
  
  emberAfPluginDeviceManagementSetSupply(&supply);
}

void emAfDeviceManagementServerCliSiteId(void)
{
  int8u length;
  EmberAfDeviceManagementSiteId siteId;

  length = emberCopyStringArgument(0,
                                   siteId.siteId + 1,
                                   EMBER_AF_DEVICE_MANAGEMENT_MAXIMUM_SITE_ID_LENGTH,
                                   FALSE);
  siteId.siteId[0] = length;
  siteId.siteIdTime = (int32u) emberUnsignedCommandArgument(1);

  emberAfPluginDeviceManagementSetSiteId(&siteId);
}

void emAfDeviceManagementServerCliSupplyStatus(void)
{
  EmberAfDeviceManagementSupplyStatus supplyStatus;
  supplyStatus.implementationDateTime = (int32u) emberUnsignedCommandArgument(0);
  supplyStatus.supplyStatus = (int8u) emberUnsignedCommandArgument(1);

  emberAfPluginDeviceManagementSetSupplyStatus(&supplyStatus);
}

void emAfDeviceManagementServerCliPassword(void)
{
  int8u length;
  EmberAfDeviceManagementPassword password;

  length = emberCopyStringArgument(0,
                                   password.password + 1,
                                   EMBER_AF_DEVICE_MANAGEMENT_MAXIMUM_PASSWORD_LENGTH,
                                   FALSE);
  password.password[0] = length;
  password.implementationDateTime = (int32u) emberUnsignedCommandArgument(1);
  password.durationInMinutes = (int16u) emberUnsignedCommandArgument(2);
  password.passwordType = (int8u) emberUnsignedCommandArgument(3);

  emberAfPluginDeviceManagementSetPassword(&password);
}

void emAfDeviceManagementServerCliUncontrolledFlowThreshold(void)
{
  EmberAfDeviceManagementUncontrolledFlowThreshold threshold;
  threshold.uncontrolledFlowThreshold = (int16u) emberUnsignedCommandArgument(0);
  threshold.multiplier = (int16u) emberUnsignedCommandArgument(1);
  threshold.divisor = (int16u) emberUnsignedCommandArgument(2);
  threshold.measurementPeriod = (int16u) emberUnsignedCommandArgument(3);
  threshold.unitOfMeasure = (int8u) emberUnsignedCommandArgument(4);
  threshold.stabilisationPeriod = (int8u) emberUnsignedCommandArgument(5);

  emberAfPluginDeviceManagementSetUncontrolledFlowThreshold(&threshold);
}
