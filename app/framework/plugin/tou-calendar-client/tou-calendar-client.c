// Copyright 2014 Silicon Laboratories, Inc.

#include "app/framework/include/af.h"
#include "tou-calendar-client.h"

#ifndef EMBER_AF_GENERATE_CLI
  #error The TOU Calendar Client plugin is not compatible with the legacy CLI.
#endif

#define fieldLength(field) \
  (emberAfCurrentCommand()->bufLen - (field - emberAfCurrentCommand()->buffer))

static EmberAfStatus addCalendar(EmberAfTouCalendar *calendar);
static EmberAfTouCalendar *findCalendar(int32u providerId,
                                        int32u issuerCalendarId);
static void removeCalendar(EmberAfTouCalendar *calendar);

static EmberAfTouCalendar calendars[EMBER_AF_TOU_CALENDAR_CLUSTER_CLIENT_ENDPOINT_COUNT][EMBER_AF_PLUGIN_TOU_CALENDAR_CLIENT_CALENDARS];

boolean emberAfTouCalendarClusterPublishCalendarCallback(int32u providerId,
                                                         int32u issuerEventId,
                                                         int32u issuerCalendarId,
                                                         int32u startTime,
                                                         int8u calendarType,
                                                         int8u *calendarName,
                                                         int8u numberOfSeasons,
                                                         int8u numberOfWeekProfiles,
                                                         int8u numberOfDayProfiles)
{
  EmberAfStatus status;

  emberAfTouCalendarClusterPrint("RX: PublishCalendar 0x%4x, 0x%4x, 0x%4x, 0x%4x, 0x%x, \"",
                                 providerId,
                                 issuerEventId,
                                 issuerCalendarId,
                                 startTime,
                                 calendarType);
  emberAfTouCalendarClusterPrintString(calendarName);
  emberAfTouCalendarClusterPrintln("\", %d, %d, %d",
                                   numberOfSeasons,
                                   numberOfWeekProfiles,
                                   numberOfDayProfiles);

  if (findCalendar(providerId, issuerCalendarId) != NULL) {
    emberAfDebugPrintln("ERR: Duplicate calendar: 0x%4x/0x%4x",
                        providerId,
                        issuerCalendarId);
    status = EMBER_ZCL_STATUS_DUPLICATE_EXISTS;
  } else if (EMBER_ZCL_CALENDAR_TYPE_AUXILLIARY_LOAD_SWITCH_CALENDAR
             < calendarType) {
    emberAfDebugPrintln("ERR: Invalid calendar type: 0x%x", calendarType);
    status = EMBER_ZCL_STATUS_INVALID_FIELD;
  } else if (0 < numberOfSeasons && numberOfWeekProfiles == 0) {
    emberAfDebugPrintln("ERR: Number of week profiles cannot be zero");
    status = EMBER_ZCL_STATUS_INCONSISTENT;
  } else if (EMBER_AF_PLUGIN_TOU_CALENDAR_CLIENT_SEASONS < numberOfSeasons) {
    emberAfDebugPrintln("ERR: Insufficient space for seasons: %d < %d",
                        EMBER_AF_PLUGIN_TOU_CALENDAR_CLIENT_SEASONS,
                        numberOfSeasons);
    status = EMBER_ZCL_STATUS_INSUFFICIENT_SPACE;
  } else if (EMBER_AF_PLUGIN_TOU_CALENDAR_CLIENT_WEEK_PROFILES
             < numberOfWeekProfiles) {
    emberAfDebugPrintln("ERR: Insufficient space for week profiles: %d < %d",
                        EMBER_AF_PLUGIN_TOU_CALENDAR_CLIENT_WEEK_PROFILES,
                        numberOfWeekProfiles);
    status = EMBER_ZCL_STATUS_INSUFFICIENT_SPACE;
  } else if (numberOfDayProfiles == 0) {
    emberAfDebugPrintln("ERR: Number of day profiles cannot be zero");
    status = EMBER_ZCL_STATUS_INVALID_FIELD;
  } else if (EMBER_AF_PLUGIN_TOU_CALENDAR_CLIENT_DAY_PROFILES
             < numberOfDayProfiles) {
    emberAfDebugPrintln("ERR: Insufficient space for day profiles: %d < %d",
                        EMBER_AF_PLUGIN_TOU_CALENDAR_CLIENT_DAY_PROFILES,
                        numberOfDayProfiles);
    status = EMBER_ZCL_STATUS_INSUFFICIENT_SPACE;
  } else {
    EmberAfTouCalendar calendar;
    int8u i;
    calendar.inUse = TRUE;
    calendar.providerId = providerId;
    calendar.issuerEventId = issuerEventId;
    calendar.issuerCalendarId = issuerCalendarId;
    calendar.startTimeUtc = startTime;
    calendar.calendarType = calendarType;
    emberAfCopyString(calendar.calendarName,
                      calendarName,
                      EMBER_AF_TOU_CALENDAR_MAXIMUM_CALENDAR_NAME_LENGTH);
    calendar.numberOfSeasons = numberOfSeasons;
    calendar.receivedSeasons = 0;
    calendar.numberOfWeekProfiles = numberOfWeekProfiles;
    calendar.numberOfDayProfiles = numberOfDayProfiles;
    for (i = 0; i < EMBER_AF_PLUGIN_TOU_CALENDAR_CLIENT_WEEK_PROFILES; i++) {
      calendar.weekProfiles[i].inUse = FALSE;
    }
    for (i = 0; i < EMBER_AF_PLUGIN_TOU_CALENDAR_CLIENT_DAY_PROFILES; i++) {
      calendar.dayProfiles[i].inUse = FALSE;
    }
    calendar.specialDayProfile.inUse = FALSE;
    status = addCalendar(&calendar);
  }

  emberAfSendImmediateDefaultResponse(status);
  return TRUE;
}

boolean emberAfTouCalendarClusterPublishDayProfileCallback(int32u providerId,
                                                           int32u issuerEventId,
                                                           int32u issuerCalendarId,
                                                           int8u dayId,
                                                           int8u totalNumberOfScheduleEntries,
                                                           int8u commandIndex,
                                                           int8u totalNumberOfCommands,
                                                           int8u calendarType,
                                                           int8u *dayScheduleEntries)
{
  EmberAfStatus status;
  EmberAfTouCalendar *calendar;

  emberAfTouCalendarClusterPrint("RX: PublishDayProfile 0x%4x, 0x%4x, 0x%4x, %d, %d, %d, %d, 0x%x, [",
                                 providerId,
                                 issuerEventId,
                                 issuerCalendarId,
                                 dayId,
                                 totalNumberOfScheduleEntries,
                                 commandIndex,
                                 totalNumberOfCommands,
                                 calendarType);
  // TODO: print dayScheduleEntries
  emberAfTouCalendarClusterPrintln("]");

  // The PublishDayProfile command is published in response to a GetDayProfile
  // command.  If the IssuerCalendarID does not match with one of the stored
  // calendar instances, the client shall ignore the command and respond using
  // ZCL Default Response with a status response of NOT_FOUND.

  // The TOU Calendar server shall send only the number of Schedule Entries
  // belonging to this calendar instance.  Server and clients shall be able to
  // store at least 1 DayProfile and at least one ScheduleEntries per day
  // profile.  If the client is not able to store all ScheduleEntries, the
  // device should respond using ZCL Default Response with a status response of
  // INSUFFICIENT_SPACE.

  calendar = findCalendar(providerId, issuerCalendarId);
  if (calendar == NULL) {
    emberAfDebugPrintln("ERR: Calendar not found: 0x%4x/0x%4x",
                        providerId,
                        issuerCalendarId);
    status = EMBER_ZCL_STATUS_NOT_FOUND;
  } else if (dayId == 0) {
    emberAfDebugPrintln("ERR: Day id cannot be zero");
    status = EMBER_ZCL_STATUS_INVALID_FIELD;
  } else if (calendar->numberOfDayProfiles < dayId) {
    emberAfDebugPrintln("ERR: Invalid day: %d < %d",
                        calendar->numberOfDayProfiles,
                        dayId);
    status = EMBER_ZCL_STATUS_INCONSISTENT;
  } else if (EMBER_AF_PLUGIN_TOU_CALENDAR_CLIENT_DAY_PROFILES < dayId) {
    // This should never happen because of the previous check and the one in
    // PublishCalendar.  It is here for completeness.
    emberAfDebugPrintln("ERR: Insufficient space for day: %d < %d",
                        EMBER_AF_PLUGIN_TOU_CALENDAR_CLIENT_DAY_PROFILES,
                        dayId);
    status = EMBER_ZCL_STATUS_INSUFFICIENT_SPACE;
  } else if (EMBER_AF_PLUGIN_TOU_CALENDAR_CLIENT_SCHEDULE_ENTRIES
             < totalNumberOfScheduleEntries) {
    emberAfDebugPrintln("ERR: Insufficient space for schedule entries: %d < %d",
                        EMBER_AF_PLUGIN_TOU_CALENDAR_CLIENT_SCHEDULE_ENTRIES,
                        totalNumberOfScheduleEntries);
    status = EMBER_ZCL_STATUS_INSUFFICIENT_SPACE;
  } else if (totalNumberOfCommands <= commandIndex) {
    emberAfDebugPrintln("ERR: Inconsistent command index: %d <= %d",
                        totalNumberOfCommands,
                        commandIndex);
    status = EMBER_ZCL_STATUS_INCONSISTENT;
  } else if (calendar->calendarType != calendarType) {
    emberAfDebugPrintln("ERR: Inconsistent calendar type: 0x%x != 0x%x",
                        calendar->calendarType,
                        calendarType);
    status = EMBER_ZCL_STATUS_INCONSISTENT;
  } else {
    EmberAfTouCalendarDayProfile *dayProfile = &calendar->dayProfiles[dayId - 1];
    int16u dayScheduleEntriesLength = fieldLength(dayScheduleEntries);
    int16u dayScheduleEntriesIndex = 0;

    status = EMBER_ZCL_STATUS_SUCCESS;
    if (!dayProfile->inUse) {
      dayProfile->inUse = TRUE;
      dayProfile->numberOfScheduleEntries = totalNumberOfScheduleEntries;
      dayProfile->receivedScheduleEntries = 0;
    } else if (dayProfile->numberOfScheduleEntries
               != totalNumberOfScheduleEntries) {
      emberAfDebugPrintln("ERR: Inconsistent number of schedule entries: 0x%x != 0x%x",
                          dayProfile->numberOfScheduleEntries,
                          totalNumberOfScheduleEntries);
      status = EMBER_ZCL_STATUS_INCONSISTENT;
    }

    while (dayScheduleEntriesIndex < dayScheduleEntriesLength
           && status == EMBER_ZCL_STATUS_SUCCESS) {
      // The rate switch times, friendly credit switch times, and auxilliary
      // load switch times all use a two-byte start time followed by a one-
      // byte value.  These are the only valid types, so all entries must have
      // at least three bytes and, below, we just stuff the bytes into the rate
      // switch time entry in the union to simplify the code.
      if (dayScheduleEntriesLength < dayScheduleEntriesIndex + 3) {
        emberAfDebugPrintln("ERR: Malformed schedule entry");
        status = EMBER_ZCL_STATUS_MALFORMED_COMMAND;
      } else if (dayProfile->numberOfScheduleEntries
                 <= dayProfile->receivedScheduleEntries) {
        emberAfDebugPrintln("ERR: Inconsistent number of received schedule entries: %d <= %d",
                            dayProfile->numberOfScheduleEntries,
                            dayProfile->receivedScheduleEntries);
        status = EMBER_ZCL_STATUS_INCONSISTENT;
      } else if (EMBER_AF_PLUGIN_TOU_CALENDAR_CLIENT_SCHEDULE_ENTRIES
                 <= dayProfile->receivedScheduleEntries) {
        // This should never happen because of the checks above.  It is here
        // for completeness.
        emberAfDebugPrintln("ERR: Insufficient space for received schedule entries: %d <= %d",
                            EMBER_AF_PLUGIN_TOU_CALENDAR_CLIENT_SCHEDULE_ENTRIES,
                            dayProfile->receivedScheduleEntries);
        status = EMBER_ZCL_STATUS_INSUFFICIENT_SPACE;
      } else {
        EmberAfTouCalendarScheduleEntry *scheduleEntry = &dayProfile->scheduleEntries[dayProfile->receivedScheduleEntries++];
        scheduleEntry->rateSwitchTime.startTimeM = emberAfGetInt16u(dayScheduleEntries,
                                                                    dayScheduleEntriesIndex,
                                                                    dayScheduleEntriesLength);
        dayScheduleEntriesIndex += 2;
        scheduleEntry->rateSwitchTime.priceTier = emberAfGetInt8u(dayScheduleEntries,
                                                                  dayScheduleEntriesIndex,
                                                                  dayScheduleEntriesLength);
        dayScheduleEntriesIndex++;
        if (scheduleEntry != dayProfile->scheduleEntries) {
          EmberAfTouCalendarScheduleEntry *previous = scheduleEntry - 1;
          if (scheduleEntry->rateSwitchTime.startTimeM
              <= previous->rateSwitchTime.startTimeM) {
            emberAfDebugPrintln("ERR: Inconsistent start times: 0x%2x <= 0x%2x",
                                scheduleEntry->rateSwitchTime.startTimeM,
                                previous->rateSwitchTime.startTimeM);
            status = EMBER_ZCL_STATUS_INCONSISTENT;
          }
        }
      }
    }
  }

  if (calendar != NULL && status != EMBER_ZCL_STATUS_SUCCESS) {
    emberAfDebugPrintln("ERR: Removing invalid calendar: 0x%4x/0x%4x",
                        calendar->providerId,
                        calendar->issuerCalendarId);
    removeCalendar(calendar);
  }

  emberAfSendImmediateDefaultResponse(status);
  return TRUE;
}

boolean emberAfTouCalendarClusterPublishWeekProfileCallback(int32u providerId,
                                                            int32u issuerEventId,
                                                            int32u issuerCalendarId,
                                                            int8u weekId,
                                                            int8u dayIdRefMonday,
                                                            int8u dayIdRefTuesday,
                                                            int8u dayIdRefWednesday,
                                                            int8u dayIdRefThursday,
                                                            int8u dayIdRefFriday,
                                                            int8u dayIdRefSaturday,
                                                            int8u dayIdRefSunday)
{
  EmberAfStatus status;
  EmberAfTouCalendar *calendar;

  emberAfTouCalendarClusterPrintln("RX: PublishWeekProfile 0x%4x, 0x%4x, 0x%4x, %d, %d, %d, %d, %d, %d, %d, %d",
                                   providerId,
                                   issuerEventId,
                                   issuerCalendarId,
                                   weekId,
                                   dayIdRefMonday,
                                   dayIdRefTuesday,
                                   dayIdRefWednesday,
                                   dayIdRefThursday,
                                   dayIdRefFriday,
                                   dayIdRefSaturday,
                                   dayIdRefSunday);

  // The PublishWeekProfile command is published in response to a
  // GetWeekProfile command.  If the IssuerCalendarID does not match with one
  // of the stored calendar instances, the client shall ignore the command and
  // respond using ZCL Default Response with a status response of NOT_FOUND.

  // The Price server shall send only the number of WeekProfiles belonging to
  // this calendar instance.  Server and clients shall be able to store at
  // least 4 WeekProfiles.  If the client is not able to store all entries, the
  // device should respond using ZCL Default Response with a status response of
  // INSUFFICIENT_SPACE.

  calendar = findCalendar(providerId, issuerCalendarId);
  if (calendar == NULL) {
    emberAfDebugPrintln("ERR: Calendar not found: 0x%4x/0x%4x",
                        providerId,
                        issuerCalendarId);
    status = EMBER_ZCL_STATUS_NOT_FOUND;
  } else if (weekId == 0) {
    emberAfDebugPrintln("ERR: Day id cannot be zero");
    status = EMBER_ZCL_STATUS_INVALID_FIELD;
  } else if (calendar->numberOfWeekProfiles < weekId) {
    emberAfDebugPrintln("ERR: Invalid week: %d < %d",
                        calendar->numberOfWeekProfiles,
                        weekId);
    status = EMBER_ZCL_STATUS_INCONSISTENT;
  } else if (EMBER_AF_PLUGIN_TOU_CALENDAR_CLIENT_WEEK_PROFILES < weekId) {
    // This should never happen because of the previous check and the one in
    // PublishCalendar.  It is here for completeness.
    emberAfDebugPrintln("ERR: Insufficient space for week: %d < %d",
                        EMBER_AF_PLUGIN_TOU_CALENDAR_CLIENT_WEEK_PROFILES,
                        weekId);
    status = EMBER_ZCL_STATUS_INSUFFICIENT_SPACE;
  } else {
    EmberAfTouCalendarWeekProfile *weekProfile = &calendar->weekProfiles[weekId - 1];
    weekProfile->inUse = TRUE;
    status = EMBER_ZCL_STATUS_SUCCESS;
    if (calendar->numberOfDayProfiles < dayIdRefMonday) {
      emberAfDebugPrintln("ERR: Invalid %pday reference: %d < %d",
                          "Mon",
                          calendar->numberOfDayProfiles,
                          dayIdRefMonday);
      status = EMBER_ZCL_STATUS_INCONSISTENT;
    } else {
      weekProfile->dayIdRefMonday = dayIdRefMonday;
    }
    if (calendar->numberOfDayProfiles < dayIdRefTuesday) {
      emberAfDebugPrintln("ERR: Invalid %pday reference: %d < %d",
                          "Tues",
                          calendar->numberOfDayProfiles,
                          dayIdRefTuesday);
      status = EMBER_ZCL_STATUS_INCONSISTENT;
    } else {
      weekProfile->dayIdRefTuesday = dayIdRefTuesday;
    }
    if (calendar->numberOfDayProfiles < dayIdRefWednesday) {
      emberAfDebugPrintln("ERR: Invalid %pday reference: %d < %d",
                          "Wednes",
                          calendar->numberOfDayProfiles,
                          dayIdRefWednesday);
      status = EMBER_ZCL_STATUS_INCONSISTENT;
    } else {
      weekProfile->dayIdRefWednesday = dayIdRefWednesday;
    }
    if (calendar->numberOfDayProfiles < dayIdRefThursday) {
      emberAfDebugPrintln("ERR: Invalid %pday reference: %d < %d",
                          "Thurs",
                          calendar->numberOfDayProfiles,
                          dayIdRefThursday);
      status = EMBER_ZCL_STATUS_INCONSISTENT;
    } else {
      weekProfile->dayIdRefThursday = dayIdRefThursday;
    }
    if (calendar->numberOfDayProfiles < dayIdRefFriday) {
      emberAfDebugPrintln("ERR: Invalid %pday reference: %d < %d",
                          "Fri",
                          calendar->numberOfDayProfiles,
                          dayIdRefFriday);
      status = EMBER_ZCL_STATUS_INCONSISTENT;
    } else {
      weekProfile->dayIdRefFriday = dayIdRefFriday;
    }
    if (calendar->numberOfDayProfiles < dayIdRefSaturday) {
      emberAfDebugPrintln("ERR: Invalid %pday reference: %d < %d",
                          "Satur",
                          calendar->numberOfDayProfiles,
                          dayIdRefSaturday);
      status = EMBER_ZCL_STATUS_INCONSISTENT;
    } else {
      weekProfile->dayIdRefSaturday = dayIdRefSaturday;
    }
    if (calendar->numberOfDayProfiles < dayIdRefSunday) {
      emberAfDebugPrintln("ERR: Invalid %pday reference: %d < %d",
                          "Sun",
                          calendar->numberOfDayProfiles,
                          dayIdRefSunday);
      status = EMBER_ZCL_STATUS_INCONSISTENT;
    } else {
      weekProfile->dayIdRefSunday = dayIdRefSunday;
    }
  }

  if (calendar != NULL && status != EMBER_ZCL_STATUS_SUCCESS) {
    emberAfDebugPrintln("ERR: Removing invalid calendar: 0x%4x/0x%4x",
                        calendar->providerId,
                        calendar->issuerCalendarId);
    removeCalendar(calendar);
  }

  emberAfSendImmediateDefaultResponse(status);
  return TRUE;
}

boolean emberAfTouCalendarClusterPublishSeasonsCallback(int32u providerId,
                                                        int32u issuerEventId,
                                                        int32u issuerCalendarId,
                                                        int8u commandIndex,
                                                        int8u totalNumberOfCommands,
                                                        int8u *seasonEntries)
{
  EmberAfStatus status;
  EmberAfTouCalendar *calendar;

  emberAfTouCalendarClusterPrint("RX: PublishSeasons 0x%4x, 0x%4x, 0x%4x, %d, %d, [",
                                 providerId,
                                 issuerEventId,
                                 issuerCalendarId,
                                 commandIndex,
                                 totalNumberOfCommands);
  // TODO: print seasonEntries
  emberAfTouCalendarClusterPrintln("]");

  // The PublishSeasons command is published in response to a GetSeason
  // command.  If the IssuerCalendarID does not match with one of the stored
  // calendar instances, the client shall ignore the command and respond using
  // ZCL Default Response with a status response of NOT_FOUND.

  // The Price server shall send only the number of SeasonEntries belonging to
  // this calendar instance.  Server and clients shall be able to store at
  // least 4 SeasonEntries.  If the client is not able to store all Season
  // Entries, the device should respond using ZCL Default Response with a
  // status response of INSUFFICIENT_SPACE.

  calendar = findCalendar(providerId, issuerCalendarId);
  if (calendar == NULL) {
    emberAfDebugPrintln("ERR: Calendar not found: 0x%4x/0x%4x",
                        providerId,
                        issuerCalendarId);
    status = EMBER_ZCL_STATUS_NOT_FOUND;
  } else if (totalNumberOfCommands <= commandIndex) {
    emberAfDebugPrintln("ERR: Inconsistent command index: %d <= %d",
                        totalNumberOfCommands,
                        commandIndex);
    status = EMBER_ZCL_STATUS_INCONSISTENT;
  } else {
    int16u seasonEntriesLength = fieldLength(seasonEntries);
    int16u seasonEntriesIndex = 0;

    status = EMBER_ZCL_STATUS_SUCCESS;
    while (seasonEntriesIndex < seasonEntriesLength
           && status == EMBER_ZCL_STATUS_SUCCESS) {
      if (seasonEntriesLength < seasonEntriesIndex + 5) {
        emberAfDebugPrintln("ERR: Malformed season");
        status = EMBER_ZCL_STATUS_MALFORMED_COMMAND;
      } else if (calendar->numberOfSeasons <= calendar->receivedSeasons) {
        emberAfDebugPrintln("ERR: Inconsistent number of received seasons: %d <= %d",
                            calendar->numberOfSeasons,
                            calendar->receivedSeasons);
        status = EMBER_ZCL_STATUS_INCONSISTENT;
      } else if (EMBER_AF_PLUGIN_TOU_CALENDAR_CLIENT_SEASONS
                 <= calendar->receivedSeasons) {
        // This should never happen because of the previous check and the one
        // in PublishCalendar.  It is here for completeness.
        emberAfDebugPrintln("ERR: Insufficient space for received seasons: %d <= %d",
                            EMBER_AF_PLUGIN_TOU_CALENDAR_CLIENT_SEASONS,
                            calendar->receivedSeasons);
        status = EMBER_ZCL_STATUS_INSUFFICIENT_SPACE;
      } else {
        EmberAfTouCalendarSeason *season = &calendar->seasons[calendar->receivedSeasons++];
        season->seasonStartDate = emberAfGetInt32u(seasonEntries,
                                                   seasonEntriesIndex,
                                                   seasonEntriesLength);
        seasonEntriesIndex += 4;
        season->weekIdRef = emberAfGetInt8u(seasonEntries,
                                            seasonEntriesIndex,
                                            seasonEntriesLength);
        seasonEntriesIndex++;
        if (season != calendar->seasons
            && season->seasonStartDate <= (season - 1)->seasonStartDate) {
          emberAfDebugPrintln("ERR: Inconsistent start dates: 0x%4x <= 0x%4x",
                              season->seasonStartDate,
                              (season - 1)->seasonStartDate);
          status = EMBER_ZCL_STATUS_INCONSISTENT;
        } else if (calendar->numberOfWeekProfiles < season->weekIdRef) {
          emberAfDebugPrintln("ERR: Invalid week reference: %d < %d",
                              calendar->numberOfWeekProfiles,
                              season->weekIdRef);
          status = EMBER_ZCL_STATUS_INCONSISTENT;
        }
      }
    }
  }

  if (calendar != NULL && status != EMBER_ZCL_STATUS_SUCCESS) {
    emberAfDebugPrintln("ERR: Removing invalid calendar: 0x%4x/0x%4x",
                        calendar->providerId,
                        calendar->issuerCalendarId);
    removeCalendar(calendar);
  }

  emberAfSendImmediateDefaultResponse(status);
  return TRUE;
}

boolean emberAfTouCalendarClusterPublishSpecialDaysCallback(int32u providerId,
                                                            int32u issuerEventId,
                                                            int32u issuerCalendarId,
                                                            int32u startTime,
                                                            int8u calendarType,
                                                            int8u totalNumberOfSpecialDays,
                                                            int8u commandIndex,
                                                            int8u totalNumberOfCommands,
                                                            int8u *specialDayEntries)
{
  EmberAfStatus status;
  EmberAfTouCalendar *calendar;

  emberAfTouCalendarClusterPrint("RX: PublishSpecialDays 0x%4x, 0x%4x, 0x%4x, 0x%4x, 0x%x, %d, %d, %d, [",
                                 providerId,
                                 issuerEventId,
                                 issuerCalendarId,
                                 startTime,
                                 calendarType,
                                 totalNumberOfSpecialDays,
                                 commandIndex,
                                 totalNumberOfCommands);
  // TODO: print specialDayEntries
  emberAfTouCalendarClusterPrintln("]");

  // If the Calendar Type does not match with one of the stored calendar
  // instances, the client shall ignore the command and respond using ZCL
  // Default Response with a status response of NOT_FOUND.

  // Server and clients shall be able to store at least 50 SpecialDayEntries.
  // If the client is not able to store all SpecialDayEntries, the device
  // should respond using ZCL Default Response with a status response of
  // INSUFFICIENT_SPACE.

  // If the maximum application payload is not sufficient to transfer all
  // SpecialDayEntries in one command, the ESI may send as many
  // PublishSpecialDays commands as needed.
  // Note that, in this case, it is the client’s responsibility to ensure that
  // it receives all associated PublishSpecialDays commands before any of the
  // payloads can be used.

  calendar = findCalendar(providerId, issuerCalendarId);
  if (calendar == NULL) {
    emberAfDebugPrintln("ERR: Calendar not found: 0x%4x/0x%4x",
                        providerId,
                        issuerCalendarId);
    status = EMBER_ZCL_STATUS_NOT_FOUND;
  } else if (calendar->calendarType != calendarType) {
    emberAfDebugPrintln("ERR: Inconsistent calendar type: 0x%x != 0x%x",
                        calendar->calendarType,
                        calendarType);
    status = EMBER_ZCL_STATUS_INCONSISTENT;
  } else if (EMBER_AF_PLUGIN_TOU_CALENDAR_CLIENT_SPECIAL_DAY_ENTRIES
             < totalNumberOfSpecialDays) {
    emberAfDebugPrintln("ERR: Insufficient space for special day entries: %d < %d",
                        EMBER_AF_PLUGIN_TOU_CALENDAR_CLIENT_SPECIAL_DAY_ENTRIES,
                        totalNumberOfSpecialDays);
    status = EMBER_ZCL_STATUS_INSUFFICIENT_SPACE;
  } else if (totalNumberOfCommands <= commandIndex) {
    emberAfDebugPrintln("ERR: Inconsistent command index: %d <= %d",
                        totalNumberOfCommands,
                        commandIndex);
    status = EMBER_ZCL_STATUS_INCONSISTENT;
  } else {
    int16u specialDayEntriesLength = fieldLength(specialDayEntries);
    int16u specialDayEntriesIndex = 0;

    status = EMBER_ZCL_STATUS_SUCCESS;
    if (!calendar->specialDayProfile.inUse) {
      calendar->specialDayProfile.inUse = TRUE;
      calendar->specialDayProfile.startTimeUtc = startTime;
      calendar->specialDayProfile.numberOfSpecialDayEntries = totalNumberOfSpecialDays;
      calendar->specialDayProfile.receivedSpecialDayEntries = 0;
    } else if (calendar->specialDayProfile.numberOfSpecialDayEntries
               != totalNumberOfSpecialDays) {
      emberAfDebugPrintln("ERR: Inconsistent number of special day entries: 0x%x != 0x%x",
                          calendar->specialDayProfile.numberOfSpecialDayEntries,
                          totalNumberOfSpecialDays);
      status = EMBER_ZCL_STATUS_INCONSISTENT;
    }

    while (specialDayEntriesIndex < specialDayEntriesLength
           && status == EMBER_ZCL_STATUS_SUCCESS) {
      if (specialDayEntriesLength < specialDayEntriesIndex + 5) {
        emberAfDebugPrintln("ERR: Malformed special day entry");
        status = EMBER_ZCL_STATUS_MALFORMED_COMMAND;
      } else if (calendar->specialDayProfile.numberOfSpecialDayEntries
                 <= calendar->specialDayProfile.receivedSpecialDayEntries) {
        emberAfDebugPrintln("ERR: Inconsistent number of received special day entries: %d <= %d",
                            calendar->specialDayProfile.numberOfSpecialDayEntries,
                            calendar->specialDayProfile.receivedSpecialDayEntries);
        status = EMBER_ZCL_STATUS_INCONSISTENT;
      } else if (EMBER_AF_PLUGIN_TOU_CALENDAR_CLIENT_SPECIAL_DAY_ENTRIES
                 <= calendar->specialDayProfile.receivedSpecialDayEntries) {
        // This should never happen because of the checks above.  It is here
        // for completeness.
        emberAfDebugPrintln("ERR: Insufficient space for received special day entries: %d <= %d",
                            EMBER_AF_PLUGIN_TOU_CALENDAR_CLIENT_SPECIAL_DAY_ENTRIES,
                            calendar->specialDayProfile.receivedSpecialDayEntries);
        status = EMBER_ZCL_STATUS_INSUFFICIENT_SPACE;
      } else {
        EmberAfTouCalendarSpecialDayEntry *specialDayEntry = &calendar->specialDayProfile.specialDayEntries[calendar->specialDayProfile.receivedSpecialDayEntries++];
        specialDayEntry->specialDayDate = emberAfGetInt32u(specialDayEntries,
                                                           specialDayEntriesIndex,
                                                           specialDayEntriesLength);
        specialDayEntriesIndex += 4;
        specialDayEntry->dayIdRef = emberAfGetInt8u(specialDayEntries,
                                                    specialDayEntriesIndex,
                                                    specialDayEntriesLength);
        specialDayEntriesIndex++;
        if (calendar->numberOfDayProfiles < specialDayEntry->dayIdRef) {
          emberAfDebugPrintln("ERR: Invalid day reference: %d < %d",
                              calendar->numberOfDayProfiles,
                              specialDayEntry->dayIdRef);
          status = EMBER_ZCL_STATUS_INCONSISTENT;
        }
      }
    }
  }

  if (calendar != NULL && status != EMBER_ZCL_STATUS_SUCCESS) {
    emberAfDebugPrintln("ERR: Removing invalid calendar: 0x%4x/0x%4x",
                        calendar->providerId,
                        calendar->issuerCalendarId);
    removeCalendar(calendar);
  }

  emberAfSendImmediateDefaultResponse(status);
  return TRUE;
}

boolean emberAfTouCalendarClusterCancelCalendarCallback(int32u providerId,
                                                        int32u issuerEventId,
                                                        int8u calendarType)
{
  EmberAfStatus status;
  int8u endpoint = emberAfCurrentEndpoint();
  int8u index = emberAfFindClusterClientEndpointIndex(endpoint,
                                                      ZCL_TOU_CALENDAR_CLUSTER_ID);

  emberAfTouCalendarClusterPrintln("RX: CancelCalendar 0x%4x, 0x%4x, 0x%x",
                                   providerId,
                                   issuerEventId,
                                   calendarType);

  // The client device shall discard all instances of PublishCalendar,
  // PublishDayProfile, PublishWeekProfile, PublishSeasons, and
  // PublishSpecialDays commands associated with the stated Provider ID,
  // Calendar Type, and Issuer Calendar ID.

  if (EMBER_ZCL_CALENDAR_TYPE_AUXILLIARY_LOAD_SWITCH_CALENDAR < calendarType) {
    status = EMBER_ZCL_STATUS_INVALID_FIELD;
  } else {
    int8u i;
    status = EMBER_ZCL_STATUS_NOT_FOUND;
    for (i = 0; i < EMBER_AF_PLUGIN_TOU_CALENDAR_CLIENT_CALENDARS; i++) {
      if (providerId       == calendars[index][i].providerId
          && issuerEventId == calendars[index][i].issuerEventId
          && calendarType  == calendars[index][i].calendarType) {
        calendars[index][i].inUse = FALSE;
        status = EMBER_ZCL_STATUS_SUCCESS;
      }
    }
  }

  emberAfSendImmediateDefaultResponse(status);
  return TRUE;
}

static EmberAfStatus addCalendar(EmberAfTouCalendar *calendar)
{
  int32u nowUtc = emberAfGetCurrentTime();
  int8u endpoint = emberAfCurrentEndpoint();
  int8u index = emberAfFindClusterClientEndpointIndex(endpoint,
                                                      ZCL_TOU_CALENDAR_CLUSTER_ID);
  int8u i;

  if (calendar->startTimeUtc == 0) {
    calendar->startTimeUtc = nowUtc;
  }

  // Nested and overlapping TOU calendars are not allowed.  In the case of
  // overlapping calendars of the same type (calendar type), the calendar with
  // the newer IssuerCalendarID takes priority over all nested and overlapping
  // calendars.  All existing calendar instances that overlap, even partially,
  // should be removed.  The only exception to this is if a calendar instance
  // with a newer Issuer Event ID overlaps with the end of the current active
  // calendar but is not yet active, then the active calendar is not deleted
  // but modified so that the active calendar ends when the new calendar
  // begins.

  // First, check that this calendar has a newer issuer calendar id than all
  // active calendars of the same type.  If it doesn't, then it overlaps, and
  // must be rejected.
  for (i = 0; i < EMBER_AF_PLUGIN_TOU_CALENDAR_CLIENT_CALENDARS; i++) {
    if (calendars[index][i].inUse
        && calendar->issuerEventId < calendars[index][i].issuerEventId
        && calendar->calendarType == calendars[index][i].calendarType) {
      emberAfDebugPrintln("ERR: Overlaps with newer calendar: 0x%4x/0x%4x",
                          calendars[index][i].providerId,
                          calendars[index][i].issuerCalendarId);
      return EMBER_ZCL_STATUS_FAILURE;
    }
  }

  // We now know that this calendar is newer and that we're going to store it,
  // assuming we have room.  Before saving it, we need to check if it starts
  // before any existing calendars of the same type.  If so, it means those
  // calendars overlap with it and must be removed.
  for (i = 0; i < EMBER_AF_PLUGIN_TOU_CALENDAR_CLIENT_CALENDARS; i++) {
    if (calendars[index][i].inUse
        && calendar->startTimeUtc <= calendars[index][i].startTimeUtc
        && calendar->calendarType == calendars[index][i].calendarType) {
      emberAfDebugPrintln("INFO: Overlaps with older calendar: 0x%4x/0x%4x",
                          calendars[index][i].providerId,
                          calendars[index][i].issuerCalendarId);
      removeCalendar(&calendars[index][i]);
    }
  }

  // Now, look for any calendars that started in the past and have been
  // superceded by a newer calendar.  These are stale and can be removed.
  // TODO: Maybe this should only be done if we don't have space to store the
  // new calendar?
  for (i = 0; i < EMBER_AF_PLUGIN_TOU_CALENDAR_CLIENT_CALENDARS; i++) {
    if (calendars[index][i].inUse
        && calendars[index][i].startTimeUtc < nowUtc) {
      int8u j;
      for (j = 0; j < EMBER_AF_PLUGIN_TOU_CALENDAR_CLIENT_CALENDARS; j++) {
        if (i != j
            && calendars[index][j].inUse
            && (calendars[index][i].startTimeUtc
                < calendars[index][j].startTimeUtc)) {
          emberAfDebugPrintln("INFO: Removing expired calendar: 0x%4x/0x%4x",
                              calendars[index][i].providerId,
                              calendars[index][i].issuerCalendarId);
          removeCalendar(&calendars[index][i]);
        }
      }
    }
  }

  // Look for an empty slot to save this calendar.  If we don't have room, we
  // have to drop it.
  for (i = 0; i < EMBER_AF_PLUGIN_TOU_CALENDAR_CLIENT_CALENDARS; i++) {
    if (!calendars[index][i].inUse) {
      MEMCOPY(&calendars[index][i], calendar, sizeof(EmberAfTouCalendar));
      return EMBER_ZCL_STATUS_SUCCESS;
    }
  }

  emberAfDebugPrintln("ERR: Insufficient space for calendar: 0x%4x/0x%4x",
                      calendar->providerId,
                      calendar->issuerCalendarId);
  return EMBER_ZCL_STATUS_INSUFFICIENT_SPACE;
}

// This function uses emberAfCurrentEndpoint and therefore must only be called
// in the context of an incoming message.
static EmberAfTouCalendar *findCalendar(int32u providerId,
                                        int32u issuerCalendarId)
{
  int8u endpoint = emberAfCurrentEndpoint();
  int8u index = emberAfFindClusterClientEndpointIndex(endpoint,
                                                      ZCL_TOU_CALENDAR_CLUSTER_ID);
  int8u i;
  for (i = 0; i < EMBER_AF_PLUGIN_TOU_CALENDAR_CLIENT_CALENDARS; i++) {
    if (calendars[index][i].inUse
        && calendars[index][i].providerId == providerId
        && calendars[index][i].issuerCalendarId == issuerCalendarId) {
      return &calendars[index][i];
    }
  }
  return NULL;
}

static void removeCalendar(EmberAfTouCalendar *calendar)
{
  emberAfDebugPrintln("INFO: Removed calendar: 0x%4x/0x%4x",
                      calendar->providerId,
                      calendar->issuerCalendarId);
  calendar->inUse = FALSE;
}

// plugin tou-calendar-client clear <endpoint:1>
void emberAfPluginTouCalendarClientClearCommand(void)
{
  int8u endpoint = (int8u)emberUnsignedCommandArgument(0);
  int8u index = emberAfFindClusterClientEndpointIndex(endpoint,
                                                      ZCL_TOU_CALENDAR_CLUSTER_ID);
  if (index == 0xFF) {
    return;
  }

  int8u i;
  for (i = 0; i < EMBER_AF_PLUGIN_TOU_CALENDAR_CLIENT_CALENDARS; i++) {
    if (calendars[index][i].inUse) {
      removeCalendar(&calendars[index][i]);
    }
  }
}

// plugin tou-calendar-client print <endpoint:1>
void emberAfPluginTouCalendarClientPrintCommand(void)
{
  int8u endpoint = (int8u)emberUnsignedCommandArgument(0);
  int8u index = emberAfFindClusterClientEndpointIndex(endpoint,
                                                      ZCL_TOU_CALENDAR_CLUSTER_ID);
  if (index == 0xFF) {
    return;
  }

  int8u i, j, k;
  for (i = 0; i < EMBER_AF_PLUGIN_TOU_CALENDAR_CLIENT_CALENDARS; i++) {
    const EmberAfTouCalendar *calendar = &calendars[index][i];
    if (!calendar->inUse) {
      continue;
    }

    emberAfTouCalendarClusterPrintln("calendar:                       %d",    i);
    emberAfTouCalendarClusterPrintln("  providerId:                   0x%4x", calendar->providerId);
    emberAfTouCalendarClusterPrintln("  issuerEventId:                0x%4x", calendar->issuerEventId);
    emberAfTouCalendarClusterPrintln("  issuerCalendarId:             0x%4x", calendar->issuerCalendarId);
    emberAfTouCalendarClusterPrintln("  startTimeUtc:                 0x%4x", calendar->startTimeUtc);
    emberAfTouCalendarClusterPrintln("  calendarType:                 0x%x",  calendar->calendarType);
    emberAfTouCalendarClusterPrint(  "  calendarName:                 \"");
    emberAfTouCalendarClusterPrintString(calendar->calendarName);
    emberAfTouCalendarClusterPrintln("\"");
    emberAfTouCalendarClusterPrintln("  numberOfSeasons:              %d", calendar->numberOfSeasons);
    emberAfTouCalendarClusterPrintln("  receivedSeasons:              %d", calendar->receivedSeasons);
    emberAfTouCalendarClusterPrintln("  numberOfWeekProfiles:         %d", calendar->numberOfWeekProfiles);
    emberAfTouCalendarClusterPrintln("  numberOfDayProfiles:          %d", calendar->numberOfDayProfiles);

    for (j = 0; j < calendar->receivedSeasons; j++) {
      const EmberAfTouCalendarSeason *season = &calendar->seasons[j];
      emberAfTouCalendarClusterPrintln("  season:                       %d",    i);
      emberAfTouCalendarClusterPrintln("    seasonStartDate:            0x%4x", season->seasonStartDate);
      emberAfTouCalendarClusterPrintln("    weekIdRef:                  %d",    season->weekIdRef);
    }

    for (j = 0; j < calendar->numberOfWeekProfiles; j++) {
      const EmberAfTouCalendarWeekProfile *weekProfile = &calendar->weekProfiles[j];
      if (weekProfile->inUse) {
        emberAfTouCalendarClusterPrintln("  weekProfile:                  %d", j);
        emberAfTouCalendarClusterPrintln("    weekId:                     %d", j + 1);
        emberAfTouCalendarClusterPrintln("    dayIdRefMonday:             %d", weekProfile->dayIdRefMonday);
        emberAfTouCalendarClusterPrintln("    dayIdRefTuesday:            %d", weekProfile->dayIdRefTuesday);
        emberAfTouCalendarClusterPrintln("    dayIdRefWednesday:          %d", weekProfile->dayIdRefWednesday);
        emberAfTouCalendarClusterPrintln("    dayIdRefThursday:           %d", weekProfile->dayIdRefThursday);
        emberAfTouCalendarClusterPrintln("    dayIdRefFriday:             %d", weekProfile->dayIdRefFriday);
        emberAfTouCalendarClusterPrintln("    dayIdRefSaturday:           %d", weekProfile->dayIdRefSaturday);
        emberAfTouCalendarClusterPrintln("    dayIdRefSunday:             %d", weekProfile->dayIdRefSunday);
      }
    }

    for (j = 0; j < calendar->numberOfDayProfiles; j++) {
      const EmberAfTouCalendarDayProfile *dayProfile = &calendar->dayProfiles[j];
      if (dayProfile->inUse) {
        emberAfTouCalendarClusterPrintln("  dayProfile:                   %d", j);
        emberAfTouCalendarClusterPrintln("    dayId:                      %d", j + 1);
        emberAfTouCalendarClusterPrintln("    numberOfScheduleEntries:    %d", dayProfile->numberOfScheduleEntries);
        emberAfTouCalendarClusterPrintln("    receivedScheduleEntries:    %d", dayProfile->receivedScheduleEntries);
        for (k = 0; k < dayProfile->receivedScheduleEntries; k++) {
          const EmberAfTouCalendarScheduleEntry *scheduleEntry = &dayProfile->scheduleEntries[k];
          emberAfTouCalendarClusterPrintln("    scheduleEntry:              %d", k);
          switch (calendar->calendarType) {
          case EMBER_ZCL_CALENDAR_TYPE_DELIVERED_CALENDAR:
          case EMBER_ZCL_CALENDAR_TYPE_RECEIVED_CALENDAR:
          case EMBER_ZCL_CALENDAR_TYPE_DELIVERED_AND_RECEIVED_CALENDAR:
            emberAfTouCalendarClusterPrintln("      startTimeM:               0x%2x", scheduleEntry->rateSwitchTime.startTimeM);
            emberAfTouCalendarClusterPrintln("      priceTier:                0x%x",  scheduleEntry->rateSwitchTime.priceTier);
            break;
          case EMBER_ZCL_CALENDAR_TYPE_FRIENDLY_CREDIT_CALENDAR:
            emberAfTouCalendarClusterPrintln("      startTimeM:               0x%2x", scheduleEntry->friendlyCreditSwitchTime.startTimeM);
            emberAfTouCalendarClusterPrintln("      friendlyCreditEnable:     %p",    scheduleEntry->friendlyCreditSwitchTime.friendlyCreditEnable ? "TRUE" : "FALSE");
            break;
          case EMBER_ZCL_CALENDAR_TYPE_AUXILLIARY_LOAD_SWITCH_CALENDAR:
            emberAfTouCalendarClusterPrintln("      startTimeM:               0x%2x", scheduleEntry->auxilliaryLoadSwitchTime.startTimeM);
            emberAfTouCalendarClusterPrintln("      auxiliaryLoadSwitchState: 0x%x",  scheduleEntry->auxilliaryLoadSwitchTime.auxiliaryLoadSwitchState);
            break;
          }
        }
      }
    }

    const EmberAfTouCalendarSpecialDayProfile *specialDayProfile = &calendar->specialDayProfile;
    if (specialDayProfile->inUse) {
      emberAfTouCalendarClusterPrintln("  specialDayProfile:");
      emberAfTouCalendarClusterPrintln("    startTimeUtc:               0x%4x", specialDayProfile->startTimeUtc);
      emberAfTouCalendarClusterPrintln("    numberOfSpecialDayEntries:  %d",    specialDayProfile->numberOfSpecialDayEntries);
      emberAfTouCalendarClusterPrintln("    receivedSpecialDayEntries:  %d",    specialDayProfile->receivedSpecialDayEntries);
      for (j = 0; j < specialDayProfile->receivedSpecialDayEntries; j++) {
        const EmberAfTouCalendarSpecialDayEntry *specialDayEntry = &specialDayProfile->specialDayEntries[j];
        emberAfTouCalendarClusterPrintln("    specialDayEntry:            %d",    j);
        emberAfTouCalendarClusterPrintln("      specialDayDate:           0x%4x", specialDayEntry->specialDayDate);
        emberAfTouCalendarClusterPrintln("      dayIdRef:                 %d",    specialDayEntry->dayIdRef);
      }
    }
  }
}
