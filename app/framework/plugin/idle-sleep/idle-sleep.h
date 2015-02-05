// Copyright 2013 Silicon Laboratories, Inc.

extern boolean emAfStayAwakeWhenNotJoined;
extern boolean emAfForceEndDeviceToStayAwake;

void emberAfForceEndDeviceToStayAwake(boolean stayAwake);
void emAfPrintSleepDuration(int32u sleepDurationMS, int8u eventIndex);
void emAfPrintForceAwakeStatus(void);
boolean emAfOkToIdleOrSleep(void);