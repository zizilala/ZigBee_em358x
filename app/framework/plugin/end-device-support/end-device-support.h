// Copyright 2013 Silicon Laboratories, Inc.

extern boolean emAfEnablePollCompletedCallback;

typedef struct {
  int32u pollIntervalMs;
  int8u numPollsFailing;
} EmAfPollingState;
extern EmAfPollingState emAfPollingStates[];
void emAfPollCompleteHandler(EmberStatus status, int8u limit);
