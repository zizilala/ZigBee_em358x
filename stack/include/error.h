/**
 * @file error.h
 * @brief Return codes for Ember API functions and module definitions.
 *
 * See @ref status_codes for documentation.
 * 
 * <!--Author(s): Lee Taylor, lee@ember.com -->
 * <!--Copyright 2004 by Ember Corporation. All rights reserved.-->
 */

#ifndef __ERRORS_H__
#define __ERRORS_H__

/**
 * @brief  Return type for Ember functions.
 */
#ifndef __EMBERSTATUS_TYPE__
#define __EMBERSTATUS_TYPE__
  typedef int8u EmberStatus;
#endif //__EMBERSTATUS_TYPE__

/**
 * @addtogroup status_codes
 * @{
 */

/**
 * @brief Macro used by error-def.h to define all of the return codes.
 *
 * @param symbol  The name of the constant being defined. All Ember returns
 *                begin with EMBER_. For example, ::EMBER_CONNECTION_OPEN.
 * 
 * @param value   The value of the return code. For example, 0x61.
 */
#define DEFINE_ERROR(symbol, value) \
 EMBER_ ## symbol = value,


enum {
#ifndef DOXYGEN_SHOULD_SKIP_THIS
#include "include/error-def.h"
#endif //DOXYGEN_SHOULD_SKIP_THIS
  /** Gets defined as a count of all the possible return codes in the 
  * EmberZNet stack API. 
  */
  EMBER_ERROR_CODE_COUNT

};

#undef DEFINE_ERROR

#endif // __ERRORS_H__

/**@} // End of addtogroup
 */
 

