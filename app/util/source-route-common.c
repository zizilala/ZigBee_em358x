// File: source-route-common.c
//
// Description: Common code used for managing source routes on both node-based
// and host-based gateways. See source-route.c for node-based gateways and
// source-route-host.c for host-based gateways.
// 
// Copyright 2007 by Ember Corporation. All rights reserved.                *80*


#include PLATFORM_HEADER
#include "stack/include/ember-types.h"
#include "source-route-common.h"

// The number of entries in use.
static int8u entryCount = 0;

// The index of the most recently added entry.
static int8u newestIndex = NULL_INDEX;

// Return the index of the entry with the specified destination.
int8u sourceRouteFindIndex(EmberNodeId id)
{
  int8u i;
  for (i = 0; i < entryCount; i++) {
    if (sourceRouteTable[i].destination == id) {
      return i;
    }
  }
  return NULL_INDEX;
}

void sourceRouteClearTable(void)
{
  entryCount = 0;
  MEMSET(sourceRouteTable, 
         0, 
         sizeof(SourceRouteTableEntry) * sourceRouteTableSize);
}

// Create an entry with the given id or update an existing entry. furtherIndex
// is the entry one hop further from the gateway.
int8u sourceRouteAddEntry(EmberNodeId id, int8u furtherIndex)
{
  // See if the id already exists in the table.
  int8u index = sourceRouteFindIndex(id);
  int8u i;

  if (index == NULL_INDEX) {
    if (entryCount < sourceRouteTableSize) {
      // No existing entry. Table is not full. Add new entry.
      index = entryCount;
      entryCount += 1;
    } else {
      // No existing entry. Table is full. Replace oldest entry.
      index = newestIndex;
      while (sourceRouteTable[index].olderIndex != NULL_INDEX) {
        index = sourceRouteTable[index].olderIndex;
      }
    }
  }

  // Update the pointers (only) if something has changed.
  if (index != newestIndex) {
    for (i = 0; i < entryCount; i++) {
      if (sourceRouteTable[i].olderIndex == index) {
        sourceRouteTable[i].olderIndex = sourceRouteTable[index].olderIndex;
        break;
      }
    }
    sourceRouteTable[index].olderIndex = newestIndex;
    newestIndex = index;
  }

  // Add the entry.
  sourceRouteTable[index].destination = id;
  sourceRouteTable[index].closerIndex = NULL_INDEX;


  // The current index is one hop closer to the gateway than furtherIndex.
  if (furtherIndex != NULL_INDEX) {
    sourceRouteTable[furtherIndex].closerIndex = index;
  }

  // Return the current index to save the caller having to look it up. 
  return index;
}

int8u sourceRouteAddEntryWithCloserNextHop(EmberNodeId newId,
                                           EmberNodeId closerNodeId)
{
  int8u closerIndex = sourceRouteFindIndex(closerNodeId);
  if (closerIndex != NULL_INDEX) {
    int8u index = sourceRouteAddEntry(newId, NULL_INDEX);
    if (index != NULL_INDEX) {
      sourceRouteTable[index].closerIndex = closerIndex;
    }
    return index;
  }
  return NULL_INDEX;
}

void sourceRouteInit(void)
{
  entryCount = 0;
}

int8u sourceRouteGetCount(void)
{
  return entryCount;
}

int8u emberGetSourceRouteOverhead(EmberNodeId destination)
{
  int8u overhead = 0;
  int8u index = sourceRouteFindIndex(destination);
  if (index != NULL_INDEX) {
    // The additional overhead required for network source routing (relay
    // count = 1, relay index = 1).  This does not include the size of the
    // relay list itself.
    overhead = 2;
    while (sourceRouteTable[index].closerIndex != NULL_INDEX) {
      // The additional overhead required per relay address for network
      // source routing.  This is in addition to the overhead included above.
      overhead += 2;
      index = sourceRouteTable[index].closerIndex;
    }
  }
  return overhead;
}
