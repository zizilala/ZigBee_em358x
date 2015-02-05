// ****************************************************************************
// * tou-calendar-server.c
// *
// *
// * Copyright 2014 Silicon Laboratories, Inc.                              *80*
// ****************************************************************************

#include "app/framework/include/af.h"
#include "app/framework/plugin/tou-calendar-common/tou-calendar-common.h"

//-----------------------------------------------------------------------------
// Globals 

// Each schedule entry is 3 bytes (2 bytes for start time, and 1 byte for data)
#define SCHEDULE_ENTRY_SIZE 3

// Season start date (4-bytes) and week ID ref (1-byte)
#define SEASON_ENTRY_SIZE 5  

// Special day date (4 bytes) and Day ID ref (1-byte)
#define SPECIAL_DAY_ENTRY_SIZE 5

static int8u currentCalendar = 0;

static int8u myEndpoint = 0;

//-----------------------------------------------------------------------------

void emberAfTouCalendarClusterServerInitCallback(int8u endpoint)
{
  int8u i;

  if (endpoint != 0 && myEndpoint == 0) {
    myEndpoint = endpoint;
  }

  for (i = 0 ; 
       i < EMBER_AF_PLUGIN_TOU_CALENDAR_COMMON_TOTAL_CALENDARS ;
       i++) {
    EmberAfTouCalendarStruct calendar;
    boolean calendarFound = emberAfPluginTouCalendarCommonGetLocalCalendar(i,
                                                                           &calendar);
    if (calendarFound) {
      calendar.startTimeUtc = 0xFFFFFFFF;
      calendarFound = emberAfPluginTouCalendarCommonSetLocalCalendarEntry(i,
                                                                          &calendar);
      if (!calendarFound) {
        emberAfTouCalendarClusterPrintln("Could not initialize local calendar entry at index %d.", i);
      }
    }
  }
}

boolean emberAfTouCalendarClusterGetCalendarCallback(int32u earliestStartTime,
                                                     int32u minIssuerEventId,
                                                     int8u numberOfCalendars,
                                                     int8u calendarType,
                                                     int32u providerId)
{
  EmberAfTouCalendarStruct calendar;
  int8u numberOfCalendarsSent = 0;
  int8u i;

  while (numberOfCalendars == 0 || numberOfCalendarsSent < numberOfCalendars) {
    int32u referenceUtc = MAX_INT32U_VALUE;
    int8u indexToSend = 0xFF;
    int8u i;

    // Find active or scheduled calendars matching the filter fields in the
    // request that have not been sent out yet.  Of those, find the one that
    // starts the earliest.
    for (i = 0; i < EMBER_AF_PLUGIN_TOU_CALENDAR_COMMON_TOTAL_CALENDARS; i++) {
      if (emberAfPluginTouCalendarCommonGetLocalCalendar(i, &calendar)
          && !READBITS(calendar.flags, EMBER_AF_PLUGIN_TOU_CALENDAR_COMMON_FLAGS_SENT)
          && (minIssuerEventId == EMBER_AF_PLUGIN_TOU_CALENDAR_COMMON_WILDCARD_ISSUER_ID
              || minIssuerEventId <= calendar.issuerEventId)
          && (calendarType == EMBER_AF_PLUGIN_TOU_CALENDAR_COMMON_WILDCARD_CALENDAR_TYPE
              || calendarType == calendar.calendarType)
          && (providerId == EMBER_AF_PLUGIN_TOU_CALENDAR_COMMON_WILDCARD_PROVIDER_ID
              || providerId == calendar.providerId)
          && earliestStartTime < emberAfPluginTouCalendarCommonEndTimeUtc(&calendar)
          && calendar.startTimeUtc < referenceUtc) {
        referenceUtc = calendar.startTimeUtc;
        indexToSend = i;
      }
    }

    // If no active or scheduled calendar were found, it either means there are
    // no active or scheduled calendars at the specified time or we've already
    // found all of them in previous iterations.  If we did find one, we send
    // it, mark it as sent, and move on.
    if (indexToSend == 0xFF) {
      break;
    } else {
      emberAfPluginTouCalendarCommonGetLocalCalendar(indexToSend, &calendar);
      emberAfFillCommandTouCalendarClusterPublishCalendar(calendar.providerId,
                                                          calendar.issuerEventId,
                                                          calendar.calendarId,
                                                          calendar.startTimeUtc,
                                                          calendar.calendarType,
                                                          calendar.name,
                                                          calendar.numberOfSeasons,
                                                          calendar.numberOfWeekProfiles,
                                                          calendar.numberOfDayProfiles);
      emberAfSendResponse();
      numberOfCalendarsSent++;

      SETBITS(calendar.flags, EMBER_AF_PLUGIN_TOU_CALENDAR_COMMON_FLAGS_SENT);
      emberAfPluginTouCalendarCommonSetLocalCalendarEntry(indexToSend,
                                                          &calendar);
    }
  }

  // If we sent nothing, we return an error.  Otherwise, we need to roll
  // through all the calendars and clear the sent bit.
  if (numberOfCalendarsSent == 0) {
    emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_NOT_FOUND);
  } else {
    for (i = 0; i < EMBER_AF_PLUGIN_TOU_CALENDAR_COMMON_TOTAL_CALENDARS; i++) {
      if (emberAfPluginTouCalendarCommonGetLocalCalendar(i, &calendar)
          && READBITS(calendar.flags, EMBER_AF_PLUGIN_TOU_CALENDAR_COMMON_FLAGS_SENT)) {
        CLEARBITS(calendar.flags, EMBER_AF_PLUGIN_TOU_CALENDAR_COMMON_FLAGS_SENT);
        emberAfPluginTouCalendarCommonSetLocalCalendarEntry(i, &calendar);
      }
    }
  }

  return TRUE;
}

boolean emberAfTouCalendarClusterGetDayProfilesCallback(int32u providerId,
                                                        int32u issuerCalendarId,
                                                        int8u startDayId,
                                                        int8u numberOfDays)
{
  int8u scheduleEntriesData[SCHEDULE_ENTRY_SIZE
                            * EMBER_AF_PLUGIN_TOU_CALENDAR_COMMON_SCHEDULE_ENTRIES_MAX];

  int8u i, totalDays;
  EmberAfTouCalendarStruct calendar;
  boolean calendarFound = emberAfPluginTouCalendarCommonGetCalendarById(issuerCalendarId,
                                                                        providerId,
                                                                        &calendar);
  if (!calendarFound) {
    emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_NOT_FOUND);
    return TRUE;
  }

  if (startDayId == 0) {
    emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_INVALID_FIELD);
    return TRUE;
  }

  if (calendar.numberOfDayProfiles < startDayId) {
    emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_NOT_FOUND);
    return TRUE;
  }

  totalDays = calendar.numberOfDayProfiles - startDayId + 1;
  if (numberOfDays != 0 && numberOfDays < totalDays) {
    totalDays = numberOfDays;
  }

  for (i = 0; i < totalDays; i++) {
    int8u dayId = startDayId + i;
    int8u dayIndex = dayId - 1;
    int8u j;
    for (j = 0; j < calendar.normalDays[dayIndex].numberOfScheduleEntries; j++) {
      emberAfCopyInt16u(scheduleEntriesData,
                        j * SCHEDULE_ENTRY_SIZE,
                        calendar.normalDays[dayIndex].scheduleEntries[j].minutesFromMidnight);
      scheduleEntriesData[j * SCHEDULE_ENTRY_SIZE + 2] = calendar.normalDays[dayIndex].scheduleEntries[j].data;
    }
    emberAfFillCommandTouCalendarClusterPublishDayProfile(calendar.providerId,
                                                          calendar.issuerEventId,
                                                          calendar.calendarId,
                                                          calendar.normalDays[dayIndex].id,
                                                          calendar.normalDays[dayIndex].numberOfScheduleEntries,
                                                          0, // command-index
                                                          1, // total commands
                                                          calendar.calendarType,
                                                          scheduleEntriesData,
                                                          SCHEDULE_ENTRY_SIZE * calendar.normalDays[dayIndex].numberOfScheduleEntries);
    emberAfSendResponse();
  }

  return TRUE;
}

boolean emberAfTouCalendarClusterGetWeekProfilesCallback(int32u providerId,
                                                         int32u issuerCalendarId,
                                                         int8u startWeekId,
                                                         int8u numberOfWeeks)
{
  int8u i, totalWeeks;
  EmberAfTouCalendarStruct calendar;
  boolean calendarFound = emberAfPluginTouCalendarCommonGetCalendarById(issuerCalendarId,
                                                                        providerId,
                                                                        &calendar);
  if (!calendarFound) {
    emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_NOT_FOUND);
    return TRUE;
  }

  if (startWeekId == 0) {
    emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_INVALID_FIELD);
    return TRUE;
  }

  if (calendar.numberOfWeekProfiles < startWeekId) {
    emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_NOT_FOUND);
    return TRUE;
  }

  totalWeeks = calendar.numberOfWeekProfiles - startWeekId + 1;
  if (numberOfWeeks != 0 && numberOfWeeks < totalWeeks) {
    totalWeeks = numberOfWeeks;
  }

  for (i = 0; i < totalWeeks; i++) {
    int8u weekId = startWeekId + i;
    int8u weekIndex = weekId - 1;
    emberAfFillCommandTouCalendarClusterPublishWeekProfile(calendar.providerId,
                                                           calendar.issuerEventId,
                                                           calendar.calendarId,
                                                           weekId,
                                                           calendar.normalDays[calendar.weeks[weekIndex].normalDayIndexes[0]].id,
                                                           calendar.normalDays[calendar.weeks[weekIndex].normalDayIndexes[1]].id,
                                                           calendar.normalDays[calendar.weeks[weekIndex].normalDayIndexes[2]].id,
                                                           calendar.normalDays[calendar.weeks[weekIndex].normalDayIndexes[3]].id,
                                                           calendar.normalDays[calendar.weeks[weekIndex].normalDayIndexes[4]].id,
                                                           calendar.normalDays[calendar.weeks[weekIndex].normalDayIndexes[5]].id,
                                                           calendar.normalDays[calendar.weeks[weekIndex].normalDayIndexes[6]].id);
    emberAfSendResponse();
  }

  return TRUE;
}

boolean emberAfTouCalendarClusterGetSeasonsCallback(int32u providerId,
                                                    int32u issuerCalendarId)
{
  int8u i;
  int8u seasonData[SEASON_ENTRY_SIZE * EMBER_AF_PLUGIN_TOU_CALENDAR_COMMON_SEASON_PROFILE_MAX];
  EmberAfTouCalendarStruct calendar;
  boolean calendarFound = emberAfPluginTouCalendarCommonGetCalendarById(issuerCalendarId,
                                                                        providerId,
                                                                        &calendar);
  if (!calendarFound) {
    emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_NOT_FOUND);
    return TRUE;
  }

  for (i = 0; i < calendar.numberOfSeasons; i++) {
    emberAfCopyInt32u(seasonData,
                      i * SEASON_ENTRY_SIZE,
                      calendar.seasons[i].startDate);
    emberAfCopyInt8u(seasonData,
                     i * SEASON_ENTRY_SIZE + 4,
                     calendar.weeks[calendar.seasons[i].weekIndex].id);
  }
  // For now we don't support segmenting commands since it isn't clear in the 
  // spec how this is done.  APS Fragmentation would be better since it is
  // already used by other clusters.
  emberAfFillCommandTouCalendarClusterPublishSeasons(calendar.providerId,
                                                     calendar.issuerEventId,
                                                     calendar.calendarId,
                                                     0, // command index
                                                     1, // total commands
                                                     seasonData,
                                                     (calendar.numberOfSeasons
                                                      * SEASON_ENTRY_SIZE));

  emberAfSendResponse();
  return TRUE;
}

boolean emberAfTouCalendarClusterGetSpecialDaysCallback(int32u startTime,
                                                        int8u numberOfEvents,
                                                        int8u calendarType,
                                                        int32u providerId,
                                                        int32u issuerCalendarId)
{
  EmberAfTouCalendarStruct calendar;
  int8u numberOfSpecialDaysSent = 0;
  int8u i;

  // TODO-SPEC: 12-0517-11 says that a start time of zero means now, but this
  // is apparently going away.  See comment TE6-7 in 13-0546-06 and Ian
  // Winterburn's email to zigbee_pro_energy@mail.zigbee.org on March 25, 2014.
  //if (startTime == 0) {
  //  startTime = emberAfGetCurrentTime();
  //}

  while (numberOfEvents == 0 || numberOfSpecialDaysSent < numberOfEvents) {
    int32u referenceUtc = MAX_INT32U_VALUE;
    int8u indexToSend = 0xFF;
    int8u i;

    // Find active or scheduled calendars matching the filter fields in the
    // request that have not been sent out yet.  Of those, find the one that
    // starts the earliest.
    for (i = 0; i < EMBER_AF_PLUGIN_TOU_CALENDAR_COMMON_TOTAL_CALENDARS; i++) {
      if (emberAfPluginTouCalendarCommonGetLocalCalendar(i, &calendar)
          && !READBITS(calendar.flags, EMBER_AF_PLUGIN_TOU_CALENDAR_COMMON_FLAGS_SENT)
          && (calendarType == EMBER_AF_PLUGIN_TOU_CALENDAR_COMMON_WILDCARD_CALENDAR_TYPE
              || calendarType == calendar.calendarType)
          && (providerId == EMBER_AF_PLUGIN_TOU_CALENDAR_COMMON_WILDCARD_PROVIDER_ID
              || providerId == calendar.providerId)
          && (issuerCalendarId == EMBER_AF_PLUGIN_TOU_CALENDAR_COMMON_WILDCARD_CALENDAR_ID
              || issuerCalendarId == calendar.calendarId)
          && startTime < emberAfPluginTouCalendarCommonEndTimeUtc(&calendar)
          && calendar.startTimeUtc < referenceUtc) {
        referenceUtc = calendar.startTimeUtc;
        indexToSend = i;
      }
    }

    // If no active or scheduled calendar were found, it either means there are
    // no active or scheduled calendars at the specified time or we've already
    // found all of them in previous iterations.  If we did find one, we need
    // to look at, and maybe send, its special days before we move on.
    if (indexToSend == 0xFF) {
      break;
    } else {
      EmberAfTimeStruct time;
      int8u specialDayData[EMBER_AF_PLUGIN_TOU_CALENDAR_COMMON_SPECIAL_DAY_PROFILE_MAX
                           * SPECIAL_DAY_ENTRY_SIZE];
      int8u totalNumberOfSpecialDays = 0;
      int32u startDate;

      emberAfFillTimeStructFromUtc(startTime, &time);
      startDate = ((((int32u)(time.year - 1900)) << 24)
                   + (((int32u)time.month) << 16)
                   + (((int32u)time.day) << 8)
                   + 0xFF);

      emberAfPluginTouCalendarCommonGetLocalCalendar(indexToSend, &calendar);

      // Find active or scheduled special days that have not been added to the
      // payload.  Of those, find the one that starts the earliest.
      for (;;) {
        int32u referenceDate = MAX_INT32U_VALUE;
        int8u indexToAdd = 0xFF;
        for (i = 0; i < calendar.numberOfSpecialDayProfiles; i++) {
          if (!READBITS(calendar.specialDays[i].flags, EMBER_AF_PLUGIN_TOU_CALENDAR_COMMON_FLAGS_SENT)
              && startDate < emberAfPluginTouCalendarCommonEndDate(&calendar, &calendar.specialDays[i])
              && calendar.specialDays[i].startDate < referenceDate) {
            referenceDate = calendar.specialDays[i].startDate;
            indexToAdd = i;
          }
        }

        // If no active or scheduled special days were found, it either means
        // there are no active or scheduled special days at the specified time
        // or we've already found all of them in previous iterations.  If we
        // did find one, we add it, mark it as added, and move on.
        if (indexToAdd == 0xFF) {
          break;
        } else {
          emberAfCopyInt32u(specialDayData,
                            totalNumberOfSpecialDays * SPECIAL_DAY_ENTRY_SIZE,
                            calendar.specialDays[indexToAdd].startDate),
          emberAfCopyInt8u(specialDayData,
                           totalNumberOfSpecialDays * SPECIAL_DAY_ENTRY_SIZE + 4,
                           calendar.normalDays[calendar.specialDays[indexToAdd].normalDayIndex].id);
          totalNumberOfSpecialDays++;

          SETBITS(calendar.specialDays[indexToAdd].flags, EMBER_AF_PLUGIN_TOU_CALENDAR_COMMON_FLAGS_SENT);
        }
      }

      if (totalNumberOfSpecialDays != 0) {
        emberAfFillCommandTouCalendarClusterPublishSpecialDays(calendar.providerId,
                                                               calendar.issuerEventId,
                                                               calendar.calendarId,
                                                               calendar.startTimeUtc,
                                                               calendar.calendarType,
                                                               totalNumberOfSpecialDays,
                                                               0, // command index
                                                               1, // total commands
                                                               specialDayData,
                                                               (totalNumberOfSpecialDays
                                                                * SPECIAL_DAY_ENTRY_SIZE));
        emberAfSendResponse();
        numberOfSpecialDaysSent++;
      }

      SETBITS(calendar.flags, EMBER_AF_PLUGIN_TOU_CALENDAR_COMMON_FLAGS_SENT);
      emberAfPluginTouCalendarCommonSetLocalCalendarEntry(indexToSend,
                                                          &calendar);
    }
  }

  // If we sent nothing, we return an error.  Otherwise, we need to roll
  // through all the calendars and clear the sent bit and roll through all the
  // special days and clear the sent it.
  if (numberOfEvents == 0) {
    emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_NOT_FOUND);
  } else {
    for (i = 0; i < EMBER_AF_PLUGIN_TOU_CALENDAR_COMMON_TOTAL_CALENDARS; i++) {
      if (emberAfPluginTouCalendarCommonGetLocalCalendar(i, &calendar)
          && READBITS(calendar.flags, EMBER_AF_PLUGIN_TOU_CALENDAR_COMMON_FLAGS_SENT)) {
        int8u j;
        CLEARBITS(calendar.flags, EMBER_AF_PLUGIN_TOU_CALENDAR_COMMON_FLAGS_SENT);
        for (j = 0; j < calendar.numberOfSpecialDayProfiles; j++) {
          if (READBITS(calendar.specialDays[j].flags, EMBER_AF_PLUGIN_TOU_CALENDAR_COMMON_FLAGS_SENT)) {
            CLEARBITS(calendar.specialDays[j].flags, EMBER_AF_PLUGIN_TOU_CALENDAR_COMMON_FLAGS_SENT);
          }
        }
        emberAfPluginTouCalendarCommonSetLocalCalendarEntry(i, &calendar);
      }
    }
  }

  return TRUE;
}
