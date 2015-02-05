// *******************************************************************
// * reporting.h
// *
// *
// * Copyright 2011 by Ember Corporation. All rights reserved.              *80*
// *******************************************************************

EmberAfStatus emberAfPluginReportingConfigureReportedAttribute(const EmberAfPluginReportingEntry *newEntry);
void emAfPluginReportingGetEntry(int8u index, EmberAfPluginReportingEntry *result);
void emAfPluginReportingSetEntry(int8u index, EmberAfPluginReportingEntry *value);
EmberStatus emAfPluginReportingRemoveEntry(int8u index);
