/**
 * @file af-types.h
 * @brief The include file for all the types for Ember ApplicationFramework
 *
 * <!--Copyright 2014 Silicon Laboratories, Inc.                         *80*-->
 */

/** @addtogroup aftypes Application Framework V2 Types Reference
 * This documentation describes the types used by the Ember
 * Application Framework V2.
 * @{
 */


#ifndef __AF_API_TYPES__
#define __AF_API_TYPES__

#include "enums.h"

/**
 * @brief Type for referring to ZigBee application profile id
 */
typedef int16u EmberAfProfileId;

/**
 * @brief Type for referring to ZCL attribute id
 */
typedef int16u EmberAfAttributeId;

/**
 * @brief Type for referring to ZCL cluster id
 */
typedef int16u EmberAfClusterId;

/**
 * @brief Type for referring to ZCL attribute type
 */
typedef int8u EmberAfAttributeType;

/**
 * @brief Type for the cluster mask
 */
typedef int8u EmberAfClusterMask;

/**
 * @brief Type for the attribute mask
 */
typedef int8u EmberAfAttributeMask;

/**
 * @brief Generic function type, used for either of the cluster function.
 *
 * This type is used for the array of the cluster functions, and should
 * always be cast into one of the specific functions before being called.
 */
typedef void (*EmberAfGenericClusterFunction)(void);

/**
 * @brief A distinguished manufacturer code that is used to indicate the
 * absence of a manufacturer-specific profile, cluster, command, or attribute.
 */
#define EMBER_AF_NULL_MANUFACTURER_CODE 0x0000

/**
 * @brief Type for default values.
 *
 * Default value is either a value itself, if it is 2 bytes or less,
 * or a pointer to the value itself, if attribute type is longer than
 * 2 bytes.
 */
typedef union {
  /**
   * Points to data if size is more than 2 bytes.
   * If size is more than 2 bytes, and this value is NULL,
   * then the default value is all zeroes.
   */
  int8u *ptrToDefaultValue;
  /**
   * Actual default value if the attribute size is 2 bytes or less.
   */
  int16u defaultValue;
} EmberAfDefaultAttributeValue;

/**
 * @brief Type describing the attribute default, min and max values.
 *
 * This struct is required if the attribute mask specifies that this
 * attribute has a known min and max values.
 */
typedef struct {
  /**
   * Default value of the attribute.
   */
  EmberAfDefaultAttributeValue defaultValue;
  /**
   * Minimum allowed value
   */
  EmberAfDefaultAttributeValue minValue;
  /**
   * Maximum allowed value.
   */
  EmberAfDefaultAttributeValue maxValue;
} EmberAfAttributeMinMaxValue;

/**
 * @brief Union describing the attribute default/min/max values.
 */
typedef union {
  /**
   * Points to data if size is more than 2 bytes.
   * If size is more than 2 bytes, and this value is NULL,
   * then the default value is all zeroes.
   */
  int8u *ptrToDefaultValue;
  /**
   * Actual default value if the attribute size is 2 bytes or less.
   */
  int16u defaultValue;
  /**
   * Points to the min max attribute value structure, if min/max is
   * supported for this attribute.
   */
  EmberAfAttributeMinMaxValue *ptrToMinMaxValue;
} EmberAfDefaultOrMinMaxAttributeValue;


/**
 * @brief Each attribute has it's metadata stored in such struct.
 *
 * There is only one of these per attribute across all endpoints.
 */
typedef struct {
  /**
   * Attribute ID, according to ZCL specs.
   */
  EmberAfAttributeId attributeId;
  /**
   * Attribute type, according to ZCL specs.
   */
  EmberAfAttributeType attributeType;
  /**
   * Size of this attribute in bytes.
   */
  int8u size;
  /**
   * Attribute mask, tagging attribute with specific
   * functionality. See ATTRIBUTE_MASK_ macros defined
   * in att-storage.h.
   */
  EmberAfAttributeMask mask;
  /**
   * Pointer to the default value union. Actual value stored
   * depends on the mask.
   */
  EmberAfDefaultOrMinMaxAttributeValue  defaultValue;
} EmberAfAttributeMetadata;

/**
 * @brief Struct describing cluster
 */
typedef struct {
  /**
   *  ID of cluster according to ZCL spec
   */
  EmberAfClusterId clusterId;
  /**
   * Pointer to attribute metadata array for this cluster.
   */
  EmberAfAttributeMetadata *attributes;
  /**
   * Total number of attributes
   */
  int16u attributeCount;
  /**
   * Total size of non-external, non-singleton attribute for this cluster.
   */
  int16u clusterSize;
  /**
   * Mask with additional functionality for cluster. See CLUSTER_MASK
   * macros.
   */
  EmberAfClusterMask mask;

  /**
   * An array into the cluster functions. The length of the array
   * is determined by the function bits in mask. This may be null
   * if this cluster has no functions.
   */
  const EmberAfGenericClusterFunction *functions;

} EmberAfCluster;

/**
 * @brief Struct used to find an attribute in storage. Together the elements
 * in this search record constitute the "primary key" used to identify a unique
 * attribute value in attribute storage.
 */
typedef struct {

  /**
  * Endpoint that the attribute is located on
  */
  int8u endpoint;

  /**
  * Cluster that the attribute is located on. If the cluster
  * id is inside the manufacturer specific range, 0xfc00 - 0xffff,
  * The manufacturer code should also be set to the code associated
  * with the manufacturer specific cluster.
  */
  EmberAfClusterId clusterId;

  /**
  * Cluster mask for the cluster, used to determine if it is
  * the server or client version of the cluster. See CLUSTER_MASK_
  * macros defined in att-storage.h
  */
  EmberAfClusterMask clusterMask;

  /**
  * The two byte identifier for the attribute. If the cluster id is
  * inside the manufacturer specific range 0xfc00 - 0xffff, or the manufacturer
  * code is NOT 0, the attribute is assumed to be manufacturer specific.
  */
  EmberAfAttributeId attributeId;

  /**
   * Manufacturer Code associated with the cluster and or attribute.
   * If the cluster id is inside the manufacturer specific
   * range, this value should indicate the manufacturer code for the
   * manufacturer specific cluster. Otherwise if this value is non zero,
   * and the cluster id is a standard ZCL cluster,
   * it is assumed that the attribute being sought is a manufacturer specific
   * extension to the standard ZCL cluster indicated by the cluster id.
   */
  int16u manufacturerCode;
} EmberAfAttributeSearchRecord;

/**
 * A struct used to construct a table of manufacturer codes for
 * manufacturer specfic attributes and clusters.
 */
typedef struct {
  int16u index;
  int16u manufacturerCode;
} EmberAfManufacturerCodeEntry;

/**
 * @brief a struct containing the superset of values
 * passed to both emberIncomingMessageHandler on the SOC and
 * ezspIncomingMessageHandler on the host.
 */
typedef struct {
  /**
   * The type of the incoming message
   */
  EmberIncomingMessageType type;
  /**
   * Aps frame for the incoming message
   */
  EmberApsFrame* apsFrame;
  /**
   * The message copied into a flat buffer
   */
  int8u* message;
  /**
   * Length of the incoming message
   */
  int16u msgLen;
  /**
   * Two byte node id of the sending node.
   */
  int16u source;
  /**
   * Link quality from the node that last relayed
   * the message.
   */
  int8u lastHopLqi;
  /**
   * The energy level (in units of dBm) observed during the reception.
   */
  int8s lastHopRssi;
  /**
   * The index of a binding that matches the message
   * or 0xFF if there is no matching binding.
   */
  int8u bindingTableIndex;
  /**
   * The index of the entry in the address table
   * that matches the sender of the message or 0xFF
   * if there is no matching entry.
   */
  int8u addressTableIndex;
  /**
   * The index of the network on which this message was received.
   */
  int8u networkIndex;
} EmberAfIncomingMessage;


/**
 * @brief Interpan Message type: unicast, broadcast, or multicast.
 */
typedef int8u EmberAfInterpanMessageType;
#define EMBER_AF_INTER_PAN_UNICAST   0x00
#define EMBER_AF_INTER_PAN_BROADCAST 0x08
#define EMBER_AF_INTER_PAN_MULTICAST 0x0C

// Legacy names
#define INTER_PAN_UNICAST   EMBER_AF_INTER_PAN_UNICAST
#define INTER_PAN_BROADCAST EMBER_AF_INTER_PAN_BROADCAST
#define INTER_PAN_MULTICAST EMBER_AF_INTER_PAN_MULTICAST


#define EMBER_AF_INTERPAN_OPTION_NONE                 0x0000
#define EMBER_AF_INTERPAN_OPTION_APS_ENCRYPT          0x0001
#define EMBER_AF_INTERPAN_OPTION_MAC_HAS_LONG_ADDRESS 0x0002

/**
 * @brief The options for sending/receiving interpan messages.
 */
typedef int16u EmberAfInterpanOptions;

/**
 * @brief Interpan header used for sending and receiving interpan
 *   messages.
 */
typedef struct {
  EmberAfInterpanMessageType messageType;

  /**
   * MAC addressing
   * For outgoing messages this is the destination.  For incoming messages
   * it is the source, which always has a long address.
   */
  EmberEUI64 longAddress;
  EmberNodeId shortAddress;
  EmberPanId panId;

  /**
   * APS data
   */
  EmberAfProfileId profileId;
  EmberAfClusterId clusterId;
  /**
   * The groupId is only used for
   * EMBER_AF_INTERPAN_MULTICAST
   */
  EmberMulticastId groupId;
  EmberAfInterpanOptions options;
} EmberAfInterpanHeader;

// Legacy Name
#define InterPanHeader EmberAfInterpanHeader
  
/**
 * @brief The options for what interpan messages are allowed.
 */
typedef int8u EmberAfAllowedInterpanOptions;

#define EMBER_AF_INTERPAN_DIRECTION_CLIENT_TO_SERVER 0x01
#define EMBER_AF_INTERPAN_DIRECTION_SERVER_TO_CLIENT 0x02
#define EMBER_AF_INTERPAN_DIRECTION_BOTH             0x03
#define EMBER_AF_INTERPAN_GLOBAL_COMMAND             0x04
#define EMBER_AF_INTERPAN_MANUFACTURER_SPECIFIC      0x08

/**
 * @brief This structure is used define an interpan message that
 *   will be accepted by the interpan filters.
 */
typedef struct {
  EmberAfProfileId profileId;
  EmberAfClusterId clusterId;
  int8u commandId;
  EmberAfAllowedInterpanOptions options;
} EmberAfAllowedInterPanMessage;


/**
 * @brief The EmberAFClusterCommand is a struct wrapper
 *   for all the data pertaining to a command which comes
 *   in over the air. This enables struct is used to
 *   encapsulate a command in a single place on the stack
 *   and pass a pointer to that location around during
 *   command processing
 */
typedef struct {
  /**
   * Aps frame for the incoming message
   */
  EmberApsFrame            *apsFrame;
  EmberIncomingMessageType  type;
  EmberNodeId               source;
  int8u                    *buffer;
  int16u                    bufLen;
  boolean                   clusterSpecific;
  boolean                   mfgSpecific;
  int16u                    mfgCode;
  int8u                     seqNum;
  int8u                     commandId;
  int8u                     payloadStartIndex;
  int8u                     direction;
  EmberAfInterpanHeader    *interPanHeader;
  int8u                     networkIndex;
} EmberAfClusterCommand;

/**
 * @brief Endpoint type struct describes clusters that are on the endpoint.
 */
typedef struct {
  /**
   * Pointer to the cluster structs, describing clusters on this
   * endpoint type.
   */
  EmberAfCluster *cluster;
  /**
   * Number of clusters in this endpoint type.
   */
  int8u  clusterCount;
  /**
   * Size of all non-external, non-singlet attribute in this endpoint type.
   */
  int16u endpointSize;
} EmberAfEndpointType;

#ifdef EZSP_HOST
  typedef EzspDecisionId EmberAfLinkKeyRequestPolicy;
  #define EMBER_AF_ALLOW_TC_KEY_REQUESTS  EZSP_ALLOW_TC_KEY_REQUESTS
  #define EMBER_AF_DENY_TC_KEY_REQUESTS   EZSP_DENY_TC_KEY_REQUESTS
  #define EMBER_AF_ALLOW_APP_KEY_REQUESTS EZSP_ALLOW_APP_KEY_REQUESTS
  #define EMBER_AF_DENY_APP_KEY_REQUESTS  EZSP_DENY_APP_KEY_REQUESTS
#else
  typedef EmberLinkKeyRequestPolicy EmberAfLinkKeyRequestPolicy;
  #define EMBER_AF_ALLOW_TC_KEY_REQUESTS  EMBER_ALLOW_KEY_REQUESTS
  #define EMBER_AF_DENY_TC_KEY_REQUESTS   EMBER_DENY_KEY_REQUESTS
  #define EMBER_AF_ALLOW_APP_KEY_REQUESTS EMBER_ALLOW_KEY_REQUESTS
  #define EMBER_AF_DENY_APP_KEY_REQUESTS  EMBER_DENY_KEY_REQUESTS
#endif

#ifdef DOXYGEN_SHOULD_SKIP_THIS
enum EmberAfSecurityProfile
#else
typedef int8u EmberAfSecurityProfile;
enum
#endif
{
  EMBER_AF_SECURITY_PROFILE_NONE    = 0x00,
  EMBER_AF_SECURITY_PROFILE_HA      = 0x01,
  EMBER_AF_SECURITY_PROFILE_HA12    = 0x02,
  EMBER_AF_SECURITY_PROFILE_SE_TEST = 0x03,
  EMBER_AF_SECURITY_PROFILE_SE_FULL = 0x04,
  EMBER_AF_SECURITY_PROFILE_CUSTOM  = 0xFF,
};

typedef struct {
  EmberAfSecurityProfile       securityProfile;
  int16u                       tcBitmask;
  EmberExtendedSecurityBitmask tcExtendedBitmask;
  int16u                       nodeBitmask;
  EmberExtendedSecurityBitmask nodeExtendedBitmask;
  EmberAfLinkKeyRequestPolicy  tcLinkKeyRequestPolicy;
  EmberAfLinkKeyRequestPolicy  appLinkKeyRequestPolicy;
  EmberKeyData                 preconfiguredKey;
} EmberAfSecurityProfileData;

#ifndef DOXYGEN_SHOULD_SKIP_THIS
typedef int8u EmAfNetworkType;
enum {
  EM_AF_NETWORK_TYPE_ZIGBEE_PRO,
  EM_AF_NETWORK_TYPE_ZIGBEE_RF4CE,
};

typedef struct {
  EmberNodeType nodeType;
  EmberAfSecurityProfile securityProfile;
} EmAfZigbeeProNetwork;

typedef int8u EmAfRf4ceNodeType;
enum {
  EM_AF_RF4CE_NODE_TYPE_CONTROLLER = 0,
  EM_AF_RF4CE_NODE_TYPE_TARGET     = 1,
};
typedef struct {
  EmAfRf4ceNodeType nodeType;
} EmAfZigbeeRf4ceNetwork;

typedef struct {
  EmAfNetworkType type;
  union {
    EmAfZigbeeProNetwork pro;
    EmAfZigbeeRf4ceNetwork rf4ce;
  } variant;
} EmAfNetwork;

#endif

#ifdef DOXYGEN_SHOULD_SKIP_THIS
enum EmberAfEndpointBitmask;
#else
typedef int8u EmberAfEndpointBitmask;
enum
#endif
{
  EMBER_AF_ENDPOINT_DISABLED = 0x00,
  EMBER_AF_ENDPOINT_ENABLED  = 0x01,
};

/**
 * @brief Struct that maps actual endpoint type, onto a specific endpoint.
 */
typedef struct {
  /**
   * Actual ZigBee endpoint number.
   */
  int8u endpoint;
  /**
   * Profile ID of the device on this endpoint.
   */
  EmberAfProfileId profileId;
  /**
   * Device ID of the device on this endpoint.
   */
  int16u deviceId;
  /**
   * Version of the device.
   */
  int8u deviceVersion;
  /**
   * Endpoint type for this endpoint.
   */
  EmberAfEndpointType *endpointType;
  /**
   * Network index for this endpoint.
   */
  int8u networkIndex;
  /**
   * Meta-data about the endpoint
   */
  EmberAfEndpointBitmask bitmask;

} EmberAfDefinedEndpoint;


// Cluster specific types

/**
 * @brief Bitmask data type for storing one bit of information for each ESI in
 * the ESI table.
 */
#if (EMBER_AF_PLUGIN_ESI_MANAGEMENT_ESI_TABLE_SIZE <= 8)
  typedef int8u EmberAfPluginEsiManagementBitmask;
#elif (EMBER_AF_PLUGIN_ESI_MANAGEMENT_ESI_TABLE_SIZE <= 16)
  typedef int16u EmberAfPluginEsiManagementBitmask;
#elif (EMBER_AF_PLUGIN_ESI_MANAGEMENT_ESI_TABLE_SIZE <= 32)
  typedef int32u EmberAfPluginEsiManagementBitmask;
#else
  #error "EMBER_AF_PLUGIN_ESI_MANAGEMENT_ESI_TABLE_SIZE cannot exceed 32"
#endif

/**
 * @brief Struct that describes a load control event.
 *
 * This is used in the load control event callback and
 * within the demand response load control cluster code.
 */
typedef struct {
  int32u     eventId;
#ifdef EMBER_AF_PLUGIN_DRLC_SERVER
  EmberEUI64 source;
  int8u      sourceEndpoint;
#endif //EMBER_AF_PLUGIN_DRLC_SERVER

#ifdef EMBER_AF_PLUGIN_DRLC
  EmberAfPluginEsiManagementBitmask esiBitmask;
#endif //EMBER_AF_PLUGIN_DRLC

  int8u      destinationEndpoint;
  int16u     deviceClass;
  int8u      utilityEnrollmentGroup;
  /**
   * Start time in seconds
   */
  int32u     startTime;
  /**
   * Duration in minutes
   */
  int16u     duration;
  int8u      criticalityLevel;
  int8u      coolingTempOffset;
  int8u      heatingTempOffset;
  int16s     coolingTempSetPoint;
  int16s     heatingTempSetPoint;
  int8s      avgLoadPercentage;
  int8u      dutyCycle;
  int8u      eventControl;
  int16u     startRand;
  int16u     endRand;
  int8u      optionControl;
} EmberAfLoadControlEvent;

/**
 * @brief This is an enum used to indicate the result of the
 *   service discovery.  Unicast discoveries are completed
 *   as soon as a response is received.  Broadcast discoveries
 *   wait a period of time for multiple responses to be received.
 */
typedef enum {
  EMBER_AF_BROADCAST_SERVICE_DISCOVERY_COMPLETE             = 0x00,
  EMBER_AF_BROADCAST_SERVICE_DISCOVERY_RESPONSE_RECEIVED    = 0x01,
  EMBER_AF_UNICAST_SERVICE_DISCOVERY_TIMEOUT                = 0x02,
  EMBER_AF_UNICAST_SERVICE_DISCOVERY_COMPLETE_WITH_RESPONSE = 0x03,
} EmberAfServiceDiscoveryStatus;

#define EM_AF_DISCOVERY_RESPONSE_MASK (0x01)

/**
 * @brief A simple way to determine if the service discovery callback
 *   has a response.
 */
#define emberAfHaveDiscoveryResponseStatus(status)  ((status) & EM_AF_DISCOVERY_RESPONSE_MASK)

/**
 * @brief A structure containing general information about the service discovery.
 */
typedef struct {
  /**
   * The status indicates both the type of request (broadcast or unicast)
   * and whether a response has been received.
   */
  EmberAfServiceDiscoveryStatus status;

  /**
   * This indicates what ZDO request cluster was associated with the request.
   * It is helpful for a callback that may be used for multiple ZDO request types
   * to determine the type of data returned.  This will be based on the
   * ZDO cluster values defined in ember-types.h.
   */
  int16u zdoRequestClusterId;

  /**
   * This is the address of the device that matched the request, which may
   * be different than the device that *actually* is responding.  This occurs
   * when parents respond on behalf of their children.
   */
  EmberNodeId matchAddress;

  /**
   * Only if the status code indicates a response will this data be non-NULL.
   * When there is data, the type is according to the ZDO cluster ID sent out.
   * For NETWORK_ADDRESS_REQUEST or IEEE_ADDRESS_REQUEST, the long ID will
   * be contained in the responseData,  so it will be a value of type ::EmberEUI64.
   * The short ID will be in the matchAddress parameter field.
   * For the MATCH_DESCRIPTORS_REQUEST the responseData will point
   * to an ::EmberAfEndpointList structure.
   */
  const void* responseData;
} EmberAfServiceDiscoveryResult;

/**
 * @brief A list of endpoints received during a service discovery attempt.
 *   This will be returned for a match descriptor request and a
 *   active endpoint request.
 */
typedef struct {
  int8u count;
  const int8u* list;
} EmberAfEndpointList;

/**
 * @brief A list of clusters received during a service discovery attempt.
 * This will be returned for a simple discriptor request.
 */
typedef struct {
  int8u inClusterCount;
  const int16u* inClusterList;
  int8u outClusterCount;
  const int16u* outClusterList;
} EmberAfClusterList;

/**
 * @brief This defines a callback where a code element or cluster can be informed
 * as to the result of a service discovery they have requested.
 * For each match, the callback is fired with all the resulting matches from
 * that source.  If the discovery was unicast to a specific device, then
 * the callback will only be fired once with either MATCH_FOUND or COMPLETE
 * (no matches found).  If the discovery is broadcast then multiple callbacks
 * may be fired with ::EMBER_AF_SERVICE_DISCOVERY_RESPONSE_RECEIVED.
 * After a couple seconds the callback will then be fired with
 * ::EMBER_AF_SERVICE_DISCOVERY_COMPLETE as the result.
 */
typedef void (EmberAfServiceDiscoveryCallback)(const EmberAfServiceDiscoveryResult* result);

/**
 * @brief This defines a callback where a code element or cluster can be
 * informed as to the result of a request to initiate a partner link key
 * exchange.  The callback will be triggered with success equal to TRUE if the
 * exchange completed successfully.
 */
typedef void (EmberAfPartnerLinkKeyExchangeCallback)(boolean success);

/**
 * @brief This is an enum used to control how the device will poll for a given
 * active cluster-related event.  When the event is scheduled, the application
 * can pass a poll control value which will be stored along with the event.
 * The processor is only allowed to poll according to the most restrictive
 * value for all active event.  For instance, if two events are active, one
 * with EMBER_AF_LONG_POLL and the other with EMBER_AF_SHORT_POLL, then the
 * processor will short poll until the second event is deactivated.
 */
typedef enum {
  EMBER_AF_LONG_POLL,
  EMBER_AF_SHORT_POLL,
} EmberAfEventPollControl;

/**
 * @brief This is an enum used to control how the device
 *        will sleep for a given active cluster related event.
 *        When the event is scheduled, the scheduling code can
 *        pass a sleep control value which will be stored along
 *        with the event. The processor is only allowed to sleep
 *        according to the most restrictive sleep control value
 *        for any active event. For instance, if two events
 *        are active, one with EMBER_AF_OK_TO_HIBERNATE and the
 *        other with EMBER_AF_OK_TO_NAP, then the processor
 *        will only be allowed to nap until the second event
 *        is deactivated.
 */
typedef enum {
  EMBER_AF_OK_TO_SLEEP,
  /** @deprecated. */
  EMBER_AF_OK_TO_HIBERNATE = EMBER_AF_OK_TO_SLEEP,
  /** @deprecated. */
  EMBER_AF_OK_TO_NAP,
  EMBER_AF_STAY_AWAKE,
} EmberAfEventSleepControl;

/**
 * @brief An enum used to track the tasks that the Application
 * framework cares about. These are inteneded to be tasks
 * that should keep the device out of hibernation like an
 * application level request / response. If the response does
 * not come in as a data ack, then the application will need
 * to stay out of hibernation to wait and poll for it.
 *
 * Of coures some tasks do not necessarily have a response. For
 * instance, a ZDO request may or may not have a response. In this
 * case, the application framework cannot rely on the fact that
 * a response will come in to end the wake cycle, so the Application
 * framework must timeout the wake cycle if no expected
 * response is received or no other event can be relied upon to
 * end the wake cycle.
 *
 * Tasks of this type should be added to the wake timeout mask
 * by calling ::emberAfSetWakeTimeoutBitmaskCallback so that they
 * can be governed by a timeout instead of a request / response
 *
 * the current tasks bitmask is an int32u bitmask used to
 * track which tasks are active at any given time. The bottom 16 bits,
 * values 0x01 - 0x8000 are reserved for Ember's use. The top
 * 16 bits are reserved for the customer, values 0x10000 -
 * 0x80000000
 */
#ifdef DOXYGEN_SHOULD_SKIP_THIS
enum EmberAfApplicationTask
#else
typedef int32u EmberAfApplicationTask;
enum
#endif
{
  // we may be able to remove these top two since they are
  // handled by the stack on the SOC.
  EMBER_AF_WAITING_FOR_DATA_ACK                  = 0x00000001, //not needed?
  EMBER_AF_LAST_POLL_GOT_DATA                    = 0x00000002, //not needed?
  EMBER_AF_WAITING_FOR_SERVICE_DISCOVERY         = 0x00000004,
  EMBER_AF_WAITING_FOR_ZDO_RESPONSE              = 0x00000008,
  EMBER_AF_WAITING_FOR_ZCL_RESPONSE              = 0x00000010,
  EMBER_AF_WAITING_FOR_REGISTRATION              = 0x00000020,
  EMBER_AF_WAITING_FOR_PARTNER_LINK_KEY_EXCHANGE = 0x00000040,
  EMBER_AF_FORCE_SHORT_POLL                      = 0x00000080,
};

/**
 * @brief a structure used to keep track of cluster related events and
 * their sleep control values. The cluster code will not know at
 * runtime all of the events that it has access to in the event table
 * This structure is stored by the application framework in an event
 * context table which allong with helper functions allows the cluster
 * code to schedule and deactivate its associated events.
 */
typedef struct {
  /**
   * The endpoint of the associated cluster event.
   */
  int8u endpoint;
  /**
   * The cluster id of the associated cluster event.
   */
  EmberAfClusterId clusterId;
  /**
   * The server/client identity of the associated cluster event.
   */
  boolean isClient;
  /**
   * A poll control value used to control the network polling behavior while
   * the event is active.
   */
  EmberAfEventPollControl pollControl;
  /**
   * A sleep control value used to control the processor's sleep
   * behavior while the event is active.
   */
  EmberAfEventSleepControl sleepControl;
  /**
   * A pointer to the event control value which is stored in the event table
   * and is used to actually schedule the event.
   */
  EmberEventControl *eventControl;
} EmberAfEventContext;

/**
 * @brief Type for referring to the handler for network events.
 */
typedef void (*EmberAfNetworkEventHandler)(void);

/**
 * @brief Type for referring to the handler for endpoint events.
 */
typedef void (*EmberAfEndpointEventHandler)(int8u endpoint);

#ifdef EMBER_AF_PLUGIN_GROUPS_SERVER
/**
 * @brief Indicates the absence of a Group table entry.
 */
#define EMBER_AF_GROUP_TABLE_NULL_INDEX 0xFF
/**
 * @brief Value used when setting or getting the endpoint in a Group table
 * entry.  It indicates that the entry is not in use.
 */
#define EMBER_AF_GROUP_TABLE_UNUSED_ENDPOINT_ID 0x00
/**
 * @brief Maximum length of Group names, not including the length byte.
 */
#define ZCL_GROUPS_CLUSTER_MAXIMUM_NAME_LENGTH 16
/**
 * @brief A structure used to store group table entries in RAM or in tokens,
 * depending on the the platform.  If the endpoint field is
 * ::EMBER_AF_GROUP_TABLE_UNUSED_ENDPOINT_ID, the entry is unused.
 */
typedef struct {
  int8u  endpoint; // 0x00 when not in use
  int16u groupId;
  int8u  bindingIndex;
#ifdef EMBER_AF_PLUGIN_GROUPS_SERVER_NAME_SUPPORT
  int8u  name[ZCL_GROUPS_CLUSTER_MAXIMUM_NAME_LENGTH + 1];
#endif
} EmberAfGroupTableEntry;
#endif //EMBER_AF_PLUGIN_GROUPS_SERVER

/**
 * @brief Indicates the absence of a Scene table entry.
 */
#define EMBER_AF_SCENE_TABLE_NULL_INDEX 0xFF
/**
 * @brief Value used when setting or getting the endpoint in a Scene table
 * entry.  It indicates that the entry is not in use.
 */
#define EMBER_AF_SCENE_TABLE_UNUSED_ENDPOINT_ID 0x00
/**
 * @brief Maximum length of Scene names, not including the length byte.
 */
#define ZCL_SCENES_CLUSTER_MAXIMUM_NAME_LENGTH 16
/**
 * @brief The group identifier for the global scene.
 */
#define ZCL_SCENES_GLOBAL_SCENE_GROUP_ID 0x0000
/**
 * @brief The scene identifier for the global scene.
 */
#define ZCL_SCENES_GLOBAL_SCENE_SCENE_ID 0x00
/**
 * @brief A structure used to store scene table entries in RAM or in tokens,
 * depending on a plugin setting.  If endpoint field is
 * ::EMBER_AF_SCENE_TABLE_UNUSED_ENDPOINT_ID, the entry is unused.
 */
typedef struct {
  int8u   endpoint;                // 0x00 when this record is not in use
  int16u  groupId;                 // 0x0000 if not associated with a group
  int8u   sceneId;
#ifdef EMBER_AF_PLUGIN_SCENES_NAME_SUPPORT
  int8u   name[ZCL_SCENES_CLUSTER_MAXIMUM_NAME_LENGTH + 1];
#endif
  int16u  transitionTime;          // in seconds
  int8u   transitionTime100ms;     // in tenths of a seconds
#ifdef ZCL_USING_ON_OFF_CLUSTER_SERVER
  boolean hasOnOffValue;
  boolean onOffValue;
#endif
#ifdef ZCL_USING_LEVEL_CONTROL_CLUSTER_SERVER
  boolean hasCurrentLevelValue;
  int8u   currentLevelValue;
#endif
#ifdef ZCL_USING_THERMOSTAT_CLUSTER_SERVER
  boolean hasOccupiedCoolingSetpointValue;
  int16s  occupiedCoolingSetpointValue;
  boolean hasOccupiedHeatingSetpointValue;
  int16s  occupiedHeatingSetpointValue;
  boolean hasSystemModeValue;
  int8u   systemModeValue;
#endif
#ifdef ZCL_USING_COLOR_CONTROL_CLUSTER_SERVER
  boolean hasCurrentXValue;
  int16u  currentXValue;
  boolean hasCurrentYValue;
  int16u  currentYValue;
  boolean hasEnhancedCurrentHueValue;
  int16u  enhancedCurrentHueValue;
  boolean hasCurrentSaturationValue;
  int8u   currentSaturationValue;
  boolean hasColorLoopActiveValue;
  int8u   colorLoopActiveValue;
  boolean hasColorLoopDirectionValue;
  int8u   colorLoopDirectionValue;
  boolean hasColorLoopTimeValue;
  int16u  colorLoopTimeValue;
#endif //ZCL_USING_COLOR_CONTROL_CLUSTER_SERVER
#ifdef ZCL_USING_DOOR_LOCK_CLUSTER_SERVER
  boolean hasLockStateValue;
  int8u   lockStateValue;
#endif
#ifdef ZCL_USING_WINDOW_COVERING_CLUSTER_SERVER
  boolean hasCurrentPositionLiftPercentageValue;
  int8u   currentPositionLiftPercentageValue;
  boolean hasCurrentPositionTiltPercentageValue;
  int8u   currentPositionTiltPercentageValue;
#endif
} EmberAfSceneTableEntry;

#if !defined(EMBER_AF_PLUGIN_MESSAGING_CLIENT)
  // In order to be able to forward declare callbacks regardless of whether the plugin
  // is enabled, we need to define all data structures.  In order to be able to define
  // the messaging client data struct, we need to declare this variable.
  #define EMBER_AF_PLUGIN_MESSAGING_CLIENT_MESSAGE_SIZE 0
#endif

typedef struct {
  boolean    valid;
  boolean    active;
  EmberAfPluginEsiManagementBitmask esiBitmask;
  int8u      clientEndpoint;
  int32u     messageId;
  int8u      messageControl;
  int32u     startTime;
  int32u     endTime;
  int16u     durationInMinutes;
  int8u      message[EMBER_AF_PLUGIN_MESSAGING_CLIENT_MESSAGE_SIZE + 1];
} EmberAfPluginMessagingClientMessage;

#define ZCL_PRICE_CLUSTER_MAXIMUM_RATE_LABEL_LENGTH 11
typedef struct {
  boolean valid;
  boolean active;
  int8u   clientEndpoint;
  int32u  providerId;
  int8u   rateLabel[ZCL_PRICE_CLUSTER_MAXIMUM_RATE_LABEL_LENGTH + 1];
  int32u  issuerEventId;
  int32u  currentTime;
  int8u   unitOfMeasure;
  int16u  currency;
  int8u   priceTrailingDigitAndPriceTier;
  int8u   numberOfPriceTiersAndRegisterTier;
  int32u  startTime;
  int32u  endTime;
  int16u  durationInMinutes;
  int32u  price;
  int8u   priceRatio;
  int32u  generationPrice;
  int8u   generationPriceRatio;
  int32u  alternateCostDelivered;
  int8u   alternateCostUnit;
  int8u   alternateCostTrailingDigit;
  int8u   numberOfBlockThresholds;
  int8u   priceControl;
} EmberAfPluginPriceClientPrice;

/**
 * @brief Value used when setting or getting the endpoint in a report table
 * entry.  It indicates that the entry is not in use.
 */
#define EMBER_AF_PLUGIN_REPORTING_UNUSED_ENDPOINT_ID 0x00
/**
 * @brief A structure used to store reporting configurations.  If endpoint
 * field is ::EMBER_AF_PLUGIN_REPORTING_UNUSED_ENDPOINT_ID, the entry is
 * unused.
 */
typedef struct {
  /** EMBER_ZCL_REPORTING_DIRECTION_REPORTED for reports sent from the local
   *  device or EMBER_ZCL_REPORTING_DIRECTION_RECEIVED for reports received
   *  from a remote device.
   */
  EmberAfReportingDirection direction;
  /** The local endpoint from which the attribute is reported or to which the
   * report is received.  If ::EMBER_AF_PLUGIN_REPORTING_UNUSED_ENDPOINT_ID,
   * the entry is unused.
   */
  int8u endpoint;
  /** The cluster where the attribute is located. */
  EmberAfClusterId clusterId;
  /** The id of the attribute being reported or received. */
  EmberAfAttributeId attributeId;
  /** CLUSTER_MASK_SERVER for server-side attributes or CLUSTER_MASK_CLIENT for
   *  client-side attributes.
   */
  int8u mask;
  /** Manufacturer code associated with the cluster and/or attribute.  If the
   *  cluster id is inside the manufacturer-specific range, this value
   *  indicates the manufacturer code for the cluster.  Otherwise, if this
   *  value is non-zero and the cluster id is a standard ZCL cluster, it
   *  indicates the manufacturer code for attribute.
   */
  int16u manufacturerCode;
  union {
    struct {
      /** The minimum reporting interval, measured in seconds. */
      int16u minInterval;
      /** The maximum reporting interval, measured in seconds. */
      int16u maxInterval;
      /** The minimum change to the attribute that will result in a report
       *  being sent.
       */
      int32u reportableChange;
    } reported;
    struct {
      /** The node id of the source of the received reports. */
      EmberNodeId source;
      /** The remote endpoint from which the attribute is reported. */
      int8u endpoint;
      /** The maximum expected time between reports, measured in seconds. */
      int16u timeout;
    } received;
  } data;
} EmberAfPluginReportingEntry;

typedef enum {
  EMBER_AF_PLUGIN_TUNNELING_CLIENT_SUCCESS                          = 0x00,
  EMBER_AF_PLUGIN_TUNNELING_CLIENT_BUSY                             = 0x01,
  EMBER_AF_PLUGIN_TUNNELING_CLIENT_NO_MORE_TUNNEL_IDS               = 0x02,
  EMBER_AF_PLUGIN_TUNNELING_CLIENT_PROTOCOL_NOT_SUPPORTED           = 0x03,
  EMBER_AF_PLUGIN_TUNNELING_CLIENT_FLOW_CONTROL_NOT_SUPPORTED       = 0x04,
  EMBER_AF_PLUGIN_TUNNELING_CLIENT_IEEE_ADDRESS_REQUEST_FAILED      = 0xF9,
  EMBER_AF_PLUGIN_TUNNELING_CLIENT_IEEE_ADDRESS_NOT_FOUND           = 0xFA,
  EMBER_AF_PLUGIN_TUNNELING_CLIENT_ADDRESS_TABLE_FULL               = 0xFB,
  EMBER_AF_PLUGIN_TUNNELING_CLIENT_LINK_KEY_EXCHANGE_REQUEST_FAILED = 0xFC,
  EMBER_AF_PLUGIN_TUNNELING_CLIENT_LINK_KEY_EXCHANGE_FAILED         = 0xFD,
  EMBER_AF_PLUGIN_TUNNELING_CLIENT_REQUEST_TUNNEL_FAILED            = 0xFE,
  EMBER_AF_PLUGIN_TUNNELING_CLIENT_REQUEST_TUNNEL_TIMEOUT           = 0xFF,
} EmberAfPluginTunnelingClientStatus;

#ifdef EMBER_AF_PLUGIN_ZLL_COMMISSIONING
/**
 * @brief Status codes used by the ZLL Commissioning plugin.
 */
#ifdef DOXYGEN_SHOULD_SKIP_THIS
enum EmberAfZllCommissioningStatus
#else
typedef int8u EmberAfZllCommissioningStatus;
enum
#endif
{
  EMBER_AF_ZLL_ABORTED_BY_APPLICATION                      = 0x00,
  EMBER_AF_ZLL_CHANNEL_CHANGE_FAILED                       = 0x01,
  EMBER_AF_ZLL_JOINING_FAILED                              = 0x02,
  EMBER_AF_ZLL_NO_NETWORKS_FOUND                           = 0x03,
  EMBER_AF_ZLL_PREEMPTED_BY_STACK                          = 0x04,
  EMBER_AF_ZLL_SENDING_START_JOIN_FAILED                   = 0x05,
  EMBER_AF_ZLL_SENDING_DEVICE_INFORMATION_REQUEST_FAILED   = 0x06,
  EMBER_AF_ZLL_SENDING_IDENTIFY_REQUEST_FAILED             = 0x07,
  EMBER_AF_ZLL_SENDING_RESET_TO_FACTORY_NEW_REQUEST_FAILED = 0x08,
};

/**
 * @brief A structure used to represent Group Information Records used by ZLL
 * Commissioning.
 */
typedef struct {
  EmberMulticastId groupId;
  int8u            groupType;
} EmberAfPluginZllCommissioningGroupInformationRecord;

/**
 * @brief A structure used to represent Endpoint Information Records used by
 * ZLL Commissioning.
 */
typedef struct {
  EmberNodeId networkAddress;
  int8u       endpointId;
  int16u      profileId;
  int16u      deviceId;
  int8u       version;
} EmberAfPluginZllCommissioningEndpointInformationRecord;
#endif

/**
 * @brief This is a unique identifier for referencing ZigBee Over-the-air upgrade
 *   images.  It is used by the OTA plugins when passing around information about
 *   an upgrade file.
 */
typedef struct {
  int16u manufacturerId;
  int16u imageTypeId;
  int32u firmwareVersion;

  /**
   * This is only used for device specific files.
   * It will be set to all 0's when the image does not
   * have an upgrade destination field in it.
   * Little endian format.
   */
  int8u deviceSpecificFileEui64[EUI64_SIZE];
} EmberAfOtaImageId;


/**
 * @brief The list of options possible for the image block request/response.
 */
enum {
  EMBER_AF_IMAGE_BLOCK_REQUEST_OPTIONS_NONE                          = 0,
  EMBER_AF_IMAGE_BLOCK_REQUEST_MIN_BLOCK_REQUEST_SUPPORTED_BY_CLIENT = 1,
  EMBER_AF_IMAGE_BLOCK_REQUEST_MIN_BLOCK_REQUEST_SUPPORTED_BY_SERVER = 2,
};
typedef int8u EmberAfImageBlockRequestOptions;

/**
 * @brief This is the data structure that is passed to the
 * emberAfImageBlockRequestCallback() to let the application decide what to do.
 */
typedef struct {
  const EmberAfOtaImageId* id;
  int32u offset;
  int32u waitTimeMinutesResponse;
  EmberNodeId source;
  int16u minBlockRequestPeriod;
  int8u maxDataSize;
  int8u clientEndpoint;
  EmberAfImageBlockRequestOptions bitmask;
} EmberAfImageBlockRequestCallbackStruct;


/**
 * @brief This status contains the success or error code of an OTA storage
 *   device operation.
 */
typedef enum {
  EMBER_AF_OTA_STORAGE_SUCCESS               = 0,
  EMBER_AF_OTA_STORAGE_ERROR                 = 1,
  EMBER_AF_OTA_STORAGE_RETURN_DATA_TOO_LONG  = 2,
  EMBER_AF_OTA_STORAGE_PARTIAL_FILE_FOUND    = 3,
  EMBER_AF_OTA_STORAGE_OPERATION_IN_PROGRESS = 4,
} EmberAfOtaStorageStatus;

/**
 * @brief This status contains the success or error code of an OTA download
 * operation.
 */
enum {
  EMBER_AF_OTA_DOWNLOAD_AND_VERIFY_SUCCESS = 0,
  EMBER_AF_OTA_DOWNLOAD_TIME_OUT           = 1,
  EMBER_AF_OTA_VERIFY_FAILED               = 2,
  EMBER_AF_OTA_SERVER_ABORTED              = 3,
  EMBER_AF_OTA_CLIENT_ABORTED              = 4,
  EMBER_AF_OTA_ERASE_FAILED                = 5,
};
typedef int8u EmberAfOtaDownloadResult;

/**
 * @brief The maximum size of the string that is present in the header
 *   of the ZigBee Over-the-air file format.
 */
#define EMBER_AF_OTA_MAX_HEADER_STRING_LENGTH 32

/**
 * @brief This structure is an in-memory representation of
 *   the Over-the-air header data that resides on disk.
 *   It is not a byte-for-byte copy.
 */
typedef struct {
  // Magic Number omitted since it is always the same.
  int16u headerVersion;
  int16u headerLength;
  int16u fieldControl;
  int16u manufacturerId;
  int16u imageTypeId;           // a.k.a. Device ID
  int32u firmwareVersion;
  int16u zigbeeStackVersion;

  /**
   * @brief The spec. does NOT require that the string be NULL terminated in the
   *   header stored on disk.  Therefore we make sure we can support a
   *   32-character string without a NULL terminator by adding +1 in the data
   *   structure.
   */
  int8u headerString[EMBER_AF_OTA_MAX_HEADER_STRING_LENGTH + 1];

  /**
   * @brief When reading the header this will be the complete length of
   *  the file. When writing the header, this must be set to
   *  the length of the MFG image data portion including all tags.
   */
  int32u imageSize;

  /**
   * @brief The remaining four fields are optional. The field control should be checked
   *   to determine if their values are valid.
   */
  int8u securityCredentials;
  int8u upgradeFileDestination[EUI64_SIZE];
  int16u minimumHardwareVersion;
  int16u maximumHardwareVersion;
} EmberAfOtaHeader;

/**
 * @brief This structure contains information about a tag that resides
 *   within an Over-the-air bootload file.
 */
typedef struct {
  int16u id;
  int32u length;
} EmberAfTagData;

typedef enum {
  NO_APP_MESSAGE               = 0,
  RECEIVED_PARTNER_CERTIFICATE = 1,
  GENERATING_EPHEMERAL_KEYS    = 2,
  GENERATING_SHARED_SECRET     = 3,
  KEY_GENERATION_DONE          = 4,
  GENERATE_SHARED_SECRET_DONE  = 5,
  /**
   * LINK_KEY_ESTABLISHED indicates Success,
   * key establishment done.
   */
  LINK_KEY_ESTABLISHED         = 6,


  /**
   * Error codes:
   * Transient failures where Key Establishment could be retried
   */
  NO_LOCAL_RESOURCES          = 7,
  PARTNER_NO_RESOURCES        = 8,
  TIMEOUT_OCCURRED            = 9,
  INVALID_APP_COMMAND         = 10,
  MESSAGE_SEND_FAILURE        = 11,
  PARTNER_SENT_TERMINATE      = 12,
  INVALID_PARTNER_MESSAGE     = 13,
  PARTNER_SENT_DEFAULT_RESPONSE_ERROR = 14,

  /**
   * Fatal Errors:
   * These results are not worth retrying because the outcome
   * will not change
   */
  BAD_CERTIFICATE_ISSUER      = 15,
  KEY_CONFIRM_FAILURE         = 16,
  BAD_KEY_ESTABLISHMENT_SUITE = 17,

  KEY_TABLE_FULL              = 18,

  /**
   * Neither initiator nor responder is an
   * ESP/TC so the key establishment is not
   * allowed per the spec.
   */
  NO_ESTABLISHMENT_ALLOWED    = 19,

  /* 283k1 certificates need to have valid key usage 
  */
  INVALID_CERTIFICATE_KEY_USAGE = 20,
} EmberAfKeyEstablishmentNotifyMessage;

#define APP_NOTIFY_ERROR_CODE_START NO_LOCAL_RESOURCES
#define APP_NOTIFY_MESSAGE_TEXT { \
  "None",                         \
  "Received Cert",                \
  "Generate keys",                \
  "Generate secret",              \
  "Key generate done",            \
  "Generate secret done",         \
  "Link key verified",            \
                                  \
  /* Transient Error codes */     \
  "No local resources",           \
  "Partner no resources",         \
  "Timeout",                      \
  "Invalid app. command",         \
  "Message send failure",         \
  "Partner sent terminate",       \
  "Bad message",                  \
  "Partner sent Default Rsp",     \
                                  \
  /* Fatal errors */              \
  "Bad cert issuer",              \
  "Key confirm failure",          \
  "Bad key est. suite",           \
  "Key table full",               \
  "Not allowed",                  \
  "Invalid Key Usage",            \
}

/**
 * @brief This enumeration is used to indicate the state of an OTA bootload
 *   image undergoing verification.  This is used both for cryptographic
 *   verification and manufacturer specific verification.
 */
typedef enum {
  EMBER_AF_IMAGE_GOOD                 = 0,
  EMBER_AF_IMAGE_BAD                  = 1,
  EMBER_AF_IMAGE_VERIFY_IN_PROGRESS   = 2,

#ifndef DOXYGEN_SHOULD_SKIP_THIS
  // Internal use only.
  EMBER_AF_IMAGE_VERIFY_WAIT          = 3,
  EMBER_AF_IMAGE_VERIFY_ERROR         = 4,
  EMBER_AF_IMAGE_UNKNOWN              = 5,
  EMBER_AF_NO_IMAGE_VERIFY_SUPPORT    = 6,
#endif
} EmberAfImageVerifyStatus;

/**
 * @brief Type for referring to the tick callback for cluster.
 *
 * Tick function will be called once for each tick for each endpoint in
 * the cluster. The rate of tick is determined by the metadata of the
 * cluster.
 */
typedef void (*EmberAfTickFunction)(int8u endpoint);


/**
 * @brief Type for referring to the init callback for cluster.
 *
 * Init function is called when the application starts up, once for
 * each cluster/endpoint combination.
 */
typedef void (*EmberAfInitFunction)(int8u endpoint);

/**
 * @brief Type for referring to the attribute changed callback function.
 *
 * This function is called just after an attribute changes.
 */
typedef void (*EmberAfClusterAttributeChangedCallback)(int8u endpoint,
                                                       EmberAfAttributeId attributeId);

/**
 * @brief Type for referring to the manufacturer specific
 *        attribute changed callback function.
 *
 * This function is called just after a manufacturer specific attribute changes.
 */
typedef void (*EmberAfManufacturerSpecificClusterAttributeChangedCallback)(int8u endpoint,
                                                       EmberAfAttributeId attributeId,
                                                       int16u manufacturerCode);

/**
 * @brief Type for referring to the pre-attribute changed callback function.
 *
 * This function is called before an attribute changes.
 */
typedef EmberAfStatus (*EmberAfClusterPreAttributeChangedCallback)(int8u endpoint,
                                                                   EmberAfAttributeId attributeId,
                                                                   EmberAfAttributeType attributeType,
                                                                   int8u size,
                                                                   int8u *value);

/**
 * @brief Type for referring to the default response callback function.
 *
 * This function is called when default response is received, before
 * the global callback. Global callback is called immediatelly afterwards.
 */
typedef void (*EmberAfDefaultResponseFunction)(int8u endpoint,
                                               int8u commandId,
                                               EmberAfStatus status);

/**
 * @brief Type for referring to the message sent callback function.
 *
 * This function is called when a message is sent.
 */
typedef void (*EmberAfMessageSentFunction)(EmberOutgoingMessageType type,
                                           int16u indexOrDestination,
                                           EmberApsFrame *apsFrame,
                                           int16u msgLen,
                                           int8u *message,
                                           EmberStatus status);

/**
 * @brief A data struct for a link key backup.
 *
 * Each entry notes the EUI64 of the device it is paired to and the key data.
 *   This key may be hashed and not the actual link key currently in use.
 */

typedef struct {
  EmberEUI64 deviceId;
  EmberKeyData key;
} EmberAfLinkKeyBackupData;

/**
 * @brief A data struct for all the trust center backup data.
 *
 * The 'keyList' pointer must point to an array and 'maxKeyListLength'
 * must be populated with the maximum number of entries the array can hold.
 *
 * Functions that modify this data structure will populate 'keyListLength'
 * indicating how many keys were actually written into 'keyList'.
 */

typedef struct {
  EmberEUI64 extendedPanId;
  int8u keyListLength;
  int8u maxKeyListLength;
  EmberAfLinkKeyBackupData* keyList;
} EmberAfTrustCenterBackupData;

/**
 * @brief The length of the hardware tag in the Ember Bootloader Query
 *   Response.
 */
#define EMBER_AF_STANDALONE_BOOTLOADER_HARDWARE_TAG_LENGTH 16


/**
 * @brief A data struct for the information retrieved during a response
 *   to an Ember Bootloader over-the-air query.
 */
typedef struct {
  int8u hardwareTag[EMBER_AF_STANDALONE_BOOTLOADER_HARDWARE_TAG_LENGTH];
  int8u eui64[EUI64_SIZE];
  int16u mfgId;
  int16u bootloaderVersion;
  int8u capabilities;
  int8u platform;
  int8u micro;
  int8u phy;
  boolean bootloaderActive;
} EmberAfStandaloneBootloaderQueryResponseData;

/**
 * @brief A data struct used to keep track of incoming and outgoing
 *   commands for command discovery
 */
typedef struct {
  int16u clusterId;
  int8u commandId;
  int8u mask;
} EmberAfCommandMetadata;

/**
* @brief A data structure used to describe the time in a human
* understandable format (as opposed to 32-bit UTC)
*/

typedef struct {
  int16u year;
  int8u month;
  int8u day;
  int8u hours;
  int8u minutes;
  int8u seconds;
} EmberAfTimeStruct;

/* Simple Metering Server Test Code */
#define EMBER_AF_PLUGIN_SIMPLE_METERING_SERVER_ELECTRIC_METER 0
#define EMBER_AF_PLUGIN_SIMPLE_METERING_SERVER_GAS_METER 1

/**
 * @brief ZigBee RF4CE status.
 */
#ifdef DOXYGEN_SHOULD_SKIP_THIS
enum EmberAfRf4ceStatus
#else
typedef int8u EmberAfRf4ceStatus;
enum
#endif
{
  EMBER_AF_RF4CE_STATUS_SUCCESS               = 0x00,
  EMBER_AF_RF4CE_STATUS_NO_ORG_CAPACITY       = 0xB0,
  EMBER_AF_RF4CE_STATUS_NO_REC_CAPACITY       = 0xB1,
  EMBER_AF_RF4CE_STATUS_NO_PAIRING            = 0xB2,
  EMBER_AF_RF4CE_STATUS_NO_RESPONSE           = 0xB3,
  EMBER_AF_RF4CE_STATUS_NOT_PERMITTED         = 0xB4,
  EMBER_AF_RF4CE_STATUS_DUPLICATE_PAIRING     = 0xB5,
  EMBER_AF_RF4CE_STATUS_FRAME_COUNTER_EXPIRED = 0xB6,
  EMBER_AF_RF4CE_STATUS_DISCOVERY_ERROR       = 0xB7,
  EMBER_AF_RF4CE_STATUS_DISCOVERY_TIMEOUT     = 0xB8,
  EMBER_AF_RF4CE_STATUS_SECURITY_TIMEOUT      = 0xB9,
  EMBER_AF_RF4CE_STATUS_SECURITY_FAILURE      = 0xBA,
  EMBER_AF_RF4CE_STATUS_INVALID_PARAMETER     = 0xE8,
  EMBER_AF_RF4CE_STATUS_UNSUPPORTED_ATTRIBUTE = 0xF4,
  EMBER_AF_RF4CE_STATUS_INVALID_INDEX         = 0xF9,
};

/**
 * @brief ZigBee RF4CE profile identifier.
 */
#ifdef DOXYGEN_SHOULD_SKIP_THIS
enum EmberAfRf4ceProfileId
#else
typedef int8u EmberAfRf4ceProfileId;
enum
#endif
{
  /** Generic Device Profile (GDP) versions 1.0 and 2.0. */
  EMBER_AF_RF4CE_PROFILE_GENERIC_DEVICE                      = 0x00,
  /** The Consumer Electronics Remote Control (CERC) profile was renamed to the
   * ZigBee Remote Control (ZRC) profile when the specification went from
   * version 1.0 in document 09-4946-00 to version 1.1 in document 09-4946-01.
   * 1.1 is backwards compatible with 1.0.  For convenience, the profile can be
   * referred to as CERC, ZRC 1.0, or ZRC 1.1.
   */
  EMBER_AF_RF4CE_PROFILE_CONSUMER_ELECTRONICS_REMOTE_CONTROL = 0x01,
  /** A convenience alias for
   * ::EMBER_AF_RF4CE_PROFILE_CONSUMER_ELECTRONICS_REMOTE_CONTROL.
   */
  EMBER_AF_RF4CE_PROFILE_REMOTE_CONTROL_1_0                  = 0x01,
  /** ZigBee Remote Control (ZRC) profile version 1.1. */
  EMBER_AF_RF4CE_PROFILE_REMOTE_CONTROL_1_1                  = 0x01,
  /** ZigBee Input Device (ZID) profile version 1.0. */
  EMBER_AF_RF4CE_PROFILE_INPUT_DEVICE_1_0                    = 0x02,
  /** ZigBee Remote Control (ZRC) profile version 2.0. */
  EMBER_AF_RF4CE_PROFILE_REMOTE_CONTROL_2_0                  = 0x03,
  /** Multiple System Operators (MSO) profile. */
  EMBER_AF_RF4CE_PROFILE_MSO                                 = 0xC0,
  /** Wildcard profile. */
  EMBER_AF_RF4CE_PROFILE_WILDCARD                            = 0xFF,
};

/**
 * @brief ZigBee RF4CE vendor identifier.
 */
#ifdef DOXYGEN_SHOULD_SKIP_THIS
enum EmberAfRf4ceVendor
#else
typedef int16u EmberAfRf4ceVendor;
enum
#endif
{
  EMBER_AF_RF4CE_VENDOR_PANASONIC         = 0x0001,
  EMBER_AF_RF4CE_VENDOR_SONY              = 0x0002,
  EMBER_AF_RF4CE_VENDOR_SAMSUNG           = 0x0003,
  EMBER_AF_RF4CE_VENDOR_PHILIPS           = 0x0004,
  EMBER_AF_RF4CE_VENDOR_FREESCALE         = 0x0005,
  EMBER_AF_RF4CE_VENDOR_OKI_SEMICONDUCTOR = 0x0006,
  EMBER_AF_RF4CE_VENDOR_TEXAS_INSTRUMENTS = 0x0007,
  EMBER_AF_RF4CE_VENDOR_TEST_VENDOR_1     = 0xFFF1,
  EMBER_AF_RF4CE_VENDOR_TEST_VENDOR_2     = 0xFFF2,
  EMBER_AF_RF4CE_VENDOR_TEST_VENDOR_3     = 0xFFF3,
};

/**
 * @brief ZigBee RF4CE device type.
 */
#ifdef DOXYGEN_SHOULD_SKIP_THIS
enum EmberAfRf4ceDeviceType
#else
typedef int8u EmberAfRf4ceDeviceType;
enum
#endif
{
  EMBER_AF_RF4CE_DEVICE_TYPE_REMOTE_CONTROL           = 0x01,
  EMBER_AF_RF4CE_DEVICE_TYPE_TELEVISION               = 0x02,
  EMBER_AF_RF4CE_DEVICE_TYPE_PROJECTOR                = 0x03,
  EMBER_AF_RF4CE_DEVICE_TYPE_PLAYER                   = 0x04,
  EMBER_AF_RF4CE_DEVICE_TYPE_RECORDER                 = 0x05,
  EMBER_AF_RF4CE_DEVICE_TYPE_VIDEO_PLAYER_RECORDER    = 0x06,
  EMBER_AF_RF4CE_DEVICE_TYPE_AUDIO_PLAYER_RECORDER    = 0x07,
  EMBER_AF_RF4CE_DEVICE_TYPE_AUDIO_VIDEO_RECORDER     = 0x08,
  EMBER_AF_RF4CE_DEVICE_TYPE_SET_TOP_BOX              = 0x09,
  EMBER_AF_RF4CE_DEVICE_TYPE_HOME_THEATER_SYSTEM      = 0x0A,
  EMBER_AF_RF4CE_DEVICE_TYPE_MEDIA_CENTER_PC          = 0x0B,
  EMBER_AF_RF4CE_DEVICE_TYPE_GAME_CONSOLE             = 0x0C,
  EMBER_AF_RF4CE_DEVICE_TYPE_SATELLITE_RADIO_RECEIVER = 0x0D,
  EMBER_AF_RF4CE_DEVICE_TYPE_IR_EXTENDER              = 0x0E,
  EMBER_AF_RF4CE_DEVICE_TYPE_MONITOR                  = 0x0F,
  EMBER_AF_RF4CE_DEVICE_TYPE_GENERIC                  = 0xFE,
  EMBER_AF_RF4CE_DEVICE_TYPE_WILDCARD                 = 0xFF,
};

/**
 * @brief RF4CE GDP command codes.
 */
#ifdef DOXYGEN_SHOULD_SKIP_THIS
enum EmberAfRf4ceGdpCommandCode
#else
typedef int8u EmberAfRf4ceGdpCommandCode;
enum
#endif
{
  EMBER_AF_RF4CE_GDP_COMMAND_GENERIC_RESPONSE         = 0x00,
  EMBER_AF_RF4CE_GDP_COMMAND_CONFIGURATION_COMPLETE   = 0x01,
  EMBER_AF_RF4CE_GDP_COMMAND_HEARTBEAT                = 0x02,
  EMBER_AF_RF4CE_GDP_COMMAND_GET_ATTRIBUTES           = 0x03,
  EMBER_AF_RF4CE_GDP_COMMAND_GET_ATTRIBUTES_RESPONSE  = 0x04,
  EMBER_AF_RF4CE_GDP_COMMAND_PUSH_ATTRIBUTES          = 0x05,
  EMBER_AF_RF4CE_GDP_COMMAND_SET_ATTRIBUTES           = 0x06,
  EMBER_AF_RF4CE_GDP_COMMAND_PULL_ATTRIBUTES          = 0x07,
  EMBER_AF_RF4CE_GDP_COMMAND_PULL_ATTRIBUTES_RESPONSE = 0x08,
  EMBER_AF_RF4CE_GDP_COMMAND_CHECK_VALIDATION         = 0x09,
  EMBER_AF_RF4CE_GDP_COMMAND_CLIENT_NOTIFICATION      = 0x0A,
  EMBER_AF_RF4CE_GDP_COMMAND_KEY_EXCHANGE             = 0x0B,
};

/**
 * @brief RF4CE GDP binding states.
 */
#ifdef DOXYGEN_SHOULD_SKIP_THIS
enum EmberAfRf4ceGdpBindingState
#else
typedef int8u EmberAfRf4ceGdpBindingState;
enum
#endif
{
  EMBER_AF_RF4CE_GDP_BINDING_STATE_DORMANT   = 0,
  EMBER_AF_RF4CE_GDP_BINDING_STATE_NOT_BOUND = 1,
  EMBER_AF_RF4CE_GDP_BINDING_STATE_BINDING   = 2,
  EMBER_AF_RF4CE_GDP_BINDING_STATE_BOUND     = 3,
};

/**
 * @brief RF4CE ZRC 1.1 command codes.
 */
#ifdef DOXYGEN_SHOULD_SKIP_THIS
enum EmberAfRf4ceZrc11CommandCode
#else
typedef int8u EmberAfRf4ceZrc11CommandCode;
enum
#endif
{
  EMBER_AF_RF4CE_ZRC_COMMAND_USER_CONTROL_PRESSED       = 0x01,
  EMBER_AF_RF4CE_ZRC_COMMAND_USER_CONTROL_REPEATED      = 0x02,
  EMBER_AF_RF4CE_ZRC_COMMAND_USER_CONTROL_RELEASED      = 0x03,
  EMBER_AF_RF4CE_ZRC_COMMAND_COMMAND_DISCOVERY_REQUEST  = 0x04,
  EMBER_AF_RF4CE_ZRC_COMMAND_COMMAND_DISCOVERY_RESPONSE = 0x05,
};

/**
 * @brief RF4CE user control codes from HDMI 1.3a CEC operand [UI Command].
 */
#ifdef DOXYGEN_SHOULD_SKIP_THIS
enum EmberAfRf4ceUserControlCode
#else
typedef int8u EmberAfRf4ceUserControlCode;
enum
#endif
{
  EMBER_AF_RF4CE_USER_CONTROL_CODE_SELECT                      = 0x00,
  EMBER_AF_RF4CE_USER_CONTROL_CODE_UP                          = 0x01,
  EMBER_AF_RF4CE_USER_CONTROL_CODE_DOWN                        = 0x02,
  EMBER_AF_RF4CE_USER_CONTROL_CODE_LEFT                        = 0x03,
  EMBER_AF_RF4CE_USER_CONTROL_CODE_RIGHT                       = 0x04,
  EMBER_AF_RF4CE_USER_CONTROL_CODE_RIGHT_UP                    = 0x05,
  EMBER_AF_RF4CE_USER_CONTROL_CODE_RIGHT_DOWN                  = 0x06,
  EMBER_AF_RF4CE_USER_CONTROL_CODE_LEFT_UP                     = 0x07,
  EMBER_AF_RF4CE_USER_CONTROL_CODE_LEFT_DOWN                   = 0x08,
  EMBER_AF_RF4CE_USER_CONTROL_CODE_ROOT_MENU                   = 0x09,
  EMBER_AF_RF4CE_USER_CONTROL_CODE_SETUP_MENU                  = 0x0A,
  EMBER_AF_RF4CE_USER_CONTROL_CODE_CONTENTS_MENU               = 0x0B,
  EMBER_AF_RF4CE_USER_CONTROL_CODE_FAVORITE_MENU               = 0x0C,
  EMBER_AF_RF4CE_USER_CONTROL_CODE_EXIT                        = 0x0D,
  EMBER_AF_RF4CE_USER_CONTROL_CODE_0                           = 0x20,
  EMBER_AF_RF4CE_USER_CONTROL_CODE_1                           = 0x21,
  EMBER_AF_RF4CE_USER_CONTROL_CODE_2                           = 0x22,
  EMBER_AF_RF4CE_USER_CONTROL_CODE_3                           = 0x23,
  EMBER_AF_RF4CE_USER_CONTROL_CODE_4                           = 0x24,
  EMBER_AF_RF4CE_USER_CONTROL_CODE_5                           = 0x25,
  EMBER_AF_RF4CE_USER_CONTROL_CODE_6                           = 0x26,
  EMBER_AF_RF4CE_USER_CONTROL_CODE_7                           = 0x27,
  EMBER_AF_RF4CE_USER_CONTROL_CODE_8                           = 0x28,
  EMBER_AF_RF4CE_USER_CONTROL_CODE_9                           = 0x29,
  EMBER_AF_RF4CE_USER_CONTROL_CODE_DOT                         = 0x2A,
  EMBER_AF_RF4CE_USER_CONTROL_CODE_ENTER                       = 0x2B,
  EMBER_AF_RF4CE_USER_CONTROL_CODE_CLEAR                       = 0x2C,
  EMBER_AF_RF4CE_USER_CONTROL_CODE_NEXT_FAVORITE               = 0x2F,
  EMBER_AF_RF4CE_USER_CONTROL_CODE_CHANNEL_UP                  = 0x30,
  EMBER_AF_RF4CE_USER_CONTROL_CODE_CHANNEL_DOWN                = 0x31,
  EMBER_AF_RF4CE_USER_CONTROL_CODE_PREVIOUS_CHANNEL            = 0x32,
  EMBER_AF_RF4CE_USER_CONTROL_CODE_SOUND_SELECT                = 0x33,
  EMBER_AF_RF4CE_USER_CONTROL_CODE_INPUT_SELECT                = 0x34,
  EMBER_AF_RF4CE_USER_CONTROL_CODE_DISPLAY_INFORMATION         = 0x35,
  EMBER_AF_RF4CE_USER_CONTROL_CODE_HELP                        = 0x36,
  EMBER_AF_RF4CE_USER_CONTROL_CODE_PAGE_UP                     = 0x37,
  EMBER_AF_RF4CE_USER_CONTROL_CODE_PAGE_DOWN                   = 0x38,
  EMBER_AF_RF4CE_USER_CONTROL_CODE_POWER                       = 0x40,
  EMBER_AF_RF4CE_USER_CONTROL_CODE_VOLUME_UP                   = 0x41,
  EMBER_AF_RF4CE_USER_CONTROL_CODE_VOLUME_DOWN                 = 0x42,
  EMBER_AF_RF4CE_USER_CONTROL_CODE_MUTE                        = 0x43,
  EMBER_AF_RF4CE_USER_CONTROL_CODE_PLAY                        = 0x44,
  EMBER_AF_RF4CE_USER_CONTROL_CODE_STOP                        = 0x45,
  EMBER_AF_RF4CE_USER_CONTROL_CODE_PAUSE                       = 0x46,
  EMBER_AF_RF4CE_USER_CONTROL_CODE_RECORD                      = 0x47,
  EMBER_AF_RF4CE_USER_CONTROL_CODE_REWIND                      = 0x48,
  EMBER_AF_RF4CE_USER_CONTROL_CODE_FAST_FORWARD                = 0x49,
  EMBER_AF_RF4CE_USER_CONTROL_CODE_EJECT                       = 0x4A,
  EMBER_AF_RF4CE_USER_CONTROL_CODE_FORWARD                     = 0x4B,
  EMBER_AF_RF4CE_USER_CONTROL_CODE_BACKWARD                    = 0x4C,
  EMBER_AF_RF4CE_USER_CONTROL_CODE_STOP_RECORD                 = 0x4D,
  EMBER_AF_RF4CE_USER_CONTROL_CODE_PAUSE_RECORD                = 0x4E,
  EMBER_AF_RF4CE_USER_CONTROL_CODE_ANGLE                       = 0x50,
  EMBER_AF_RF4CE_USER_CONTROL_CODE_SUB_PICTURE                 = 0x51,
  EMBER_AF_RF4CE_USER_CONTROL_CODE_VIDEO_ON_DEMAND             = 0x52,
  EMBER_AF_RF4CE_USER_CONTROL_CODE_ELECTRONIC_PROGRAM_GUIDE    = 0x53,
  EMBER_AF_RF4CE_USER_CONTROL_CODE_TIMER_PROGRAMMING           = 0x54,
  EMBER_AF_RF4CE_USER_CONTROL_CODE_INITIAL_CONFIGURATION       = 0x55,
  EMBER_AF_RF4CE_USER_CONTROL_CODE_PLAY_FUNCTION               = 0x60, // Play Mode - 1 byte
  EMBER_AF_RF4CE_USER_CONTROL_CODE_PAUSE_PLAY_FUNCTION         = 0x61,
  EMBER_AF_RF4CE_USER_CONTROL_CODE_RECORD_FUNCTION             = 0x62,
  EMBER_AF_RF4CE_USER_CONTROL_CODE_PAUSE_RECORD_FUNCTION       = 0x63,
  EMBER_AF_RF4CE_USER_CONTROL_CODE_STOP_FUNCTION               = 0x64,
  EMBER_AF_RF4CE_USER_CONTROL_CODE_MUTE_FUNCTION               = 0x65,
  EMBER_AF_RF4CE_USER_CONTROL_CODE_RESTORE_VOLUME_FUNCTION     = 0x66,
  EMBER_AF_RF4CE_USER_CONTROL_CODE_TUNE_FUNCTION               = 0x67, // Channel Identifier - 4 bytes
  EMBER_AF_RF4CE_USER_CONTROL_CODE_SELECT_MEDIA_FUNCTION       = 0x68, // UI Function Media - 1 byte
  EMBER_AF_RF4CE_USER_CONTROL_CODE_SELECT_A_V_INPUT_FUNCTION   = 0x69, // UI Function Select A/V Input - 1 byte
  EMBER_AF_RF4CE_USER_CONTROL_CODE_SELECT_AUDIO_INPUT_FUNCTION = 0x6A, // UI Function Select Audio Input - 1 byte
  EMBER_AF_RF4CE_USER_CONTROL_CODE_POWER_TOGGLE_FUNCTION       = 0x6B,
  EMBER_AF_RF4CE_USER_CONTROL_CODE_POWER_OFF_FUNCTION          = 0x6C,
  EMBER_AF_RF4CE_USER_CONTROL_CODE_POWER_ON_FUNCTION           = 0x6D,
  EMBER_AF_RF4CE_USER_CONTROL_CODE_F1_BLUE                     = 0x71,
  EMBER_AF_RF4CE_USER_CONTROL_CODE_F2_RED                      = 0x72,
  EMBER_AF_RF4CE_USER_CONTROL_CODE_F3_GREEN                    = 0x73,
  EMBER_AF_RF4CE_USER_CONTROL_CODE_F4_YELLOW                   = 0x74,
  EMBER_AF_RF4CE_USER_CONTROL_CODE_F5                          = 0x75,
  EMBER_AF_RF4CE_USER_CONTROL_CODE_DATA                        = 0x76,
};

/**
 * @brief RF4CE MSO binding states.
 */
#ifdef DOXYGEN_SHOULD_SKIP_THIS
enum EmberAfRf4ceMsoBindingState
#else
typedef int8u EmberAfRf4ceMsoBindingState;
enum
#endif
{
  EMBER_AF_RF4CE_MSO_BINDING_STATE_NOT_BOUND = 0,
  EMBER_AF_RF4CE_MSO_BINDING_STATE_BINDING   = 1,
  EMBER_AF_RF4CE_MSO_BINDING_STATE_BOUND     = 2,
};

/**
 * @brief RF4CE MSO validation states.
 */
#ifdef DOXYGEN_SHOULD_SKIP_THIS
enum EmberAfRf4ceMsoValidationState
#else
typedef int8u EmberAfRf4ceMsoValidationState;
enum
#endif
{
  EMBER_AF_RF4CE_MSO_VALIDATION_STATE_NOT_VALIDATED        = 0,
  EMBER_AF_RF4CE_MSO_VALIDATION_STATE_REJECTED             = 1,
  EMBER_AF_RF4CE_MSO_VALIDATION_STATE_VALIDATING           = 2,
  EMBER_AF_RF4CE_MSO_VALIDATION_STATE_REVALIDATING         = 3,
  EMBER_AF_RF4CE_MSO_VALIDATION_STATE_VALIDATED            = 4,
  //EMBER_AF_RF4CE_MSO_VALIDATION_STATE_PREVIOUSLY_VALIDATED = 5,
};

/**
 * @brief RF4CE MSO command codes.
 */
#ifdef DOXYGEN_SHOULD_SKIP_THIS
enum EmberAfRf4ceMsoCommandCode
#else
typedef int8u EmberAfRf4ceMsoCommandCode;
enum
#endif
{
  EMBER_AF_RF4CE_MSO_COMMAND_USER_CONTROL_PRESSED      = 0x01,
  EMBER_AF_RF4CE_MSO_COMMAND_USER_CONTROL_REPEATED     = 0x02,
  EMBER_AF_RF4CE_MSO_COMMAND_USER_CONTROL_RELEASED     = 0x03,
  EMBER_AF_RF4CE_MSO_COMMAND_CHECK_VALIDATION_REQUEST  = 0x20,
  EMBER_AF_RF4CE_MSO_COMMAND_CHECK_VALIDATION_RESPONSE = 0x21,
  EMBER_AF_RF4CE_MSO_COMMAND_SET_ATTRIBUTE_REQUEST     = 0x22,
  EMBER_AF_RF4CE_MSO_COMMAND_SET_ATTRIBUTE_RESPONSE    = 0x23,
  EMBER_AF_RF4CE_MSO_COMMAND_GET_ATTRIBUTE_REQUEST     = 0x24,
  EMBER_AF_RF4CE_MSO_COMMAND_GET_ATTRIBUTE_RESPONSE    = 0x25,
};

/**
 * @brief RF4CE MSO key codes.
 */
#ifdef DOXYGEN_SHOULD_SKIP_THIS
enum EmberAfRf4ceMsoKeyCode
#else
typedef int8u EmberAfRf4ceMsoKeyCode;
enum
#endif
{
  EMBER_AF_RF4CE_MSO_KEY_CODE_OK                                        = 0x00,
  EMBER_AF_RF4CE_MSO_KEY_CODE_UP_ARROW                                  = 0x01,
  EMBER_AF_RF4CE_MSO_KEY_CODE_DOWN_ARROW                                = 0x02,
  EMBER_AF_RF4CE_MSO_KEY_CODE_LEFT_ARROW                                = 0x03,
  EMBER_AF_RF4CE_MSO_KEY_CODE_RIGHT_ARROW                               = 0x04,
  EMBER_AF_RF4CE_MSO_KEY_CODE_MENU                                      = 0x09,
  EMBER_AF_RF4CE_MSO_KEY_CODE_DVR                                       = 0x0B,
  EMBER_AF_RF4CE_MSO_KEY_CODE_FAV                                       = 0x0C,
  EMBER_AF_RF4CE_MSO_KEY_CODE_EXIT                                      = 0x0D,
  EMBER_AF_RF4CE_MSO_KEY_CODE_HOME                                      = 0x10,
  EMBER_AF_RF4CE_MSO_KEY_CODE_DIGIT_0                                   = 0x20,
  EMBER_AF_RF4CE_MSO_KEY_CODE_DIGIT_1                                   = 0x21,
  EMBER_AF_RF4CE_MSO_KEY_CODE_DIGIT_2                                   = 0x22,
  EMBER_AF_RF4CE_MSO_KEY_CODE_DIGIT_3                                   = 0x23,
  EMBER_AF_RF4CE_MSO_KEY_CODE_DIGIT_4                                   = 0x24,
  EMBER_AF_RF4CE_MSO_KEY_CODE_DIGIT_5                                   = 0x25,
  EMBER_AF_RF4CE_MSO_KEY_CODE_DIGIT_6                                   = 0x26,
  EMBER_AF_RF4CE_MSO_KEY_CODE_DIGIT_7                                   = 0x27,
  EMBER_AF_RF4CE_MSO_KEY_CODE_DIGIT_8                                   = 0x28,
  EMBER_AF_RF4CE_MSO_KEY_CODE_DIGIT_9                                   = 0x29,
  EMBER_AF_RF4CE_MSO_KEY_CODE_FULL_STOP                                 = 0x2A,
  EMBER_AF_RF4CE_MSO_KEY_CODE_RETURN                                    = 0x2B,
  EMBER_AF_RF4CE_MSO_KEY_CODE_CHANNEL_UP                                = 0x30,
  EMBER_AF_RF4CE_MSO_KEY_CODE_CHANNEL_DOWN                              = 0x31,
  EMBER_AF_RF4CE_MSO_KEY_CODE_LAST                                      = 0x32,
  EMBER_AF_RF4CE_MSO_KEY_CODE_LANG                                      = 0x33,
  EMBER_AF_RF4CE_MSO_KEY_CODE_INPUT_SELECT                              = 0x34,
  EMBER_AF_RF4CE_MSO_KEY_CODE_INFO                                      = 0x35,
  EMBER_AF_RF4CE_MSO_KEY_CODE_HELP                                      = 0x36,
  EMBER_AF_RF4CE_MSO_KEY_CODE_PAGE_UP                                   = 0x37,
  EMBER_AF_RF4CE_MSO_KEY_CODE_PAGE_DOWN                                 = 0x38,
  EMBER_AF_RF4CE_MSO_KEY_CODE_MOTION                                    = 0x3B,
  EMBER_AF_RF4CE_MSO_KEY_CODE_SEARCH                                    = 0x3C,
  EMBER_AF_RF4CE_MSO_KEY_CODE_LIVE                                      = 0x3D,
  EMBER_AF_RF4CE_MSO_KEY_CODE_HD_ZOOM                                   = 0x3E,
  EMBER_AF_RF4CE_MSO_KEY_CODE_SHARE                                     = 0x3F,
  EMBER_AF_RF4CE_MSO_KEY_CODE_TV_POWER                                  = 0x40,
  EMBER_AF_RF4CE_MSO_KEY_CODE_VOLUME_UP                                 = 0x41,
  EMBER_AF_RF4CE_MSO_KEY_CODE_VOLUME_DOWN                               = 0x42,
  EMBER_AF_RF4CE_MSO_KEY_CODE_MUTE                                      = 0x43,
  EMBER_AF_RF4CE_MSO_KEY_CODE_PLAY                                      = 0x44,
  EMBER_AF_RF4CE_MSO_KEY_CODE_STOP                                      = 0x45,
  EMBER_AF_RF4CE_MSO_KEY_CODE_PAUSE                                     = 0x46,
  EMBER_AF_RF4CE_MSO_KEY_CODE_RECORD                                    = 0x47,
  EMBER_AF_RF4CE_MSO_KEY_CODE_REWIND                                    = 0x48,
  EMBER_AF_RF4CE_MSO_KEY_CODE_FAST_FORWARD                              = 0x49,
  EMBER_AF_RF4CE_MSO_KEY_CODE_30_SECOND_SKIP_AHEAD                      = 0x4B,
  EMBER_AF_RF4CE_MSO_KEY_CODE_REPLAY                                    = 0x4C,
  EMBER_AF_RF4CE_MSO_KEY_CODE_SWAP                                      = 0x51,
  EMBER_AF_RF4CE_MSO_KEY_CODE_ON_DEMAND                                 = 0x52,
  EMBER_AF_RF4CE_MSO_KEY_CODE_GUIDE                                     = 0x53,
  EMBER_AF_RF4CE_MSO_KEY_CODE_PUSH_TO_TALK                              = 0x57,
  EMBER_AF_RF4CE_MSO_KEY_CODE_PIP_ON_OFF                                = 0x58,
  EMBER_AF_RF4CE_MSO_KEY_CODE_PIP_MOVE                                  = 0x59,
  EMBER_AF_RF4CE_MSO_KEY_CODE_PIP_CHANNEL_UP                            = 0x5A,
  EMBER_AF_RF4CE_MSO_KEY_CODE_PIP_CHANNEL_DOWN                          = 0x5B,
  EMBER_AF_RF4CE_MSO_KEY_CODE_LOCK                                      = 0x5C,
  EMBER_AF_RF4CE_MSO_KEY_CODE_DAY_UP                                    = 0x5D,
  EMBER_AF_RF4CE_MSO_KEY_CODE_DAY_DOWN                                  = 0x5E,
  EMBER_AF_RF4CE_MSO_KEY_CODE_PLAY_PAUSE                                = 0x61,
  EMBER_AF_RF4CE_MSO_KEY_CODE_STOP_VIDEO                                = 0x64,
  EMBER_AF_RF4CE_MSO_KEY_CODE_MUTE_MIC                                  = 0x65,
  EMBER_AF_RF4CE_MSO_KEY_CODE_POWER_TOGGLE                              = 0x6B,
  EMBER_AF_RF4CE_MSO_KEY_CODE_POWER_OFF                                 = 0x6C,
  EMBER_AF_RF4CE_MSO_KEY_CODE_POWER_ON                                  = 0x6D,
  EMBER_AF_RF4CE_MSO_KEY_CODE_OCAP_B                                    = 0x71,
  EMBER_AF_RF4CE_MSO_KEY_CODE_BLUE_SQUARE                               = 0x71,
  EMBER_AF_RF4CE_MSO_KEY_CODE_OCAP_C                                    = 0x72,
  EMBER_AF_RF4CE_MSO_KEY_CODE_RED_CIRCLE                                = 0x72,
  EMBER_AF_RF4CE_MSO_KEY_CODE_OCAP_D                                    = 0x73,
  EMBER_AF_RF4CE_MSO_KEY_CODE_GREEN_DIAMOND                             = 0x73,
  EMBER_AF_RF4CE_MSO_KEY_CODE_OCAP_A                                    = 0x74,
  EMBER_AF_RF4CE_MSO_KEY_CODE_YELLOW_TRIANGLE                           = 0x74,
  EMBER_AF_RF4CE_MSO_KEY_CODE_PROFILE                                   = 0xA0,
  EMBER_AF_RF4CE_MSO_KEY_CODE_CALL                                      = 0xA1,
  EMBER_AF_RF4CE_MSO_KEY_CODE_HOLD                                      = 0xA2,
  EMBER_AF_RF4CE_MSO_KEY_CODE_END                                       = 0xA3,
  EMBER_AF_RF4CE_MSO_KEY_CODE_VIEWS                                     = 0xA4,
  EMBER_AF_RF4CE_MSO_KEY_CODE_SELF_VIEW                                 = 0xA5,
  EMBER_AF_RF4CE_MSO_KEY_CODE_ZOOM_IN                                   = 0xA6,
  EMBER_AF_RF4CE_MSO_KEY_CODE_ZOOM_OUT                                  = 0xA7,
  EMBER_AF_RF4CE_MSO_KEY_CODE_BACKSPACE                                 = 0xA8,
  EMBER_AF_RF4CE_MSO_KEY_CODE_LOCK_UNLOCK                               = 0xA9,
  EMBER_AF_RF4CE_MSO_KEY_CODE_CAPS                                      = 0xAA,
  EMBER_AF_RF4CE_MSO_KEY_CODE_ALT                                       = 0xAB,
  EMBER_AF_RF4CE_MSO_KEY_CODE_SPACE                                     = 0xAC,
  EMBER_AF_RF4CE_MSO_KEY_CODE_WWW_DOT                                   = 0xAD,
  EMBER_AF_RF4CE_MSO_KEY_CODE_DOT_COM                                   = 0xAE,
  EMBER_AF_RF4CE_MSO_KEY_CODE_LATIN_CAPITAL_LETTER_A                    = 0xB0,
  EMBER_AF_RF4CE_MSO_KEY_CODE_LATIN_CAPITAL_LETTER_B                    = 0xB1,
  EMBER_AF_RF4CE_MSO_KEY_CODE_LATIN_CAPITAL_LETTER_C                    = 0xB2,
  EMBER_AF_RF4CE_MSO_KEY_CODE_LATIN_CAPITAL_LETTER_D                    = 0xB3,
  EMBER_AF_RF4CE_MSO_KEY_CODE_LATIN_CAPITAL_LETTER_E                    = 0xB4,
  EMBER_AF_RF4CE_MSO_KEY_CODE_LATIN_CAPITAL_LETTER_F                    = 0xB5,
  EMBER_AF_RF4CE_MSO_KEY_CODE_LATIN_CAPITAL_LETTER_G                    = 0xB6,
  EMBER_AF_RF4CE_MSO_KEY_CODE_LATIN_CAPITAL_LETTER_H                    = 0xB7,
  EMBER_AF_RF4CE_MSO_KEY_CODE_LATIN_CAPITAL_LETTER_I                    = 0xB8,
  EMBER_AF_RF4CE_MSO_KEY_CODE_LATIN_CAPITAL_LETTER_J                    = 0xB9,
  EMBER_AF_RF4CE_MSO_KEY_CODE_LATIN_CAPITAL_LETTER_K                    = 0xBA,
  EMBER_AF_RF4CE_MSO_KEY_CODE_LATIN_CAPITAL_LETTER_L                    = 0xBB,
  EMBER_AF_RF4CE_MSO_KEY_CODE_LATIN_CAPITAL_LETTER_M                    = 0xBC,
  EMBER_AF_RF4CE_MSO_KEY_CODE_LATIN_CAPITAL_LETTER_N                    = 0xBD,
  EMBER_AF_RF4CE_MSO_KEY_CODE_LATIN_CAPITAL_LETTER_O                    = 0xBE,
  EMBER_AF_RF4CE_MSO_KEY_CODE_LATIN_CAPITAL_LETTER_P                    = 0xBF,
  EMBER_AF_RF4CE_MSO_KEY_CODE_LATIN_CAPITAL_LETTER_Q                    = 0xC0,
  EMBER_AF_RF4CE_MSO_KEY_CODE_LATIN_CAPITAL_LETTER_R                    = 0xC1,
  EMBER_AF_RF4CE_MSO_KEY_CODE_LATIN_CAPITAL_LETTER_S                    = 0xC2,
  EMBER_AF_RF4CE_MSO_KEY_CODE_LATIN_CAPITAL_LETTER_T                    = 0xC3,
  EMBER_AF_RF4CE_MSO_KEY_CODE_LATIN_CAPITAL_LETTER_U                    = 0xC4,
  EMBER_AF_RF4CE_MSO_KEY_CODE_LATIN_CAPITAL_LETTER_V                    = 0xC5,
  EMBER_AF_RF4CE_MSO_KEY_CODE_LATIN_CAPITAL_LETTER_W                    = 0xC6,
  EMBER_AF_RF4CE_MSO_KEY_CODE_LATIN_CAPITAL_LETTER_X                    = 0xC7,
  EMBER_AF_RF4CE_MSO_KEY_CODE_LATIN_CAPITAL_LETTER_Y                    = 0xC8,
  EMBER_AF_RF4CE_MSO_KEY_CODE_LATIN_CAPITAL_LETTER_Z                    = 0xC9,
  EMBER_AF_RF4CE_MSO_KEY_CODE_LATIN_SMALL_LETTER_A                      = 0xCA,
  EMBER_AF_RF4CE_MSO_KEY_CODE_LATIN_SMALL_LETTER_B                      = 0xCB,
  EMBER_AF_RF4CE_MSO_KEY_CODE_LATIN_SMALL_LETTER_C                      = 0xCC,
  EMBER_AF_RF4CE_MSO_KEY_CODE_LATIN_SMALL_LETTER_D                      = 0xCD,
  EMBER_AF_RF4CE_MSO_KEY_CODE_LATIN_SMALL_LETTER_E                      = 0xCE,
  EMBER_AF_RF4CE_MSO_KEY_CODE_LATIN_SMALL_LETTER_F                      = 0xCF,
  EMBER_AF_RF4CE_MSO_KEY_CODE_LATIN_SMALL_LETTER_G                      = 0xD0,
  EMBER_AF_RF4CE_MSO_KEY_CODE_LATIN_SMALL_LETTER_H                      = 0xD1,
  EMBER_AF_RF4CE_MSO_KEY_CODE_LATIN_SMALL_LETTER_I                      = 0xD2,
  EMBER_AF_RF4CE_MSO_KEY_CODE_LATIN_SMALL_LETTER_J                      = 0xD3,
  EMBER_AF_RF4CE_MSO_KEY_CODE_LATIN_SMALL_LETTER_K                      = 0xD4,
  EMBER_AF_RF4CE_MSO_KEY_CODE_LATIN_SMALL_LETTER_L                      = 0xD5,
  EMBER_AF_RF4CE_MSO_KEY_CODE_LATIN_SMALL_LETTER_M                      = 0xD6,
  EMBER_AF_RF4CE_MSO_KEY_CODE_LATIN_SMALL_LETTER_N                      = 0xD7,
  EMBER_AF_RF4CE_MSO_KEY_CODE_LATIN_SMALL_LETTER_O                      = 0xD8,
  EMBER_AF_RF4CE_MSO_KEY_CODE_LATIN_SMALL_LETTER_P                      = 0xD9,
  EMBER_AF_RF4CE_MSO_KEY_CODE_LATIN_SMALL_LETTER_Q                      = 0xDA,
  EMBER_AF_RF4CE_MSO_KEY_CODE_LATIN_SMALL_LETTER_R                      = 0xDB,
  EMBER_AF_RF4CE_MSO_KEY_CODE_LATIN_SMALL_LETTER_S                      = 0xDC,
  EMBER_AF_RF4CE_MSO_KEY_CODE_LATIN_SMALL_LETTER_T                      = 0xDD,
  EMBER_AF_RF4CE_MSO_KEY_CODE_LATIN_SMALL_LETTER_U                      = 0xDE,
  EMBER_AF_RF4CE_MSO_KEY_CODE_LATIN_SMALL_LETTER_V                      = 0xDF,
  EMBER_AF_RF4CE_MSO_KEY_CODE_LATIN_SMALL_LETTER_W                      = 0xE0,
  EMBER_AF_RF4CE_MSO_KEY_CODE_LATIN_SMALL_LETTER_X                      = 0xE1,
  EMBER_AF_RF4CE_MSO_KEY_CODE_LATIN_SMALL_LETTER_Y                      = 0xE2,
  EMBER_AF_RF4CE_MSO_KEY_CODE_LATIN_SMALL_LETTER_Z                      = 0xE3,
  EMBER_AF_RF4CE_MSO_KEY_CODE_QUESTION_MARK                             = 0xE4,
  EMBER_AF_RF4CE_MSO_KEY_CODE_EXCLAMATION_MARK                          = 0xE5,
  EMBER_AF_RF4CE_MSO_KEY_CODE_NUMBER_SIGN                               = 0xE6,
  EMBER_AF_RF4CE_MSO_KEY_CODE_DOLLAR_SIGN                               = 0xE7,
  EMBER_AF_RF4CE_MSO_KEY_CODE_PERCENT_SIGN                              = 0xE8,
  EMBER_AF_RF4CE_MSO_KEY_CODE_AMPERSAND                                 = 0xE9,
  EMBER_AF_RF4CE_MSO_KEY_CODE_ASTERISK                                  = 0xEA,
  EMBER_AF_RF4CE_MSO_KEY_CODE_LEFT_PARENTHESIS                          = 0xEB,
  EMBER_AF_RF4CE_MSO_KEY_CODE_RIGHT_PARENTHESIS                         = 0xEC,
  EMBER_AF_RF4CE_MSO_KEY_CODE_PLUS_SIGN                                 = 0xED,
  EMBER_AF_RF4CE_MSO_KEY_CODE_MINUS_SIGN                                = 0xEE,
  EMBER_AF_RF4CE_MSO_KEY_CODE_EQUALS_SIGN                               = 0xEF,
  EMBER_AF_RF4CE_MSO_KEY_CODE_SLASH                                     = 0xF0,
  EMBER_AF_RF4CE_MSO_KEY_CODE_UNDERSCORE                                = 0xF1,
  EMBER_AF_RF4CE_MSO_KEY_CODE_QUOTATION_MARK                            = 0xF2,
  EMBER_AF_RF4CE_MSO_KEY_CODE_COLON                                     = 0xF3,
  EMBER_AF_RF4CE_MSO_KEY_CODE_SEMICOLON                                 = 0xF4,
  EMBER_AF_RF4CE_MSO_KEY_CODE_AT_SIGN                                   = 0xF5,
  EMBER_AF_RF4CE_MSO_KEY_CODE_APOSTROPHE                                = 0xF6,
  EMBER_AF_RF4CE_MSO_KEY_CODE_COMMA                                     = 0xF7,
};

/**
 * @brief RF4CE MSO check validation statuses.
 */
#ifdef DOXYGEN_SHOULD_SKIP_THIS
enum EmberAfRf4ceMsoCheckValidationControl
#else
typedef int8u EmberAfRf4ceMsoCheckValidationControl;
enum
#endif
{
  EMBER_AF_RF4CE_MSO_CHECK_VALIDATION_CONTROL_REQUEST_AUTOMATIC_VALIDATION = BIT(0)
};

/**
 * @brief RF4CE MSO check validation statuses.
 */
#ifdef DOXYGEN_SHOULD_SKIP_THIS
enum EmberAfRf4ceMsoCheckValidationStatus
#else
typedef int8u EmberAfRf4ceMsoCheckValidationStatus;
enum
#endif
{
  EMBER_AF_RF4CE_MSO_CHECK_VALIDATION_STATUS_SUCCESS    = 0x00,
  EMBER_AF_RF4CE_MSO_CHECK_VALIDATION_STATUS_PENDING    = 0xC0,
  EMBER_AF_RF4CE_MSO_CHECK_VALIDATION_STATUS_TIMEOUT    = 0xC1,
  EMBER_AF_RF4CE_MSO_CHECK_VALIDATION_STATUS_COLLISION  = 0xC2,
  EMBER_AF_RF4CE_MSO_CHECK_VALIDATION_STATUS_FAILURE    = 0xC3,
  EMBER_AF_RF4CE_MSO_CHECK_VALIDATION_STATUS_ABORT      = 0xC4,
  EMBER_AF_RF4CE_MSO_CHECK_VALIDATION_STATUS_FULL_ABORT = 0xC5,
};

/*
* @brief RF4CE MSO binding statuses.
*/
#ifdef DOXYGEN_SHOULD_SKIP_THIS
enum EmberAfRf4ceMsoBindingStatus
#else
typedef int8u EmberAfRf4ceMsoBindingStatus;
enum
#endif
{
  EMBER_AF_RF4CE_MSO_BINDING_STATUS_SUCCESS                 = 0x00,
  EMBER_AF_RF4CE_MSO_BINDING_STATUS_NO_VALID_RESPONSE       = 0x01,
  EMBER_AF_RF4CE_MSO_BINDING_STATUS_NO_VALID_CANDIDATE      = 0x02,
  EMBER_AF_RF4CE_MSO_BINDING_STATUS_DUPLICATE_CLASS_ABORT   = 0x03,
  EMBER_AF_RF4CE_MSO_BINDING_STATUS_PAIRING_FAILED          = 0x04,
  EMBER_AF_RF4CE_MSO_BINDING_STATUS_VALIDATION_TIMEOUT      = EMBER_AF_RF4CE_MSO_CHECK_VALIDATION_STATUS_TIMEOUT,
  EMBER_AF_RF4CE_MSO_BINDING_STATUS_VALIDATION_COLLISION    = EMBER_AF_RF4CE_MSO_CHECK_VALIDATION_STATUS_COLLISION,
  EMBER_AF_RF4CE_MSO_BINDING_STATUS_VALIDATION_FAILURE      = EMBER_AF_RF4CE_MSO_CHECK_VALIDATION_STATUS_FAILURE,
  EMBER_AF_RF4CE_MSO_BINDING_STATUS_VALIDATION_ABORT        = EMBER_AF_RF4CE_MSO_CHECK_VALIDATION_STATUS_ABORT,
  EMBER_AF_RF4CE_MSO_BINDING_STATUS_VALIDATION_FULL_ABORT   = EMBER_AF_RF4CE_MSO_CHECK_VALIDATION_STATUS_FULL_ABORT,
};

/**
 * @brief RF4CE MSO attribute ids.
 */
#ifdef DOXYGEN_SHOULD_SKIP_THIS
enum EmberAfRf4ceMsoAttributeId
#else
typedef int8u EmberAfRf4ceMsoAttributeId;
enum
#endif
{
  EMBER_AF_RF4CE_MSO_ATTRIBUTE_ID_PERIPHERAL_IDS           = 0x00,
  EMBER_AF_RF4CE_MSO_ATTRIBUTE_ID_RF_STATISTICS            = 0x01,
  EMBER_AF_RF4CE_MSO_ATTRIBUTE_ID_VERSIONING               = 0x02,
  EMBER_AF_RF4CE_MSO_ATTRIBUTE_ID_BATTERY_STATUS           = 0x03,
  EMBER_AF_RF4CE_MSO_ATTRIBUTE_ID_SHORT_RF_RETRY_PERIOD    = 0x04,
  EMBER_AF_RF4CE_MSO_ATTRIBUTE_ID_IR_RF_DATABASE           = 0xDB,
  EMBER_AF_RF4CE_MSO_ATTRIBUTE_ID_VALIDATION_CONFIGURATION = 0xDC,
  EMBER_AF_RF4CE_MSO_ATTRIBUTE_ID_GENERAL_PURPOSE          = 0xFF,
};

/**
 * @brief CBKE Library types
 */
#ifdef DOXYGEN_SHOULD_SKIP_THIS
enum EmberAfCbkeKeyEstablishmentSuite
#else
typedef int16u EmberAfCbkeKeyEstablishmentSuite;
enum 
#endif
{
  EMBER_AF_INVALID_KEY_ESTABLISHMENT_SUITE      = 0x0000,
  EMBER_AF_CBKE_KEY_ESTABLISHMENT_SUITE_163K1   = 0x0001,
  EMBER_AF_CBKE_KEY_ESTABLISHMENT_SUITE_283K1   = 0x0002,
};

/** @} END addtogroup */

#endif // __AF_API_TYPES__
