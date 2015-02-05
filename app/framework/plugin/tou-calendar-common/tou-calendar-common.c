// ****************************************************************************
// * tou-calendar-common.c
// *
// *
// * Copyright 2014 Silicon Laboratories, Inc.                              *80*
// ****************************************************************************

#include "app/framework/include/af.h"
#include "tou-calendar-common.h"

//-----------------------------------------------------------------------------
// Globals

static EmberAfTouCalendarStruct calendars[EMBER_AF_PLUGIN_TOU_CALENDAR_COMMON_TOTAL_CALENDARS];

//-----------------------------------------------------------------------------

void emberAfPluginTouCalendarCommonInitCallback(int8u endpoint)
{
  int8u i;
  for (i = 0; i < EMBER_AF_PLUGIN_TOU_CALENDAR_COMMON_TOTAL_CALENDARS; i++) {
    MEMSET(&(calendars[i]), 0, sizeof(EmberAfTouCalendarStruct));
    calendars[i].calendarId = EMBER_AF_PLUGIN_TOU_CALENDAR_COMMON_INVALID_CALENDAR_ID;
  }
}

boolean emberAfPluginTouCalendarCommonGetLocalCalendar(int8u index,
                                                       EmberAfTouCalendarStruct *calendar)
{
  if (index < EMBER_AF_PLUGIN_TOU_CALENDAR_COMMON_TOTAL_CALENDARS) {
    MEMCOPY(calendar, &(calendars[index]), sizeof(EmberAfTouCalendarStruct));
    return TRUE;
  }
  return FALSE;
}

boolean emberAfPluginTouCalendarCommonGetCalendarById(int32u calendarId,
                                                      int32u providerId,
                                                      EmberAfTouCalendarStruct *calendar)
{
  int8u i;
  for (i = 0; i < EMBER_AF_PLUGIN_TOU_CALENDAR_COMMON_TOTAL_CALENDARS; i++) {
    if (calendarId == calendars[i].calendarId
        && (providerId == EMBER_AF_PLUGIN_TOU_CALENDAR_COMMON_WILDCARD_PROVIDER_ID
            || providerId == calendars[i].providerId)) {
      MEMCOPY(calendar, &(calendars[i]), sizeof(EmberAfTouCalendarStruct));
      return TRUE;
    }
  }
  return FALSE;
}

boolean emberAfPluginTouCalendarCommonSetLocalCalendarEntry(int8u index,
                                                            EmberAfTouCalendarStruct *calendar)
{
  if (index < EMBER_AF_PLUGIN_TOU_CALENDAR_COMMON_TOTAL_CALENDARS) {
    MEMCOPY(&(calendars[index]), calendar, sizeof(EmberAfTouCalendarStruct));
    return TRUE;
  }
  return FALSE;
}

int32u emberAfPluginTouCalendarCommonEndTimeUtc(const EmberAfTouCalendarStruct *calendar)
{
  EmberAfTouCalendarStruct reference;
  int32u endTimeUtc = MAX_INT32U_VALUE;
  int8u i;
  for (i = 0; i < EMBER_AF_PLUGIN_TOU_CALENDAR_COMMON_TOTAL_CALENDARS; i++) {
    if (emberAfPluginTouCalendarCommonGetLocalCalendar(i, &reference)
        && calendar->providerId == reference.providerId
        && calendar->issuerEventId < reference.issuerEventId
        && calendar->startTimeUtc < reference.startTimeUtc
        && calendar->calendarType == reference.calendarType
        && reference.startTimeUtc < endTimeUtc) {
      endTimeUtc = reference.startTimeUtc;
    }
  }

  return endTimeUtc;
}

int32u emberAfPluginTouCalendarCommonEndDate(const EmberAfTouCalendarStruct *calendar,
                                             const EmberAfTouCalendarSpecialDayStruct *specialDay)
{
  int32u endDate = MAX_INT32U_VALUE;
  int8u i;
  for (i = 0; i < EMBER_AF_PLUGIN_TOU_CALENDAR_COMMON_SPECIAL_DAY_PROFILE_MAX; i++) {
    if (specialDay->startDate < calendar->specialDays[i].startDate
        && calendar->specialDays[i].startDate < endDate) {
      endDate = calendar->specialDays[i].startDate;
    }
  }
  return endDate;
}
