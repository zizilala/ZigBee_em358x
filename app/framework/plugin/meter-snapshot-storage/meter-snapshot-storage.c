// *****************************************************************************
// * meter-snapshot-storage.c
// *
// * Code to handle meter snapshot storage behavior.
// * 
// * Copyright 2013 by Silicon Labs. All rights reserved.                   *80*
// *****************************************************************************

#include "app/framework/include/af.h"
#include "meter-snapshot-storage.h"

static EmberAfSnapshotPayload snapshots[EMBER_AF_PLUGIN_METER_SNAPSHOT_STORAGE_SNAPSHOT_CAPACITY];

static EmberAfSnapshotSchedulePayload schedules[EMBER_AF_PLUGIN_METER_SNAPSHOT_STORAGE_SCHEDULE_CAPACITY];

static int32u snapshotIdCounter;

static void initSnapshots(void);
static void initSchedules(void);
static EmberAfSnapshotPayload *allocateSnapshot(void);
static EmberAfSnapshotSchedulePayload *allocateSchedule(void);
static void processTiers(int8u endpoint, 
                         EmberAfSnapshotPayload *snapshot, 
                         boolean delivered);
static void processTiersAndBlocks(int8u endpoint, 
                                  EmberAfSnapshotPayload *snapshot, 
                                  boolean delivered);
static EmberAfSnapshotPayload *findSnapshot(int32u startTime,
                                            int8u snapshotOffset,
                                            int32u snapshotCause);
static int16u fillPayloadBuffer(int8u *buffer, EmberAfSnapshotPayload *payload);

static void initSnapshots(void) 
{
  int8u i;

  for (i = 0; i < EMBER_AF_PLUGIN_METER_SNAPSHOT_STORAGE_SNAPSHOT_CAPACITY; i++) {
    snapshots[i].snapshotId = INVALID_SNAPSHOT_ID;
  }
}

static void initSchedules(void) 
{
  int8u i;
  for (i = 0; i < EMBER_AF_PLUGIN_METER_SNAPSHOT_STORAGE_SCHEDULE_CAPACITY; i++) {
    schedules[i].snapshotScheduleId = INVALID_SNAPSHOT_SCHEDULE_ID;
  }
}

static EmberAfSnapshotPayload *allocateSnapshot(void)
{
  int8u i;
  for (i = 0; i < EMBER_AF_PLUGIN_METER_SNAPSHOT_STORAGE_SNAPSHOT_CAPACITY; i++) {
    if (snapshots[i].snapshotId == INVALID_SNAPSHOT_ID) {
      return &snapshots[i];
    }
  }

  return NULL;
}

static EmberAfSnapshotSchedulePayload *allocateSchedule(void)
{
  int8u i;

  for (i = 0; i < EMBER_AF_PLUGIN_METER_SNAPSHOT_STORAGE_SCHEDULE_CAPACITY; i++) {
    if (schedules[i].snapshotScheduleId == INVALID_SNAPSHOT_SCHEDULE_ID) {
      return &schedules[i];
    }
  }

  return NULL;
}

void emberAfPluginMeterSnapshotStorageInitCallback(int8u endpoint)
{
  snapshotIdCounter = INVALID_SNAPSHOT_ID;
  initSchedules();
  initSnapshots();
}

void emberAfPluginMeterSnapshotServerScheduleSnapshotCallback(int8u srcEndpoint,
                                                              int8u dstEndpoint,
                                                              EmberNodeId dest,
                                                              int8u *snapshotPayload,
                                                              int8u *responsePayload)
{
  EmberAfSnapshotSchedulePayload *schedule;
  int8u scheduleId, index = 0;
  EmberAfSnapshotScheduleConfirmation confirmation = ACCEPTED;

  // Attempt to allocate a schedule
  schedule = allocateSchedule();

  if (schedule == NULL) {
    confirmation = INSUFFICIENT_SPACE;
    goto kickout;
  }

  // Set the schedule
  schedule->snapshotScheduleId = snapshotPayload[index];
  if (schedule->snapshotScheduleId == INVALID_SNAPSHOT_SCHEDULE_ID) {
    confirmation = SCHEDULE_NOT_AVAILABLE;
    goto kickout;
  }
  index++;

  schedule->snapshotStartDate = emberAfGetInt32u(snapshotPayload,
                                                 index,
                                                 SNAPSHOT_SCHEDULE_PAYLOAD_SIZE);
  index += 4;
  schedule->snapshotSchedule = emberAfGetInt24u(snapshotPayload,
                                                index,
                                                SNAPSHOT_SCHEDULE_PAYLOAD_SIZE);
  index += 3;
  schedule->snapshotPayloadType = emberAfGetInt8u(snapshotPayload,
                                                  index,
                                                  SNAPSHOT_SCHEDULE_PAYLOAD_SIZE);
  index++;
  schedule->snapshotCause = emberAfGetInt32u(snapshotPayload,
                                             index,
                                             SNAPSHOT_SCHEDULE_PAYLOAD_SIZE);

  // Log where to send the snapshot(s) when the time is right
  schedule->requestingId = dest;
  schedule->srcEndpoint = srcEndpoint;
  schedule->dstEndpoint = dstEndpoint;

kickout:
  // Fill the response payload
  responsePayload[0] = schedule->snapshotScheduleId;
  responsePayload[1] = confirmation;
}

int32u emberAfPluginMeterSnapshotServerTakeSnapshotCallback(int8u endpoint,
                                                            int32u snapshotCause,
                                                            int8u *snapshotConfirmation)
{
  EmberAfSnapshotPayload *snapshot;
  EmberAfClusterCommand *cmd = emberAfCurrentCommand();
  int32u snapshotId = INVALID_SNAPSHOT_ID;
  int8u manualType = EMBER_AF_PLUGIN_METER_SNAPSHOT_STORAGE_MANUAL_SNAPSHOT_TYPE;
  boolean delivered = (manualType == 0 || manualType == 2);

  // Attempt to allocate a snapshot
  snapshot = allocateSnapshot();

  if (snapshot == NULL) {
    // FAIL
    goto kickout;
  }

  // Set up snapshot identification information
  snapshot->snapshotId = ++snapshotIdCounter;

  if (snapshot->snapshotId == INVALID_SNAPSHOT_ID) {
    *snapshotConfirmation = 0x01;
    goto kickout;
  }

  snapshotId = snapshot->snapshotId;

  snapshot->payloadType = manualType;
  snapshot->tiersInUse = SUMMATION_TIERS;
  snapshot->tiersAndBlocksInUse = BLOCK_TIERS;
  snapshot->snapshotTime = emberAfGetCurrentTime();
  snapshot->snapshotCause = snapshotCause | SNAPSHOT_CAUSE_MANUAL;
  emberAfReadAttribute(endpoint,
                       ZCL_SIMPLE_METERING_CLUSTER_ID,
                       (delivered
                        ? ZCL_CURRENT_SUMMATION_DELIVERED_ATTRIBUTE_ID
                        : ZCL_CURRENT_SUMMATION_RECEIVED_ATTRIBUTE_ID),
                       CLUSTER_MASK_SERVER,
                       (int8u *)(snapshot->currentSummation),
                       sizeof(snapshot->currentSummation),
                       NULL);
  emberAfReadAttribute(endpoint,
                       ZCL_SIMPLE_METERING_CLUSTER_ID,
                       (delivered
                        ? ZCL_BILL_TO_DATE_DELIVERED_ATTRIBUTE_ID
                        : ZCL_BILL_TO_DATE_RECEIVED_ATTRIBUTE_ID),
                       CLUSTER_MASK_SERVER,
                       (int8u *)&(snapshot->billToDate),
                       sizeof(snapshot->billToDate),
                       NULL);
  emberAfReadAttribute(endpoint,
                       ZCL_SIMPLE_METERING_CLUSTER_ID,
                       (delivered
                        ? ZCL_BILL_TO_DATE_TIME_STAMP_DELIVERED_ATTRIBUTE_ID
                        : ZCL_BILL_TO_DATE_TIME_STAMP_RECEIVED_ATTRIBUTE_ID),
                       CLUSTER_MASK_SERVER,
                       (int8u *)&(snapshot->billToDateTimeStamp),
                       sizeof(snapshot->billToDateTimeStamp),
                       NULL);
  emberAfReadAttribute(endpoint,
                       ZCL_SIMPLE_METERING_CLUSTER_ID,
                       (delivered
                        ? ZCL_PROJECTED_BILL_DELIVERED_ATTRIBUTE_ID
                        : ZCL_PROJECTED_BILL_RECEIVED_ATTRIBUTE_ID),
                       CLUSTER_MASK_SERVER,
                       (int8u *)&(snapshot->projectedBill),
                       sizeof(snapshot->projectedBill),
                       NULL);
  emberAfReadAttribute(endpoint,
                       ZCL_SIMPLE_METERING_CLUSTER_ID,
                       (delivered 
                        ? ZCL_PROJECTED_BILL_TIME_STAMP_DELIVERED_ATTRIBUTE_ID
                        : ZCL_PROJECTED_BILL_TIME_STAMP_RECEIVED_ATTRIBUTE_ID),
                       CLUSTER_MASK_SERVER,
                       (int8u *)&(snapshot->projectedBillTimeStamp),
                       sizeof(snapshot->projectedBillTimeStamp),
                       NULL);
  emberAfReadAttribute(endpoint,
                       ZCL_SIMPLE_METERING_CLUSTER_ID,
                       (delivered
                        ? ZCL_BILL_DELIVERED_TRAILING_DIGIT_ATTRIBUTE_ID
                        : ZCL_BILL_RECEIVED_TRAILING_DIGIT_ATTRIBUTE_ID),
                       CLUSTER_MASK_SERVER,
                       (int8u *)&(snapshot->billTrailingDigit),
                       sizeof(snapshot->billTrailingDigit),
                       NULL);

  processTiers(endpoint, snapshot, delivered);
  processTiersAndBlocks(endpoint, snapshot, delivered);
 
  *snapshotConfirmation = 0x00;
kickout:
  return snapshotId;
}

void emberAfPluginMeterSnapshotServerGetSnapshotCallback(int8u srcEndpoint,
                                                         int8u dstEndpoint,
                                                         EmberNodeId dest,
                                                         int8u *snapshotCriteria)
{
  int32u startTime = emberAfGetInt32u(snapshotCriteria, 0, 9);
  int8u snapshotOffset = emberAfGetInt8u(snapshotCriteria, 4, 9);
  int32u snapshotCause = emberAfGetInt32u(snapshotCriteria, 5, 9);
  int8u snapshotPayload[SNAPSHOT_PAYLOAD_SIZE];
  int16u payloadSize;
  
  // Find the snapshot
  EmberAfSnapshotPayload *payload = findSnapshot(startTime,
                                                 snapshotOffset,
                                                 snapshotCause);
  if (payload == NULL) {
    return;
  }

  // Fill our payload buffer
  payloadSize = fillPayloadBuffer(snapshotPayload, payload);
  
  emberAfFillCommandSimpleMeteringClusterPublishSnapshot(payload->snapshotId,
                                                         payload->snapshotTime,
                                                         1,
                                                         0,
                                                         1,
                                                         payload->snapshotCause,
                                                         payload->payloadType,
                                                         (int8u *)payload,
                                                         payloadSize);
  emberAfSetCommandEndpoints(srcEndpoint, dstEndpoint);
  emberAfSendCommandUnicast(EMBER_OUTGOING_DIRECT, dest);
}

static void processTiers(int8u endpoint, 
                         EmberAfSnapshotPayload *snapshot, 
                         boolean delivered)
{
  int16u i, attr = (delivered
                    ? ZCL_CURRENT_TIER1_SUMMATION_DELIVERED_ATTRIBUTE_ID
                    : ZCL_CURRENT_TIER1_SUMMATION_RECEIVED_ATTRIBUTE_ID);

  for (i = 0; i < SUMMATION_TIERS; i++) {
    emberAfReadAttribute(endpoint,
                         ZCL_SIMPLE_METERING_CLUSTER_ID,
                         attr + i,
                         CLUSTER_MASK_SERVER,
                         (int8u *)(&(snapshot->tierSummation[i * 6])),
                         6,
                         NULL);
  }
}

static void processTiersAndBlocks(int8u endpoint, 
                                  EmberAfSnapshotPayload *snapshot, 
                                  boolean delivered)
{
  int16u i, attr = (delivered
                    ? ZCL_CURRENT_TIER1_BLOCK1_SUMMATION_DELIVERED_ATTRIBUTE_ID
                    : ZCL_CURRENT_TIER1_BLOCK1_SUMMATION_RECEIVED_ATTRIBUTE_ID);

  for (i = 0; i < BLOCK_TIERS; i++) {
    emberAfReadAttribute(endpoint,
                         ZCL_SIMPLE_METERING_CLUSTER_ID,
                         attr + i,
                         CLUSTER_MASK_SERVER,
                         (int8u *)(&(snapshot->tierBlockSummation[i * 6])),
                         6,
                         NULL);
  }
}

static EmberAfSnapshotPayload *findSnapshot(int32u startTime,
                                            int8u snapshotOffset,
                                            int32u snapshotCause)
{
  int8u i, offsetCount = snapshotOffset;
  EmberAfSnapshotPayload *snapshot;
  for (i = 0; i < EMBER_AF_PLUGIN_METER_SNAPSHOT_STORAGE_SNAPSHOT_CAPACITY; i++) {
    snapshot = &(snapshots[i]);
    if (snapshot->snapshotTime >= startTime) {
      if (snapshotCause == 0xFFFFFFFF || (snapshot->snapshotCause & snapshotCause)) {
        if (offsetCount == 0) {
          return &(snapshots[i]);
        }
        offsetCount--;
      }
    }
  }

  return NULL;
}

static int16u fillPayloadBuffer(int8u *buffer, EmberAfSnapshotPayload *payload)
{
  int16u index = 0;
  int16u i;
  boolean block = (payload->payloadType == 2 || payload->payloadType == 3);

  MEMCOPY(buffer, payload->currentSummation, 6);
  index += 6;
  
  emberAfCopyInt32u(buffer, index, payload->billToDate);
  index += 4;

  emberAfCopyInt32u(buffer, index, payload->billToDateTimeStamp);
  index += 4;

  emberAfCopyInt32u(buffer, index, payload->projectedBill);
  index += 4;

  emberAfCopyInt32u(buffer, index, payload->projectedBillTimeStamp);
  index += 4;

  emberAfCopyInt8u(buffer, index, payload->billTrailingDigit);
  index++;

  emberAfCopyInt8u(buffer, index, payload->tiersInUse);
  index++;

  for (i = 0; i < SUMMATION_TIERS; i++) {
    MEMCOPY((buffer+index), &(payload->tierSummation[i*6]), 6);
    index += 6;
  }

  // Our payload is bigger if we're operating on the block information set
  if (block) {
    emberAfCopyInt8u(buffer, index, payload->tiersAndBlocksInUse);
    index++;

    for (i = 0; i < BLOCK_TIERS; i++) {
      MEMCOPY((buffer+index), &(payload->tierBlockSummation[i*6]), 6);
      index += 6;
    }
  }

  return index;
}
