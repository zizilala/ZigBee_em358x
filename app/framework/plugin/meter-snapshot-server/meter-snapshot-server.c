// *****************************************************************************
// * meter-snapshot-server.c
// *
// * Code to handle meter snapshot server behavior.
// * 
// * Copyright 2013 by Silicon Labs. All rights reserved.                   *80*
// *****************************************************************************

#include "app/framework/include/af.h"
#include "app/framework/util/common.h"

boolean emberAfSimpleMeteringClusterScheduleSnapshotCallback(int32u issuerEventId,
                                                             int8u commandIndex,
                                                             int8u commandCount,
                                                             int8u* snapshotSchedulePayload)
{
  EmberAfClusterCommand *cmd = emberAfCurrentCommand();
  int8u responsePayload[2];

  // Attempt to schedule the snapshot
  emberAfPluginMeterSnapshotServerScheduleSnapshotCallback(cmd->apsFrame->destinationEndpoint,
                                                           cmd->apsFrame->sourceEndpoint,
                                                           cmd->source,
                                                           snapshotSchedulePayload,
                                                           (int8u *)responsePayload);

  emberAfFillCommandSimpleMeteringClusterScheduleSnapshotResponse(issuerEventId,
                                                                  responsePayload,
                                                                  2);

  emberAfSendResponse();
  return TRUE;
}

boolean emberAfSimpleMeteringClusterTakeSnapshotCallback(int32u snapshotCause)
{
  int8u endpoint = emberAfCurrentEndpoint(), *snapshotConfirmation;
  int32u snapshotId;

  // Attempt to take the snapshot
  snapshotId = emberAfPluginMeterSnapshotServerTakeSnapshotCallback(endpoint,
                                                                    snapshotCause,
                                                                    snapshotConfirmation);

  emberAfFillCommandSimpleMeteringClusterTakeSnapshotResponse(snapshotId,
                                                              *snapshotConfirmation);
  return TRUE;
}

boolean emberAfSimpleMeteringClusterGetSnapshotCallback(int32u startTime,
                                                        int8u numberOfSnapshots,
                                                        int32u snapshotCause)
{
  EmberAfClusterCommand *cmd = emberAfCurrentCommand();
  int8u snapshotCriteria[9];

  // Package the snapshot criteria for our callback to process
  emberAfCopyInt32u((int8u *)snapshotCriteria, 0, startTime);
  snapshotCriteria[4] = numberOfSnapshots;
  emberAfCopyInt32u((int8u *)snapshotCriteria, 5, snapshotCause);

  // Get / publish the snapshot
  emberAfPluginMeterSnapshotServerGetSnapshotCallback(cmd->apsFrame->destinationEndpoint,
                                                      cmd->apsFrame->sourceEndpoint,
                                                      cmd->source,
                                                      (int8u *)snapshotCriteria);

  return TRUE;
}
