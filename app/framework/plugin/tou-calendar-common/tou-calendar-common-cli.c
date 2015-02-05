// ****************************************************************************
// * tou-calendar-common-cli.c
// *
// *
// * Copyright 2014 Silicon Laboratories, Inc.                              *80*
// ****************************************************************************

#include "app/framework/include/af.h"
#include "tou-calendar-common.h"

//-----------------------------------------------------------------------------
// Globals

int8u calendarName0[EMBER_AF_PLUGIN_TOU_CALENDAR_MAX_CALENDAR_NAME_LENGTH + 1] =
  {0x0C, 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l'};
int8u calendarName1[EMBER_AF_PLUGIN_TOU_CALENDAR_MAX_CALENDAR_NAME_LENGTH + 1] = 
  {0x0C, 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x'};

const char* calendarTypeStrings[] = {
  "Delivered",
  "Received",
  "Delivered and Received",
  "Friendly Credit",
  "Auxilliary Load Switch",
};
#define MAX_CALENDAR_TYPE EMBER_ZCL_CALENDAR_TYPE_AUXILLIARY_LOAD_SWITCH_CALENDAR

#define PROVIDER_ID_START     0x0000A000
#define ISSUER_EVENT_ID_START 0x0000B000
#define CALENDAR_ID_START     0

#define MINUTES_PER_HOUR 60
#define MINUTES_PER_DAY (MINUTES_PER_HOUR * 24)

#define SCHEDULE_ENTRIES_DATA_OFFSET 0xC0

static int8u calendarIndexForPrinting = 0;

//-----------------------------------------------------------------------------

static boolean printSelectedCalendar(void)
{
  EmberAfTouCalendarStruct calendar; 
  boolean calendarFound = emberAfPluginTouCalendarCommonGetLocalCalendar(calendarIndexForPrinting, 
                                                                         &calendar);
  emberAfTouCalendarClusterPrintln("Selected Calendar Index: %d",
                                   calendarIndexForPrinting);
  if (!calendarFound
      || calendar.calendarId == EMBER_AF_PLUGIN_TOU_CALENDAR_COMMON_INVALID_CALENDAR_ID) {
    emberAfTouCalendarClusterPrintln("No data.");
    return FALSE;
  }
  return TRUE;
}

static void printScheduleEntryData(int8u calendarType, int8u data)
{
  switch (calendarType) {
    case EMBER_ZCL_CALENDAR_TYPE_DELIVERED_CALENDAR:
    case EMBER_ZCL_CALENDAR_TYPE_RECEIVED_CALENDAR:
    case EMBER_ZCL_CALENDAR_TYPE_DELIVERED_AND_RECEIVED_CALENDAR:
      emberAfTouCalendarClusterPrint("Price Tier: 0x%X", data);
      break;
    case EMBER_ZCL_CALENDAR_TYPE_FRIENDLY_CREDIT_CALENDAR:
      emberAfTouCalendarClusterPrint("Friendly credit: %p", 
                                     (data == 0
                                      ? "not available"
                                      : "enabled"));
      break;
    case EMBER_ZCL_CALENDAR_TYPE_AUXILLIARY_LOAD_SWITCH_CALENDAR:
      emberAfTouCalendarClusterPrint("Auxilliary Switch state: 0x%X", data);
      break;
    default:
      emberAfTouCalendarClusterPrint("Invalid calendar type (%d), data: %d",
                                     calendarType,
                                     data);
      break;
  }
}

static void printTime(const char* prefix,
                      int32u zigbeeUtcTime)
{
  EmberAfTimeStruct time;
  emberAfFillTimeStructFromUtc(zigbeeUtcTime, &time);
  emberAfTouCalendarClusterPrint("%p", prefix);
  emberAfTouCalendarClusterPrintln("%4X (%d/%p%d/%p%d, %p%d:%p%d:%p%d)",
                                   zigbeeUtcTime,
                                   time.year,
                                   (time.month < 10
                                    ? " "
                                    : ""),
                                   time.month,
                                   (time.day < 10
                                    ? " "
                                    : ""),
                                   time.day,
                                   (time.hours < 10
                                    ? " "
                                    : ""),
                                   time.hours,
                                   (time.minutes < 10
                                    ? " "
                                    : ""),
                                   time.minutes,
                                   (time.seconds < 10
                                    ? " "
                                    : ""),
                                   time.seconds);
}

static void printScheduleEntry(const EmberAfTouCalendarDayScheduleEntryStruct* scheduleEntry,
                               int8u calendarType)
{
  int8u hours = 0;
  int8u minutes = 0;
  if (scheduleEntry->minutesFromMidnight != 0) {
    hours = scheduleEntry->minutesFromMidnight / MINUTES_PER_HOUR;
    minutes = scheduleEntry->minutesFromMidnight % MINUTES_PER_HOUR;
  }
  emberAfTouCalendarClusterPrint("%s%d:%s%d - ",
                                 (hours < 10
                                  ? " "
                                  : ""),
                                 hours,
                                 (minutes < 10
                                  ? "0"
                                  : ""),
                                 minutes);
  printScheduleEntryData(calendarType, scheduleEntry->data);
  emberAfTouCalendarClusterPrintln("");
}

static void printDayProfile(const EmberAfTouCalendarDayStruct* day,
                            int8u calendarType)
{
  int8u i;
  emberAfTouCalendarClusterPrintln("Day id: %d - Schedule",
                                   day->id);
  for (i = 0; i < day->numberOfScheduleEntries; i++) {
    emberAfTouCalendarClusterPrint("  ");
    printScheduleEntry(&(day->scheduleEntries[i]),
                       calendarType);
  }
}

static void printSpecialDayProfile(const EmberAfTouCalendarSpecialDayStruct* specialDay,
                                   const EmberAfTouCalendarDayStruct* normalDays)
{
  emberAfTouCalendarClusterPrintln("Day ID: %d", normalDays[specialDay->normalDayIndex].id);
  printTime("Start Date: ", specialDay->startDate);
}

static void printDayProfiles(const EmberAfTouCalendarStruct* calendar,
                             boolean printSpecialDays)
{
  int8u i;
  int8u max = (printSpecialDays
               ? calendar->numberOfSpecialDayProfiles
               : calendar->numberOfDayProfiles);
  for (i = 0; i < max; i++) {
    if (printSpecialDays) {
      printSpecialDayProfile(&(calendar->specialDays[i]), calendar->normalDays);
    } else {
      printDayProfile(&(calendar->normalDays[i]),
                      calendar->calendarType);
    }
  }
}

static void printCalendar(const EmberAfTouCalendarStruct* calendar)
{
  EmberAfTimeStruct time;
  emberAfFillTimeStructFromUtc(calendar->startTimeUtc, &time);
  emberAfTouCalendarClusterPrint("Name: ");
  emberAfTouCalendarClusterPrintString(calendar->name);
  emberAfTouCalendarClusterPrintln("");
  emberAfTouCalendarClusterPrintln("ID:              0x%4X", calendar->calendarId);
  emberAfTouCalendarClusterPrintln("Issuer Event ID: 0x%4X", calendar->issuerEventId);
  emberAfTouCalendarClusterPrintln("Provider ID:     0x%4X", calendar->providerId);
  emberAfTouCalendarClusterPrintln("Type:            %d (%p)",
                                   calendar->calendarType,
                                   (calendar->calendarType <= MAX_CALENDAR_TYPE
                                    ? calendarTypeStrings[calendar->calendarType]
                                    : "???"));
  printTime("Start Time:      ", calendar->startTimeUtc);
  emberAfTouCalendarClusterPrintln("Season Profiles:      %d", calendar->numberOfSeasons);
  emberAfTouCalendarClusterPrintln("Week Profiles:        %d", calendar->numberOfWeekProfiles);
  emberAfTouCalendarClusterPrintln("Day Profiles:         %d", calendar->numberOfDayProfiles);
  emberAfTouCalendarClusterPrintln("Special Day Profiles: %d", calendar->numberOfSpecialDayProfiles);
}

void emberAfPluginTouCalendarCommonPrintSummaryCommand(void)
{
  EmberAfTouCalendarStruct calendar;
  boolean calendarFound = emberAfPluginTouCalendarCommonGetLocalCalendar(calendarIndexForPrinting,
                                                                         &calendar);
  if (!calendarFound || !printSelectedCalendar()) {
    return;
  }
  printCalendar(&calendar);
  emberAfTouCalendarClusterPrintln("");
}

void emberAfPluginTouCalendarCommonSelectCommand(void)
{
  int8u temp = (int8u)emberUnsignedCommandArgument(0);
  if (temp >= EMBER_AF_PLUGIN_TOU_CALENDAR_COMMON_TOTAL_CALENDARS) {
    emberAfTouCalendarClusterPrintln("Error: Invalid index.  Max is %d.",
                                     EMBER_AF_PLUGIN_TOU_CALENDAR_COMMON_TOTAL_CALENDARS - 1);
    return;
  }
  calendarIndexForPrinting = temp;
}

void emberAfPluginTouCalendarCommonPrintDaysCommand(void)
{
  int8u commandChar0 = emberCurrentCommand->name[0];
  EmberAfTouCalendarStruct calendar;
  boolean calendarFound = emberAfPluginTouCalendarCommonGetLocalCalendar(calendarIndexForPrinting,
                                                                         &calendar);
  if (!calendarFound || !printSelectedCalendar()) {
    return;
  }

  if (commandChar0 == 'd') {
    emberAfTouCalendarClusterPrintln("Normal Day Profiles");
    printDayProfiles(&calendar, FALSE);  // special days?
  } else {
    emberAfTouCalendarClusterPrintln("Special Day Profiles");
    printDayProfiles(&calendar, TRUE);   // special days?
  }
}

// The offset is useful for making each day's schedule entry different so as it tell
// the days apart.
static void setupTestScheduleEntries(EmberAfTouCalendarDayStruct* day, int8u offset)
{
  int8u i;
  for (i = 0; i < day->numberOfScheduleEntries; i++) {
    int16u temp = (i * MINUTES_PER_HOUR) + offset;
    if (temp >= MINUTES_PER_DAY) {
      temp = MINUTES_PER_DAY - 1;
    }
    day->scheduleEntries[i].minutesFromMidnight = temp;
    day->scheduleEntries[i].data = SCHEDULE_ENTRIES_DATA_OFFSET + i;
  }
}

static void setupTestDays(EmberAfTouCalendarStruct* calendar)
{
  int8u i;
  for (i = 0; i < calendar->numberOfDayProfiles; i++) {
    calendar->normalDays[i].id = i + 1; // day ids start at one
    calendar->normalDays[i].numberOfScheduleEntries = EMBER_AF_PLUGIN_TOU_CALENDAR_COMMON_SCHEDULE_ENTRIES_MAX;
    setupTestScheduleEntries(&(calendar->normalDays[i]), i);
  }
  for (i = 0; i < calendar->numberOfSpecialDayProfiles; i++) {
    calendar->specialDays[i].startDate = (i * (60 * 60 * 24));
    calendar->specialDays[i].normalDayIndex = 6;
    calendar->specialDays[i].flags = 0;
  }
}

static void setupTestWeeks(EmberAfTouCalendarStruct* calendar)
{
  int8u i;
  for (i = 0; i < calendar->numberOfWeekProfiles; i++) {
    int8u j;
    calendar->weeks[i].id = i + 1; // week ids start at one
    for (j = 0; j < EMBER_AF_DAYS_IN_THE_WEEK; j++) {
      calendar->weeks[i].normalDayIndexes[j] = (j < EMBER_AF_PLUGIN_TOU_CALENDAR_COMMON_DAY_PROFILE_MAX
                                                ? j
                                                : EMBER_AF_PLUGIN_TOU_CALENDAR_COMMON_INVALID_ID);
    }
  }
}

static void setupTestSeasons(EmberAfTouCalendarStruct* calendar)
{
  int8u i; 
  for (i = 0; i < calendar->numberOfSeasons; i++) {
    if (calendar->weeks[i].id != EMBER_AF_PLUGIN_TOU_CALENDAR_COMMON_INVALID_ID) {
      calendar->seasons[i].startDate = (i * (60 * 60 * 24));
      calendar->seasons[i].weekIndex = i;
    } else {
      calendar->seasons[i].weekIndex = EMBER_AF_PLUGIN_TOU_CALENDAR_COMMON_INVALID_ID;
    }
  }
}

void emberAfPluginTouCalendarCommonSetupTestCalendarsCommand(void)
{
  int8u i;
  int32u currentTimeUtc;
  for (i = 0; i < EMBER_AF_PLUGIN_TOU_CALENDAR_COMMON_TOTAL_CALENDARS; i++) {
    EmberAfTouCalendarStruct calendar;
    boolean calendarFound = emberAfPluginTouCalendarCommonGetLocalCalendar(i,
                                                                           &calendar);
    if (!calendarFound) {
      emberAfTouCalendarClusterPrintln("Calendar at index %d could not be accessed in setup.", i);
      continue;
    }
    emberAfCopyString(calendar.name,
                      (i == 0 ? calendarName0 : calendarName1),
                      EMBER_AF_PLUGIN_TOU_CALENDAR_MAX_CALENDAR_NAME_LENGTH);

    calendar.calendarId = CALENDAR_ID_START + i;
    calendar.startTimeUtc = (i * 1000);
    calendar.providerId = PROVIDER_ID_START + i;
    calendar.issuerEventId = ISSUER_EVENT_ID_START + i;
    calendar.calendarType = EMBER_ZCL_CALENDAR_TYPE_DELIVERED_CALENDAR;
    calendar.numberOfSeasons = EMBER_AF_PLUGIN_TOU_CALENDAR_COMMON_SEASON_PROFILE_MAX;
    calendar.numberOfDayProfiles = EMBER_AF_PLUGIN_TOU_CALENDAR_COMMON_DAY_PROFILE_MAX;
    calendar.numberOfWeekProfiles = EMBER_AF_PLUGIN_TOU_CALENDAR_COMMON_WEEK_PROFILE_MAX;
    calendar.numberOfSpecialDayProfiles = EMBER_AF_PLUGIN_TOU_CALENDAR_COMMON_SPECIAL_DAY_PROFILE_MAX;
    calendar.flags = 0;

    setupTestDays(&calendar);
    setupTestWeeks(&calendar);
    setupTestSeasons(&calendar);

    // Maybe there is some benefit to having a copy of the calendar table localized to the CLI, so I'll leave 
    // most of this intact. However, we DO need to actually propagate the table through to the plugin code for
    // it to be useful.
    calendarFound = emberAfPluginTouCalendarCommonSetLocalCalendarEntry(i, &calendar);
    if (!calendarFound) {
      emberAfTouCalendarClusterPrintln("Calendar entry at index %d could not be set.", i);
    }
  }
  emberAfTouCalendarClusterPrintln("Calendars initialized with test-data.");
}

void emberAfPluginTouCalendarCommonPrintSeasonsCommand(void)
{
  int8u i;
  EmberAfTouCalendarStruct calendar;
  boolean calendarFound = emberAfPluginTouCalendarCommonGetLocalCalendar(calendarIndexForPrinting,
                                                                         &calendar);
  if (!calendarFound || !printSelectedCalendar()) {
    return;
  }
  for (i = 0; i < EMBER_AF_PLUGIN_TOU_CALENDAR_COMMON_SEASON_PROFILE_MAX; i++) {
    emberAfTouCalendarClusterPrintln("Season: %d", i);
    if (calendar.seasons[i].weekIndex == EMBER_AF_PLUGIN_TOU_CALENDAR_COMMON_INVALID_ID) {
      emberAfTouCalendarClusterPrintln("  No data");
    } else {
      printTime("  Start Date:  ", calendar.seasons[i].startDate);
      emberAfTouCalendarClusterPrintln("  Week ID: %d", calendar.weeks[calendar.seasons[i].weekIndex].id);
    }
  }
}
