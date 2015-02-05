
#define EMBER_AF_PLUGIN_TOU_CALENDAR_COMMON_INVALID_SCHEDULE_ENTRY 0xFFFF
#define EMBER_AF_PLUGIN_TOU_CALENDAR_COMMON_INVALID_ID 0xFF
#define EMBER_AF_PLUGIN_TOU_CALENDAR_MAX_CALENDAR_NAME_LENGTH 12

typedef struct {
  int16u minutesFromMidnight;
  
  // the format of the actual data in the entry depends on the calendar type
  //   for calendar type 00 - 0x02, it is a rate switch time
  //     the data is a price tier enum (8-bit)
  //   for calendar type 0x03 it is friendly credit switch time
  //     the data is a boolean (8-bit) meaning friendly credit enabled
  //   for calendar type 0x04 it is an auxilliary load switch time
  //     the data is a bitmap (8-bit)
  int8u data;  
} EmberAfTouCalendarDayScheduleEntryStruct ;

typedef struct {
  EmberAfTouCalendarDayScheduleEntryStruct scheduleEntries[EMBER_AF_PLUGIN_TOU_CALENDAR_COMMON_SCHEDULE_ENTRIES_MAX];
  int8u id;
  int8u numberOfScheduleEntries;
} EmberAfTouCalendarDayStruct;

typedef struct {
  int32u startDate;
  int8u normalDayIndex;
  int8u flags;
} EmberAfTouCalendarSpecialDayStruct;

#define EMBER_AF_PLUGIN_TOU_CALENDAR_COMMON_MONDAY_INDEX 0
#define EMBER_AF_PLUGIN_TOU_CALENDAR_COMMON_SUNDAY_INDEX 6
#define EMBER_AF_DAYS_IN_THE_WEEK 7


typedef struct {
  int8u normalDayIndexes[EMBER_AF_DAYS_IN_THE_WEEK];
  int8u id;
} EmberAfTouCalendarWeekStruct;

typedef struct {
  int32u startDate;
  int8u weekIndex;
} EmberAfTouCalendarSeasonStruct;

#define EMBER_AF_PLUGIN_TOU_CALENDAR_COMMON_INVALID_CALENDAR_ID 0xFFFFFFFF
#define EMBER_AF_PLUGIN_TOU_CALENDAR_COMMON_WILDCARD_CALENDAR_ID 0x00000000
#define EMBER_AF_PLUGIN_TOU_CALENDAR_COMMON_WILDCARD_PROVIDER_ID 0xFFFFFFFF
#define EMBER_AF_PLUGIN_TOU_CALENDAR_COMMON_WILDCARD_ISSUER_ID 0xFFFFFFFF
#define EMBER_AF_PLUGIN_TOU_CALENDAR_COMMON_WILDCARD_CALENDAR_TYPE 0xFF

enum {
  EMBER_AF_PLUGIN_TOU_CALENDAR_COMMON_FLAGS_SENT = 0x01,
};

typedef struct {
  EmberAfTouCalendarWeekStruct weeks[EMBER_AF_PLUGIN_TOU_CALENDAR_COMMON_WEEK_PROFILE_MAX];
  EmberAfTouCalendarDayStruct normalDays[EMBER_AF_PLUGIN_TOU_CALENDAR_COMMON_DAY_PROFILE_MAX];
  EmberAfTouCalendarSpecialDayStruct specialDays[EMBER_AF_PLUGIN_TOU_CALENDAR_COMMON_SPECIAL_DAY_PROFILE_MAX];
  EmberAfTouCalendarSeasonStruct seasons[EMBER_AF_PLUGIN_TOU_CALENDAR_COMMON_SEASON_PROFILE_MAX];
  int32u providerId;
  int32u issuerEventId;
  int32u calendarId;
  int32u startTimeUtc;
  int8u name[EMBER_AF_PLUGIN_TOU_CALENDAR_MAX_CALENDAR_NAME_LENGTH + 1];
  int8u calendarType;
  int8u numberOfSeasons;
  int8u numberOfWeekProfiles;
  int8u numberOfDayProfiles;
  int8u numberOfSpecialDayProfiles;
  int8u flags;
} EmberAfTouCalendarStruct;


boolean emberAfPluginTouCalendarCommonGetLocalCalendar(int8u index,
                                                       EmberAfTouCalendarStruct *calendar);
boolean emberAfPluginTouCalendarCommonGetCalendarById(int32u calendarId,
                                                      int32u providerId,
                                                      EmberAfTouCalendarStruct *calendar);
boolean emberAfPluginTouCalendarCommonSetLocalCalendarEntry(int8u index,
                                                            EmberAfTouCalendarStruct *calendar);
int32u emberAfPluginTouCalendarCommonEndTimeUtc(const EmberAfTouCalendarStruct *calendar);
int32u emberAfPluginTouCalendarCommonEndDate(const EmberAfTouCalendarStruct *calendar,
                                             const EmberAfTouCalendarSpecialDayStruct *specialDay);
