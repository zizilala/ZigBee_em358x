// *******************************************************************
// * device-management-server.c
// *
// *
// * Copyright 2014 by Silicon Laboratories. All rights reserved.           *80*
// *******************************************************************

#include "app/framework/include/af.h"
#include "app/framework/util/common.h"
#include "device-management-server.h"

static EmberAfDeviceManagementInfo info;

boolean emberAfPluginDeviceManagementSetTenancy(EmberAfDeviceManagementTenancy *tenancy)
{
  if (tenancy == NULL) {
    MEMSET(&(info.tenancy), 0, sizeof(EmberAfDeviceManagementTenancy));
  } else {
    MEMCOPY(&(info.tenancy), tenancy, sizeof(EmberAfDeviceManagementTenancy));
  }

  return TRUE;
}

boolean emberAfPluginDeviceManagementGetTenancy(EmberAfDeviceManagementTenancy *tenancy)
{
  if (tenancy == NULL) {
    return FALSE;
  }

  MEMCOPY(tenancy, &(info.tenancy), sizeof(EmberAfDeviceManagementTenancy));

  return TRUE;
}

boolean emberAfPluginDeviceManagementSetSupplier(EmberAfDeviceManagementSupplier *supplier)
{
  if (supplier == NULL) {
    MEMSET(&(info.supplier), 0, sizeof(EmberAfDeviceManagementSupplier));
  } else {
    MEMCOPY(&(info.supplier), supplier, sizeof(EmberAfDeviceManagementSupplier));
  }

  return TRUE;
}

boolean emberAfPluginDeviceManagementGetSupplier(EmberAfDeviceManagementSupplier *supplier)
{
  if (supplier == NULL) {
    return FALSE;
  }

  MEMCOPY(supplier, &(info.supplier), sizeof(EmberAfDeviceManagementSupplier));

  return TRUE;
}

boolean emberAfPluginDeviceManagementSetSupply(EmberAfDeviceManagementSupply *supply)
{
  if (supply == NULL) {
    MEMSET(&(info.supply), 0, sizeof(EmberAfDeviceManagementSupply));
  } else {
    MEMCOPY(&(info.supply), supply, sizeof(EmberAfDeviceManagementSupply));
  }

  return TRUE;
}

boolean emberAfPluginDeviceManagementGetSupply(EmberAfDeviceManagementSupply *supply)
{
  if (supply == NULL) {
    return FALSE;
  }

  MEMCOPY(supply, &(info.supply), sizeof(EmberAfDeviceManagementSupply));

  return TRUE;
}

boolean emberAfPluginDeviceManagementSetInfoGlobalData(int32u providerId,
                                                       int32u issuerEventId,
                                                       int8u tariffType)
{
  info.providerId = providerId;
  info.issuerEventId = issuerEventId;
  info.tariffType = tariffType;

  return TRUE;
}

boolean emberAfPluginDeviceManagementSetSupplyStatus(EmberAfDeviceManagementSupplyStatus *status)
{
  if (status == NULL) {
    MEMSET(&(info.supplyStatus), 0, sizeof(EmberAfDeviceManagementSupplyStatus));
  } else {
    MEMCOPY(&(info.supplyStatus), status, sizeof(EmberAfDeviceManagementSupplyStatus));
  }

  return TRUE;
}

boolean emberAfPluginDeviceManagementGetSupplyStatus(EmberAfDeviceManagementSupplyStatus *status)
{
  if (status == NULL) {
    return FALSE;
  }

  MEMCOPY(status, &(info.supplyStatus), sizeof(EmberAfDeviceManagementSupplyStatus));

  return TRUE;
}

boolean emberAfPluginDeviceManagementSetSiteId(EmberAfDeviceManagementSiteId *siteId)
{
  if (siteId == NULL) {
    MEMSET(&(info.siteId), 0, sizeof(EmberAfDeviceManagementSiteId));
  } else {
    MEMCOPY(&(info.siteId), siteId, sizeof(EmberAfDeviceManagementSiteId));
  }

  return TRUE;
}

boolean emberAfPluginDeviceManagementGetSiteId(EmberAfDeviceManagementSiteId *siteId)
{
  if (siteId == NULL) {
    return FALSE;
  }

  MEMCOPY(siteId, &(info.siteId), sizeof(EmberAfDeviceManagementSiteId));

  return TRUE;
}

boolean emberAfPluginDeviceManagementSetPassword(EmberAfDeviceManagementPassword *password)
{
  if (password == NULL) {
    MEMSET(&(info.servicePassword), 0, sizeof(EmberAfDeviceManagementPassword));
    MEMSET(&(info.consumerPassword), 0, sizeof(EmberAfDeviceManagementPassword));
  } else {
    if (password->passwordType == SERVICE_PASSWORD) {
      MEMCOPY(&(info.servicePassword), password, sizeof(EmberAfDeviceManagementPassword));
    } else if (password->passwordType == CONSUMER_PASSWORD) {
      MEMCOPY(&(info.consumerPassword), password, sizeof(EmberAfDeviceManagementPassword));
    } else {
      return FALSE;
    }
  }

  return TRUE;
}

boolean emberAfPluginDeviceManagementGetPassword(EmberAfDeviceManagementPassword *password,
                                                 int8u passwordType)
{
  if (password == NULL) {
    return FALSE;
  }

  switch (passwordType) {
    case SERVICE_PASSWORD:
      MEMCOPY(password, &(info.servicePassword), sizeof(EmberAfDeviceManagementPassword));
      break;
    case CONSUMER_PASSWORD:
      MEMCOPY(password, &(info.consumerPassword), sizeof(EmberAfDeviceManagementPassword));
      break;
    default:
      return FALSE;
  }

  return TRUE;
}

boolean emberAfPluginDeviceManagementSetUncontrolledFlowThreshold(EmberAfDeviceManagementUncontrolledFlowThreshold *threshold)
{
  if (threshold == NULL) {
    MEMSET(&(info.threshold), 0, sizeof(EmberAfDeviceManagementUncontrolledFlowThreshold));
  } else {
    MEMCOPY(&(info.threshold), threshold, sizeof(EmberAfDeviceManagementUncontrolledFlowThreshold));
  }

  return TRUE;
}

boolean emberAfPluginDeviceManagementGetUncontrolledFlowThreshold(EmberAfDeviceManagementUncontrolledFlowThreshold *threshold)
{
  if (threshold == NULL) {
    return FALSE;
  }

  MEMCOPY(threshold, &(info.threshold), sizeof(EmberAfDeviceManagementUncontrolledFlowThreshold));

  return TRUE;
}

void emberAfDeviceManagementServerPrint(void)
{
  emberAfDeviceManagementClusterPrintln("== Device Management Information ==\n");

  emberAfDeviceManagementClusterPrintln("  == Tenancy ==");
  emberAfDeviceManagementClusterPrintln("  Implementation Date / Time: %4x", info.tenancy.implementationDateTime);
  emberAfDeviceManagementClusterPrintln("  Tenancy: %4x\n", info.tenancy.tenancy);

  emberAfDeviceManagementClusterPrintln("  == Supplier ==");
  emberAfDeviceManagementClusterPrint("  Provider name: ");
  emberAfDeviceManagementClusterPrintString(info.supplier.providerName);
  emberAfDeviceManagementClusterPrintln("\n  Proposed Provider Id: %4x", info.supplier.proposedProviderId);
  emberAfDeviceManagementClusterPrintln("  Implementation Date / Time: %4x", info.supplier.implementationDateTime);
  emberAfDeviceManagementClusterPrintln("  Provider Change Control: %4x\n", info.supplier.providerChangeControl);

  emberAfDeviceManagementClusterPrintln("  == Supply ==");
  emberAfDeviceManagementClusterPrintln("  Request Date / Time: %4x", info.supply.requestDateTime);
  emberAfDeviceManagementClusterPrintln("  Implementation Date / Time: %4x", info.supply.implementationDateTime);
  emberAfDeviceManagementClusterPrintln("  Supply Status: %x", info.supply.supplyStatus);
  emberAfDeviceManagementClusterPrintln("  Originator Id / Supply Control Bits: %x\n", info.supply.originatorIdSupplyControlBits);

  emberAfDeviceManagementClusterPrintln("  == Site ID ==");
  emberAfDeviceManagementClusterPrint("  Site Id: ");
  emberAfDeviceManagementClusterPrintString(info.siteId.siteId);
  emberAfDeviceManagementClusterPrintln("\n  Site Id Time: %4x\n", info.siteId.siteIdTime);

  emberAfDeviceManagementClusterPrintln("  == Supply Status ==");
  emberAfDeviceManagementClusterPrintln("  Implementation Date / Time: %4x", info.supplyStatus.implementationDateTime);
  emberAfDeviceManagementClusterPrintln("  Supply Status: %x\n", info.supplyStatus.supplyStatus);

  emberAfDeviceManagementClusterPrintln("  == Passwords ==");

  emberAfDeviceManagementClusterPrintln("   = Service Password =");
  emberAfDeviceManagementClusterPrint("   Password: ");
  emberAfDeviceManagementClusterPrintString(info.servicePassword.password);
  emberAfDeviceManagementClusterPrintln("\n   Implementation Date / Time: %4x", info.servicePassword.implementationDateTime);
  emberAfDeviceManagementClusterPrintln("   Duration In Minutes: %2x", info.servicePassword.durationInMinutes);
  emberAfDeviceManagementClusterPrintln("   Password Type: %x\n", info.servicePassword.passwordType);

  emberAfDeviceManagementClusterPrintln("   = Consumer Password =");
  emberAfDeviceManagementClusterPrint("   Password: ");
  emberAfDeviceManagementClusterPrintString(info.consumerPassword.password);
  emberAfDeviceManagementClusterPrintln("\n   Implementation Date / Time: %4x", info.consumerPassword.implementationDateTime);
  emberAfDeviceManagementClusterPrintln("   Duration In Minutes: %2x", info.consumerPassword.durationInMinutes);
  emberAfDeviceManagementClusterPrintln("   Password Type: %x\n", info.consumerPassword.passwordType);

  emberAfDeviceManagementClusterPrintln("  == Uncontrolled Flow Threshold ==");
  emberAfDeviceManagementClusterPrintln("  Uncontrolled Flow Threshold: %2x", info.threshold.uncontrolledFlowThreshold);
  emberAfDeviceManagementClusterPrintln("  Multiplier: %2x", info.threshold.multiplier);
  emberAfDeviceManagementClusterPrintln("  Divisor: %2x", info.threshold.divisor);
  emberAfDeviceManagementClusterPrintln("  Measurement Period: %2x", info.threshold.measurementPeriod);
  emberAfDeviceManagementClusterPrintln("  Unit of Measure: %x", info.threshold.unitOfMeasure);
  emberAfDeviceManagementClusterPrintln("  Stabilisation Period: %x\n", info.threshold.stabilisationPeriod);

  emberAfDeviceManagementClusterPrintln("== End of Device Management Information ==");
}

boolean emberAfDeviceManagementClusterGetChangeOfTenancyCallback(void)
{
  EmberAfDeviceManagementTenancy *tenancy = &(info.tenancy);

  emberAfFillCommandDeviceManagementClusterPublishChangeOfTenancy(info.providerId,
                                                                  info.issuerEventId,
                                                                  info.tariffType,
                                                                  tenancy->implementationDateTime,
                                                                  tenancy->tenancy);

  emberAfSendResponse();

  return TRUE;
}

boolean emberAfDeviceManagementClusterGetChangeOfSupplierCallback(void)
{
  EmberAfDeviceManagementSupplier *supplier = &(info.supplier);

  emberAfFillCommandDeviceManagementClusterPublishChangeOfSupplier(info.providerId,
                                                                   info.issuerEventId,
                                                                   info.tariffType,
                                                                   supplier->proposedProviderId,
                                                                   supplier->implementationDateTime,
                                                                   supplier->providerChangeControl,
                                                                   supplier->providerName);

  emberAfSendResponse();

  return TRUE;
}

boolean emberAfDeviceManagementClusterGetChangeSupplyCallback(void)
{
  EmberAfDeviceManagementSupply *supply = (&info.supply);

  emberAfFillCommandDeviceManagementClusterChangeSupply(info.providerId,
                                                        info.issuerEventId,
                                                        supply->requestDateTime,
                                                        supply->implementationDateTime,
                                                        supply->supplyStatus,
                                                        supply->originatorIdSupplyControlBits);

  emberAfSendResponse();

  return TRUE;
}

boolean emberAfDeviceManagementClusterSupplyStatusResponseCallback(int32u providerId,
                                                                   int32u issuerEventId,
                                                                   int32u implementationDateTime,
                                                                   int8u supplyStatus)
{
  EmberAfDeviceManagementSupplyStatus *status = &(info.supplyStatus);

  status->implementationDateTime = implementationDateTime;
  status->supplyStatus = supplyStatus;

  emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_SUCCESS);

  return TRUE;
}

boolean emberAfDeviceManagementClusterGetSiteIdCallback(void) 
{
  EmberAfDeviceManagementSiteId *siteId = &(info.siteId);

  emberAfFillCommandDeviceManagementClusterUpdateSiteId(info.issuerEventId,
                                                        siteId->siteIdTime,
                                                        info.providerId,
                                                        siteId->siteId);

  emberAfSendResponse();

  return TRUE;
}

boolean emberAfDeviceManagementClusterRequestNewPasswordCallback(int8u passwordType)
{
  EmberAfDeviceManagementPassword *password;
  switch (passwordType) {
    case SERVICE_PASSWORD:
      password = &(info.servicePassword);
      break;
    case CONSUMER_PASSWORD:
      password = &(info.consumerPassword);
      break;
    default:
      emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_NOT_FOUND);
      return TRUE;
  }
  
  // Is the password still valid?
  if (password->durationInMinutes != 0 
      && (emberAfGetCurrentTime() - (password->durationInMinutes * 60) 
          > password->implementationDateTime)) {
    emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_NOT_FOUND);
    return TRUE;
  }

  emberAfFillCommandDeviceManagementClusterRequestNewPasswordResponse(info.issuerEventId,
                                                                      password->implementationDateTime,
                                                                      password->durationInMinutes,
                                                                      password->passwordType,
                                                                      password->password);

  emberAfSendResponse();

  return TRUE;
}

// TODO: how many events?
/*
boolean emberAfDeviceManagementClusterReportEventConfigurationCallback(int8u commandIndex,
                                                                       int8u totalCommands,
                                                                       int8u* eventConfigurationPayload)
{
  return TRUE;
}
*/

boolean emberAfDeviceManagementClusterGetUncontrolledFlowThresholdCallback(void)
{
  EmberAfDeviceManagementUncontrolledFlowThreshold *threshold = &(info.threshold);

  emberAfFillCommandDeviceManagementClusterPublishUncontrolledFlowThreshold(info.providerId,
                                                                            info.issuerEventId,
                                                                            threshold->uncontrolledFlowThreshold,
                                                                            threshold->unitOfMeasure,
                                                                            threshold->multiplier,
                                                                            threshold->divisor,
                                                                            threshold->stabilisationPeriod,
                                                                            threshold->measurementPeriod);

  emberAfSendResponse();

  return TRUE;
}
