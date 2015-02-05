// *******************************************************************
// * device-management-server.h
// *
// *
// * Copyright 2014 by Silicon Laboratories. All rights reserved.           *80*
// *******************************************************************

#define EMBER_AF_DEVICE_MANAGEMENT_MAXIMUM_PROVIDER_NAME_LENGTH 16
#define EMBER_AF_DEVICE_MANAGEMENT_MAXIMUM_PASSWORD_LENGTH 10
#define EMBER_AF_DEVICE_MANAGEMENT_MAXIMUM_SITE_ID_LENGTH 32

enum {
  UNUSED_PASSWORD   = 0x00,
  SERVICE_PASSWORD  = 0x01,
  CONSUMER_PASSWORD = 0x02,
};

typedef struct {
  int32u implementationDateTime;
  int32u tenancy;
} EmberAfDeviceManagementTenancy;

typedef struct {
  int8u providerName[EMBER_AF_DEVICE_MANAGEMENT_MAXIMUM_PROVIDER_NAME_LENGTH + 1];
  int32u proposedProviderId;
  int32u implementationDateTime;
  int32u providerChangeControl;
} EmberAfDeviceManagementSupplier;

typedef struct {
  int32u requestDateTime;
  int32u implementationDateTime;
  int8u supplyStatus;
  int8u originatorIdSupplyControlBits;
} EmberAfDeviceManagementSupply;

typedef struct {
  int8u siteId[EMBER_AF_DEVICE_MANAGEMENT_MAXIMUM_SITE_ID_LENGTH + 1];
  int32u siteIdTime;
} EmberAfDeviceManagementSiteId;

typedef struct {
  boolean supplyTamperState;
  boolean supplyDepletionState;
  boolean supplyUncontrolledFlowState;
  boolean loadLimitSupplyState;
} EmberAfDeviceManagementSupplyStatusFlags;

typedef struct {
  int32u implementationDateTime;
  int8u supplyStatus;
} EmberAfDeviceManagementSupplyStatus;

typedef struct {
  int8u password[EMBER_AF_DEVICE_MANAGEMENT_MAXIMUM_PASSWORD_LENGTH + 1];
  int32u implementationDateTime;
  int16u durationInMinutes;
  int8u passwordType;
} EmberAfDeviceManagementPassword;

typedef struct {
  int16u uncontrolledFlowThreshold;
  int16u multiplier;
  int16u divisor;
  int16u measurementPeriod;
  int8u unitOfMeasure;
  int8u stabilisationPeriod;
} EmberAfDeviceManagementUncontrolledFlowThreshold;

typedef struct {
  EmberAfDeviceManagementTenancy tenancy;
  EmberAfDeviceManagementSupplier supplier;
  EmberAfDeviceManagementSupply supply;
  EmberAfDeviceManagementSiteId siteId;
  EmberAfDeviceManagementSupplyStatusFlags supplyStatusFlags;
  EmberAfDeviceManagementSupplyStatus supplyStatus;
  //TODO: These passwords ought to be tokenized / hashed
  EmberAfDeviceManagementPassword servicePassword;
  EmberAfDeviceManagementPassword consumerPassword;
  EmberAfDeviceManagementUncontrolledFlowThreshold threshold;
  int32u providerId;
  int32u issuerEventId;
  int8u proposedLocalSupplyStatus;
  int8u tariffType;
} EmberAfDeviceManagementInfo;

boolean emberAfPluginDeviceManagementSetTenancy(EmberAfDeviceManagementTenancy *tenancy);
boolean emberAfPluginDeviceManagementGetTenancy(EmberAfDeviceManagementTenancy *tenancy);

boolean emberAfPluginDeviceManagementSetSupplier(EmberAfDeviceManagementSupplier *supplier);
boolean emberAfPluginDeviceManagementGetSupplier(EmberAfDeviceManagementSupplier *supplier);

boolean emberAfPluginDeviceManagementSetSupply(EmberAfDeviceManagementSupply *supply);
boolean emberAfPluginDeviceManagementGetSupply(EmberAfDeviceManagementSupply *supply);

boolean emberAfPluginDeviceManagementSetInfoGlobalData(int32u providerId,
                                                       int32u issuerEventId,
                                                       int8u tariffType);

boolean emberAfPluginDeviceManagementSetSupplyStatus(EmberAfDeviceManagementSupplyStatus *status);
boolean emberAfPluginDeviceManagementGetSupplyStatus(EmberAfDeviceManagementSupplyStatus *status);

boolean emberAfPluginDeviceManagementSetSiteId(EmberAfDeviceManagementSiteId *siteId);
boolean emberAfPluginDeviceManagementGetSiteId(EmberAfDeviceManagementSiteId *siteId);

boolean emberAfPluginDeviceManagementSetPassword(EmberAfDeviceManagementPassword *password);
boolean emberAfPluginDeviceManagementGetPassword(EmberAfDeviceManagementPassword *password,
                                                 int8u passwordType);

boolean emberAfPluginDeviceManagementSetUncontrolledFlowThreshold(EmberAfDeviceManagementUncontrolledFlowThreshold *threshold);
boolean emberAfPluginDeviceManagementGetUncontrolledFlowThreshold(EmberAfDeviceManagementUncontrolledFlowThreshold *threshold);

void emberAfDeviceManagementServerPrint(void);
