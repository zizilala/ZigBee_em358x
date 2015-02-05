/** @file hal/micro/cortexm3/memmap.h
 *
 * @brief
 * Memory map definitions
 *
 * THIS IS A GENERATED FILE.  DO NOT EDIT.
 *
 * <!-- Copyright 2014 Silicon Laboratories, Inc.                        *80*-->
 */

#ifndef __MEMMAP_H__
#define __MEMMAP_H__

#if defined(CORTEXM3_EM351)
  #include "hal/micro/cortexm3/em35x/em351/memmap.h"
#elif defined(CORTEXM3_EM356)
  #include "hal/micro/cortexm3/em35x/em356/memmap.h"
#elif defined(CORTEXM3_EM357) || defined(EMBER_TEST)
  #include "hal/micro/cortexm3/em35x/em357/memmap.h"
#elif defined(CORTEXM3_EM3581)
  #include "hal/micro/cortexm3/em35x/em3581/memmap.h"
#elif defined(CORTEXM3_EM3582)
  #include "hal/micro/cortexm3/em35x/em3582/memmap.h"
#elif defined(CORTEXM3_EM3585)
  #include "hal/micro/cortexm3/em35x/em3585/memmap.h"
#elif defined(CORTEXM3_EM3586)
  #include "hal/micro/cortexm3/em35x/em3586/memmap.h"
#elif defined(CORTEXM3_EM3587)
  #include "hal/micro/cortexm3/em35x/em3587/memmap.h"
#elif defined(CORTEXM3_EM3588)
  #include "hal/micro/cortexm3/em35x/em3588/memmap.h"
#elif defined(CORTEXM3_EM359)
  #include "hal/micro/cortexm3/em35x/em359/memmap.h"
#elif defined(CORTEXM3_EM3591)
  #include "hal/micro/cortexm3/em35x/em3591/memmap.h"
#elif defined(CORTEXM3_EM3592)
  #include "hal/micro/cortexm3/em35x/em3592/memmap.h"
#elif defined(CORTEXM3_EM3595)
  #include "hal/micro/cortexm3/em35x/em3595/memmap.h"
#elif defined(CORTEXM3_EM3596)
  #include "hal/micro/cortexm3/em35x/em3596/memmap.h"
#elif defined(CORTEXM3_EM3597)
  #include "hal/micro/cortexm3/em35x/em3597/memmap.h"
#elif defined(CORTEXM3_EM3598)
  #include "hal/micro/cortexm3/em35x/em3598/memmap.h"
#elif defined(CORTEXM3_SKY66107)
  #include "hal/micro/cortexm3/em35x/sky66107/memmap.h"
#endif

//=============================================================================
// A union that describes the entries of the vector table.  The union is needed
// since the first entry is the stack pointer and the remainder are function
// pointers.
//
// Normally the vectorTable below would require entries such as:
//   { .topOfStack = x },
//   { .ptrToHandler = y },
// But since ptrToHandler is defined first in the union, this is the default
// type which means we don't need to use the full, explicit entry.  This makes
// the vector table easier to read because it's simply a list of the handler
// functions.  topOfStack, though, is the second definition in the union so
// the full entry must be used in the vectorTable.
//=============================================================================
typedef union
{
  void (*ptrToHandler)(void);
  void *topOfStack;
} HalVectorTableType;


// ****************************************************************************
// If any of these address table definitions ever need to change, it is highly
// desirable to only add new entries, and only add them on to the end of an
// existing address table... this will provide the best compatibility with
// any existing code which may utilize the tables, and which may not be able to 
// be updated to understand a new format (example: bootloader which reads the 
// application address table)

// Generic Address table definition which describes leading fields which
// are common to all address table types
typedef struct {
  void *topOfStack;
  void (*resetVector)(void);
  void (*nmiHandler)(void);
  void (*hardFaultHandler)(void);
  int16u type;
  int16u version;
  const HalVectorTableType *vectorTable;
  // Followed by more fields depending on the specific address table type
} HalBaseAddressTableType;

#ifdef MINIMAL_HAL
  // Minimal hal only references the FAT
  #include "memmap-fat.h"
#else
  // Full hal references all address tables
  #include "memmap-tables.h"
#endif

#endif // __MEMMAP_H__
