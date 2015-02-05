// Copyright 2014 Silicon Laboratories, Inc.

#define EMBER_AF_TOU_CALENDAR_MAXIMUM_CALENDAR_NAME_LENGTH 12

typedef struct {
  int32u seasonStartDate;
  int8u weekIdRef;
} EmberAfTouCalendarSeason;

typedef struct {
  boolean inUse;
  int8u dayIdRefMonday;
  int8u dayIdRefTuesday;
  int8u dayIdRefWednesday;
  int8u dayIdRefThursday;
  int8u dayIdRefFriday;
  int8u dayIdRefSaturday;
  int8u dayIdRefSunday;
} EmberAfTouCalendarWeekProfile;

// All valid calendar types have the same general format for schedule entries:
// a two-byte start time followed by a one-byte, type-specific value.  The code
// in this plugin takes advantage of this similarity to simply the logic.  If
// new types are added, the code will need to change.  See
// emberAfTouCalendarClusterPublishDayProfileCallback.
typedef union {
  struct {
    int16u startTimeM;
    int8u priceTier;
  } rateSwitchTime;
  struct {
    int16u startTimeM;
    boolean friendlyCreditEnable;
  } friendlyCreditSwitchTime;
  struct {
    int16u startTimeM;
    int8u auxiliaryLoadSwitchState;
  } auxilliaryLoadSwitchTime;
} EmberAfTouCalendarScheduleEntry;

typedef struct {
  boolean inUse;
  int8u numberOfScheduleEntries;
  int8u receivedScheduleEntries;
  EmberAfTouCalendarScheduleEntry scheduleEntries[EMBER_AF_PLUGIN_TOU_CALENDAR_CLIENT_SCHEDULE_ENTRIES];
} EmberAfTouCalendarDayProfile;

typedef struct {
  int32u specialDayDate;
  int8u dayIdRef;
} EmberAfTouCalendarSpecialDayEntry;

typedef struct {
  boolean inUse;
  int32u startTimeUtc;
  int8u numberOfSpecialDayEntries;
  int8u receivedSpecialDayEntries;
  EmberAfTouCalendarSpecialDayEntry specialDayEntries[EMBER_AF_PLUGIN_TOU_CALENDAR_CLIENT_SPECIAL_DAY_ENTRIES];
} EmberAfTouCalendarSpecialDayProfile;

typedef struct {
  boolean inUse;
  int32u providerId;
  int32u issuerEventId;
  int32u issuerCalendarId;
  int32u startTimeUtc;
  EmberAfCalendarType calendarType;
  int8u calendarName[EMBER_AF_TOU_CALENDAR_MAXIMUM_CALENDAR_NAME_LENGTH + 1];
  int8u numberOfSeasons;
  int8u receivedSeasons;
  int8u numberOfWeekProfiles;
  int8u numberOfDayProfiles;
  EmberAfTouCalendarSeason seasons[EMBER_AF_PLUGIN_TOU_CALENDAR_CLIENT_SEASONS];
  EmberAfTouCalendarWeekProfile weekProfiles[EMBER_AF_PLUGIN_TOU_CALENDAR_CLIENT_WEEK_PROFILES];
  EmberAfTouCalendarDayProfile dayProfiles[EMBER_AF_PLUGIN_TOU_CALENDAR_CLIENT_DAY_PROFILES];
  EmberAfTouCalendarSpecialDayProfile specialDayProfile;
} EmberAfTouCalendar;
