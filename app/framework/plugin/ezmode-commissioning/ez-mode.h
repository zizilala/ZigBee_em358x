// Copyright 2013 Silicon Laboratories, Inc.

typedef enum {
  /* allows the creation of bindings from
   * an attribute server to an attribute client. */
  EMBER_AF_EZMODE_COMMISSIONING_SERVER_TO_CLIENT,

  /* allows the creation of bindings from
   * an attribute client to an attribute server. */
  EMBER_AF_EZMODE_COMMISSIONING_CLIENT_TO_SERVER,
} EmberAfEzModeCommissioningDirection;

/**
 * @brief Starts EZ-Mode Client commissioning
 * 
 * Kicks off the ezmode commissioning process by sending out
 * an identify query command to the given endpoint. Endpoints that
 * return an identify query response are interrogated for the given
 * cluster ids in the given direction (client or server).
 *
 * @param endpoint The endpoint to send the identify query command from
 * @param direction The side of the cluster ids given either client or server
 * @param clusterIds An array of clusters against which to match. 
 *        *NOTE* The API only keeps the pointer to
 *        to the data structure. The data is expected to exist throughout the
 *        ezmode-commissioning calls.
 * @param clusterIdsLength The number of cluster ids passed for the match
 */
EmberStatus emberAfEzmodeClientCommission(int8u endpoint,
                                          EmberAfEzModeCommissioningDirection direction,
                                          int16u *clusterIds,
                                          int8u clusterIdsLength);

/**
 * @brief Begins EZ-Mode server commissioning
 *
 * Kicks of the server side of EZ-Mode commissioning by putting the 
 * device into identify mode.
 *
 * @param endpoint The endpoint on which to begin identifying
 */
EmberStatus emberAfEzmodeServerCommission(int8u endpoint);

/**
 * @brief Begins EZ-Mode server commissioning with a given timeout
 *
 * Kicks of the server side of EZ-Mode commissioning by putting the 
 * device into identify mode for a given time.
 *
 * @param endpoint The endpoint on which to begin identifying
 * @param identifyTimeoutSeconds The number of seconds to identify for before
 *                               stopping identify mode.
 */
EmberStatus emberAfEzmodeServerCommissionWithTimeout(int8u endpoint,
                                                     int16u identifyTimeoutSeconds);
