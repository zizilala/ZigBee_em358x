#define INVALID_SNAPSHOT_SCHEDULE_ID 0

// 0 is technically valid, but we'll designate it to be our "invalid" id for init purposes
#define INVALID_SNAPSHOT_ID 0

#define SNAPSHOT_SCHEDULE_PAYLOAD_SIZE 15

#define SNAPSHOT_CAUSE_MANUAL 0x00000400

#define SUMMATION_TIERS EMBER_AF_PLUGIN_METER_SNAPSHOT_STORAGE_SUM_TIERS_SUPPORTED 
#define BLOCK_TIERS  EMBER_AF_PLUGIN_METER_SNAPSHOT_STORAGE_BLOCK_TIERS_SUPPORTED 
#define SNAPSHOT_PAYLOAD_SIZE SUMMATION_TIERS + BLOCK_TIERS + 94

typedef struct {
  int8u tierSummation[SUMMATION_TIERS * 6];
  int8u tierBlockSummation[BLOCK_TIERS * 6];
  int8u currentSummation[6];
  int32u billToDate;
  int32u billToDateTimeStamp;
  int32u projectedBill;
  int32u projectedBillTimeStamp;
  int32u snapshotId;
  int32u snapshotTime;
  int32u snapshotCause;
  EmberNodeId requestingId;
  int8u tiersInUse;
  int8u tiersAndBlocksInUse;
  int8u srcEndpoint;
  int8u dstEndpoint;
  int8u billTrailingDigit;
  int8u payloadType;
} EmberAfSnapshotPayload;

typedef struct {
  int32u snapshotStartDate;
  int32u snapshotSchedule;
  int32u snapshotCause;
  EmberNodeId requestingId;
  int8u srcEndpoint;
  int8u dstEndpoint;
  int8u snapshotPayloadType;
  int8u snapshotScheduleId;
} EmberAfSnapshotSchedulePayload;

typedef enum {
  ACCEPTED                = 0x00,
  TYPE_NOT_SUPPORTED      = 0x01,
  CAUSE_NOT_SUPPORTED     = 0x02,
  SCHEDULE_NOT_AVAILABLE  = 0x03,
  SCHEDULES_NOT_SUPPORTED = 0x04,
  INSUFFICIENT_SPACE      = 0x05
} EmberAfSnapshotScheduleConfirmation;

