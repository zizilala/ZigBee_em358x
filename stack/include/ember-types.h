/**
 * @file stack/include/ember-types.h
 * @brief Ember data type definitions.
 *
 *  See @ref ember_types for details.
 *
 * <!--Author(s): Richard Kelsey (richard@ember.com) --> 
 * <!--Lee Taylor (lee@ember.com) -->
 *
 * <!--Copyright 2006 by Ember Corporation. All rights reserved.         *80*-->
 */

/**
 * @addtogroup ember_types
 *
 * See ember-types.h for source code.
 * @{
 */

#ifndef EMBER_TYPES_H
#define EMBER_TYPES_H

#ifndef DOXYGEN_SHOULD_SKIP_THIS
#include "stack/config/ember-configuration-defaults.h"
#include "stack/include/ember-static-struct.h"
#endif //DOXYGEN_SHOULD_SKIP_THIS

/**
 * @name Miscellaneous Ember Types
 */
//@{

/**
 * @brief Type of Ember software version
 */
#ifdef DOXYGEN_SHOULD_SKIP_THIS
enum EmberVersionType
#else
typedef int8u EmberVersionType;
enum
#endif
{
  EMBER_VERSION_TYPE_PRE_RELEASE = 0x00,

	//Alpha, should be used rarely
  EMBER_VERSION_TYPE_ALPHA_1     = 0x11,
  EMBER_VERSION_TYPE_ALPHA_2     = 0x12,
  EMBER_VERSION_TYPE_ALPHA_3     = 0x13,
  // Leave space in case we decide to add other types in the future.
  EMBER_VERSION_TYPE_BETA_1      = 0x21,
  EMBER_VERSION_TYPE_BETA_2      = 0x22,
  EMBER_VERSION_TYPE_BETA_3      = 0x23,
  
 

  // Anything other than 0xAA is considered pre-release
  // We may define other types in the future (e.g. beta, alpha)
  // We chose an arbitrary number (0xAA) to allow for expansion, but
  // to prevent ambiguity in case 0x00 or 0xFF is accidentally retrieved
  // as the version type.
  EMBER_VERSION_TYPE_GA = 0xAA,
};

/** 
 * @brief A structure relating version types to human readable strings.
 */
typedef struct {
  EmberVersionType typeNum;
  PGM_P typeString;
} EmberReleaseTypeStruct;

/**
 * @brief ::EmberReleaseTypeStruct Data that relates release type to the correct string.
 */
#define EMBER_RELEASE_TYPE_TO_STRING_STRUCT_DATA     \
  { EMBER_VERSION_TYPE_PRE_RELEASE, "Pre-Release" }, \
  { EMBER_VERSION_TYPE_ALPHA_1,     "Alpha 1" },     \
  { EMBER_VERSION_TYPE_ALPHA_2,     "Alpha 2" },     \
  { EMBER_VERSION_TYPE_ALPHA_3,     "Alpha 3" },     \
  { EMBER_VERSION_TYPE_BETA_1,      "Beta 1" },      \
  { EMBER_VERSION_TYPE_BETA_2,      "Beta 2" },      \
  { EMBER_VERSION_TYPE_BETA_3,      "Beta 3" },      \
  { EMBER_VERSION_TYPE_GA,          "GA" },          \
  { 0xFF, NULL },
  

/**
 * @brief Version struct containing all version information
 */
typedef struct {
  int16u build;
  int8u major;
  int8u minor;
  int8u patch;
  int8u special;
  EmberVersionType type;
} EmberVersion;

/**
 * @brief Struct containing the version info
 */
extern const EmberVersion emberVersion;

/**
 * @brief Size of EUI64 (an IEEE address) in bytes (8).
 */
#define EUI64_SIZE 8

/**
 * @brief Size of an extended PAN identifier in bytes (8).
 */
#define EXTENDED_PAN_ID_SIZE 8

/**
 * @brief Size of an encryption key in bytes (16).
 */
#define EMBER_ENCRYPTION_KEY_SIZE 16

/**
 * @brief Size of Implicit Certificates used for Certificate Based
 * Key Exchange.
 */
#define EMBER_CERTIFICATE_SIZE 48

/**
 * @brief Size of Public Keys used in Elliptical Cryptography ECMQV algorithms.
 */
#define EMBER_PUBLIC_KEY_SIZE 22

/** 
 * @brief Size of Private Keys used in Elliptical Cryptography ECMQV algorithms.
 */
#define EMBER_PRIVATE_KEY_SIZE 21

/**
 * @brief Size of the SMAC used in Elliptical Cryptography ECMQV algorithms.
 */
#define EMBER_SMAC_SIZE 16

/**
 * @brief Size of the DSA signature used in Elliptical Cryptography 
 *   Digital Signature Algorithms. 
 */
#define EMBER_SIGNATURE_SIZE 42

/** @brief The size of AES-128 MMO hash is 16-bytes.  This is defined in the core.
 *  ZigBee specification.
 */
#define EMBER_AES_HASH_BLOCK_SIZE 16

/**
 * @brief Size of Implicit Certificates used for Certificate Based
 * Key Exchange using the ECC283K1 curve in bytes.
  */
#define EMBER_CERTIFICATE_283K1_SIZE 74

/**
 * @brief Size of Public Keys used in SECT283k1 Elliptical Cryptography ECMQV algorithms
 */
#define EMBER_PUBLIC_KEY_283K1_SIZE 37

/** 
 * @brief Size of Private Keys used SECT283k1 in Elliptical Cryptography ECMQV algorithms
 */
#define EMBER_PRIVATE_KEY_283K1_SIZE 36

/**
 * @brief Size of the DSA signature used in SECT283k1 Elliptical Cryptography 
 *   Digital Signature Algorithms. 
 */
#define EMBER_SIGNATURE_283K1_SIZE 72

/**
 * @brief  Return type for Ember functions.
 */
#ifndef __EMBERSTATUS_TYPE__
#define __EMBERSTATUS_TYPE__
  typedef int8u EmberStatus;
#endif //__EMBERSTATUS_TYPE__

/**
 * @brief EUI 64-bit ID (an IEEE address).
 */
typedef int8u EmberEUI64[EUI64_SIZE];

/**
 * @brief Incoming and outgoing messages are stored in buffers.
 * These buffers are allocated and freed as needed.
 *
 * Buffers are 32 bytes in length and can be linked together to hold
 * longer messages. 
 *
 * See packet-buffer.h for APIs related to stack and linked buffers.
 */
typedef int8u EmberMessageBuffer;

/**
 * @brief 16-bit ZigBee network address.
 */
typedef int16u EmberNodeId;

/** @brief 16-bit ZigBee multicast group identifier. */
typedef int16u EmberMulticastId;

/**
 * @brief 802.15.4 PAN ID. 
 */
typedef int16u EmberPanId;

/**
 * @brief The maximum 802.15.4 channel number is 26.
 */
#define EMBER_MAX_802_15_4_CHANNEL_NUMBER 26

/**
 * @brief The minimum 802.15.4 channel number is 11.
 */
#define EMBER_MIN_802_15_4_CHANNEL_NUMBER 11

/**
 * @brief There are sixteen 802.15.4 channels.
 */
#define EMBER_NUM_802_15_4_CHANNELS \
  (EMBER_MAX_802_15_4_CHANNEL_NUMBER - EMBER_MIN_802_15_4_CHANNEL_NUMBER + 1)

/**
 * @brief Bitmask to scan all 802.15.4 channels.
 */
#define EMBER_ALL_802_15_4_CHANNELS_MASK 0x07FFF800UL

/**
 * @brief The network ID of the coordinator in a ZigBee network is 0x0000.
 */
#define EMBER_ZIGBEE_COORDINATOR_ADDRESS 0x0000

/**
 * @brief A distinguished network ID that will never be assigned
 * to any node.  Used to indicate the absence of a node ID.
 */
#define EMBER_NULL_NODE_ID 0xFFFF

/**
 * @brief A distinguished binding index used to indicate the absence
 * of a binding.
 */
#define EMBER_NULL_BINDING 0xFF

/**
 * @brief A distinguished network ID that will never be assigned
 * to any node.  
 *
 * This value is used when setting or getting the remote
 * node ID in the address table or getting the remote node ID from the
 * binding table.  It indicates that address or binding table entry is
 * not in use.
 */
#define EMBER_TABLE_ENTRY_UNUSED_NODE_ID  0xFFFF

/**
 * @brief A distinguished network ID that will never be assigned
 * to any node.  This value is returned when getting the remote node
 * ID from the binding table and the given binding table index refers
 * to a multicast binding entry.
 */
#define EMBER_MULTICAST_NODE_ID           0xFFFE

/**
 * @brief A distinguished network ID that will never be assigned
 * to any node.  This value is used when getting the remote node ID
 * from the address or binding tables.  It indicates that the address
 * or binding table entry is currently in use but the node ID
 * corresponding to the EUI64 in the table is currently unknown.
 */
#define EMBER_UNKNOWN_NODE_ID             0xFFFD

/**
 * @brief A distinguished network ID that will never be assigned
 * to any node.  This value is used when getting the remote node ID
 * from the address or binding tables.  It indicates that the address
 * or binding table entry is currently in use and network address
 * discovery is underway.
 */
#define EMBER_DISCOVERY_ACTIVE_NODE_ID    0xFFFC

/**
 * @brief A distinguished address table index used to indicate the 
 * absence of an address table entry.
 */
#define EMBER_NULL_ADDRESS_TABLE_INDEX 0xFF

/**
 * @brief The endpoint where the ZigBee Device Object (ZDO) resides.
 */
#define EMBER_ZDO_ENDPOINT 0

/**
 * @brief The broadcast endpoint, as defined in the ZigBee spec.
 */
#define EMBER_BROADCAST_ENDPOINT 0xFF

/**
 * @brief The profile ID used by the ZigBee Device Object (ZDO).
 */
#define EMBER_ZDO_PROFILE_ID  0x0000

/**
 * @brief The profile ID used to address all the public profiles.
 */
#define EMBER_WILDCARD_PROFILE_ID  0xFFFF

/**
 * @brief The maximum value for a profile ID in the standard profile range.
 */
#define EMBER_MAXIMUM_STANDARD_PROFILE_ID  0x7FFF

/**
 * @brief The broadcast table timeout.  How long a broadcast entry persists in
 *   the local device's broadcast table.  This is the maximum length it will
 *   persist, in quarter seconds.
 */
#define EMBER_BROADCAST_TABLE_TIMEOUT_QS (20 * 4)


/**
 * @brief Ember's Manufacturer ID
 */
#define EMBER_MANUFACTURER_ID 0x1002


#ifdef DOXYGEN_SHOULD_SKIP_THIS
enum EmberLeaveRequestFlags
#else
typedef int8u EmberLeaveRequestFlags;
enum
#endif
{
  /** Leave and rejoin */
  EMBER_ZIGBEE_LEAVE_AND_REJOIN          = 0x20,

  /** Send all children leave command */
  EMBER_ZIGBEE_LEAVE_AND_REMOVE_CHILDREN = 0x40,
};

#ifdef DOXYGEN_SHOULD_SKIP_THIS
enum EmberLeaveReason
#else
typedef int8u EmberLeaveReason;
enum
#endif
{
  EMBER_LEAVE_REASON_NONE               = 0,
  EMBER_LEAVE_DUE_TO_NWK_LEAVE_MESSAGE  = 1,
  EMBER_LEAVE_DUE_TO_APS_REMOVE_MESSAGE = 2,
  // Currently, the stack does not process the ZDO leave message since it is optional
  EMBER_LEAVE_DUE_TO_ZDO_LEAVE_MESSAGE  = 3,
  EMBER_LEAVE_DUE_TO_ZLL_TOUCHLINK      = 4,

  EMBER_LEAVE_DUE_TO_APP_EVENT_1        = 0xFF,
};

//@} \\END Misc. Ember Types


/**
 * @name ZigBee Broadcast Addresses
 *@{
 *  ZigBee specifies three different broadcast addresses that
 *  reach different collections of nodes.  Broadcasts are normally sent only
 *  to routers.  Broadcasts can also be forwarded to end devices, either
 *  all of them or only those that do not sleep.  Broadcasting to end
 *  devices is both significantly more resource-intensive and significantly
 *  less reliable than broadcasting to routers.
 */

/** Broadcast to all routers. */
#define EMBER_BROADCAST_ADDRESS 0xFFFC
/** Broadcast to all non-sleepy devices. */
#define EMBER_RX_ON_WHEN_IDLE_BROADCAST_ADDRESS 0xFFFD
/** Broadcast to all devices, including sleepy end devices. */
#define EMBER_SLEEPY_BROADCAST_ADDRESS 0xFFFF

/** @} END Broadcast Addresses */


/**
 * @brief Defines the possible types of nodes and the roles that a 
 * node might play in a network.
 */
#ifdef DOXYGEN_SHOULD_SKIP_THIS
enum EmberNodeType
#else
typedef int8u EmberNodeType;
enum
#endif
{
  /** Device is not joined */
  EMBER_UNKNOWN_DEVICE = 0,
  /** Will relay messages and can act as a parent to other nodes. */
  EMBER_COORDINATOR = 1,
  /** Will relay messages and can act as a parent to other nodes. */
  EMBER_ROUTER = 2,
  /** Communicates only with its parent and will not relay messages. */
  EMBER_END_DEVICE = 3,
  /** An end device whose radio can be turned off to save power.
   *  The application must call ::emberPollForData() to receive messages.
   */
  EMBER_SLEEPY_END_DEVICE = 4,
  /** A sleepy end device that can move through the network. */
  EMBER_MOBILE_END_DEVICE = 5
};

/**
 * @brief Defines a ZigBee network and the associated parameters.
 */
typedef struct {
  int16u panId;
  int8u channel;
  boolean allowingJoin;
  int8u extendedPanId[8];
  int8u stackProfile;
  int8u nwkUpdateId;
} EmberZigbeeNetwork;


/**
 * @brief Defines the options that should be used when initializing the
 *   node's network configuration.
 */
#ifdef DOXYGEN_SHOULD_SKIP_THIS
enum EmberNetworkInitBitmask
#else
typedef int16u EmberNetworkInitBitmask;
enum
#endif
{
  EMBER_NETWORK_INIT_NO_OPTIONS           = 0x0000,
  /** The Parent Node ID and EUI64 are stored in a token.
      This prevents the need to perform an Orphan scan on startup.
  */
  EMBER_NETWORK_INIT_PARENT_INFO_IN_TOKEN = 0x0001,
};


/**
 * @brief Defines the network initialization configuration that should be used
 *   when ::emberNetworkInitExtended() is called by the application.
 */
typedef struct {
  EmberNetworkInitBitmask bitmask;
} EmberNetworkInitStruct;


/**
 * @brief Options to use when sending a message.
 * 
 * The discover route, APS retry, and APS indirect options may be used together.
 * Poll response cannot be combined with any other options.
 */
#ifdef DOXYGEN_SHOULD_SKIP_THIS
enum EmberApsOption
#else
typedef int16u EmberApsOption;
enum
#endif
{
  /** No options. */
  EMBER_APS_OPTION_NONE                     = 0x0000,
  /** This signs the application layer message body (APS Frame not included)
      and appends the ECDSA signature to the end of the message.  Needed by
      Smart Energy applications.  This requires the CBKE and ECC libraries.
      The ::emberDsaSignHandler() function is called after DSA signing
      is complete but before the message has been sent by the APS layer.
      Note that when passing a buffer to the stack for DSA signing, the final
      byte in the buffer has special significance as an indicator of how many
      leading bytes should be ignored for signature purposes.  Refer to API
      documentation of emberDsaSign() or the dsaSign EZSP command for further
      details about this requirement.
  */
  EMBER_APS_OPTION_DSA_SIGN                 = 0x0010,
  /** Send the message using APS Encryption, using the Link Key shared
      with the destination node to encrypt the data at the APS Level. */
  EMBER_APS_OPTION_ENCRYPTION               = 0x0020,
  /** Resend the message using the APS retry mechanism.  In the mesh stack,
      this option and the enable route discovery option must be enabled for 
      an existing route to be repaired automatically. */
  EMBER_APS_OPTION_RETRY                    = 0x0040,
  /** Send the message with the NWK 'enable route discovery' flag, which
      causes a route discovery to be initiated if no route to the destination
      is known.  Note that in the mesh stack, this option and the APS retry 
      option must be enabled an existing route to be repaired 
      automatically. */
  EMBER_APS_OPTION_ENABLE_ROUTE_DISCOVERY   = 0x0100,
  /** Send the message with the NWK 'force route discovery' flag, which causes
      a route discovery to be initiated even if one is known. */
  EMBER_APS_OPTION_FORCE_ROUTE_DISCOVERY    = 0x0200,
  /** Include the source EUI64 in the network frame. */
  EMBER_APS_OPTION_SOURCE_EUI64             = 0x0400,
  /** Include the destination EUI64 in the network frame. */
  EMBER_APS_OPTION_DESTINATION_EUI64        = 0x0800,
  /** Send a ZDO request to discover the node ID of the destination, if it is
      not already know. */
  EMBER_APS_OPTION_ENABLE_ADDRESS_DISCOVERY = 0x1000,
  /** This message is being sent in response to a call to
      ::emberPollHandler().  It causes the message to be sent
      immediately instead of being queued up until the next poll from the
      (end device) destination. */
  EMBER_APS_OPTION_POLL_RESPONSE            = 0x2000,
  /** This incoming message is a valid ZDO request and the application 
   * is responsible for sending a ZDO response. This flag is used only
   * within emberIncomingMessageHandler() when 
   * EMBER_APPLICATION_RECEIVES_UNSUPPORTED_ZDO_REQUESTS is defined. */
  EMBER_APS_OPTION_ZDO_RESPONSE_REQUIRED    = 0x4000,
  /** This message is part of a fragmented message.  This option may only
      be set for unicasts.  The groupId field gives the index of this
      fragment in the low-order byte.  If the low-order byte is zero this
      is the first fragment and the high-order byte contains the number
      of fragments in the message. */
  EMBER_APS_OPTION_FRAGMENT                 = SIGNED_ENUM 0x8000
};



/**
 * @brief Defines the possible incoming message types.
 */
#ifdef DOXYGEN_SHOULD_SKIP_THIS
enum EmberIncomingMessageType
#else
typedef int8u EmberIncomingMessageType;
enum
#endif
{
  /** Unicast. */
  EMBER_INCOMING_UNICAST,
  /** Unicast reply. */
  EMBER_INCOMING_UNICAST_REPLY,
  /** Multicast. */
  EMBER_INCOMING_MULTICAST,
  /** Multicast sent by the local device. */
  EMBER_INCOMING_MULTICAST_LOOPBACK,
  /** Broadcast. */
  EMBER_INCOMING_BROADCAST,
  /** Broadcast sent by the local device. */
  EMBER_INCOMING_BROADCAST_LOOPBACK
};


/**
 * @brief Defines the possible outgoing message types.
 */
#ifdef DOXYGEN_SHOULD_SKIP_THIS
enum EmberOutgoingMessageType
#else
typedef int8u EmberOutgoingMessageType;
enum
#endif
{
  /** Unicast sent directly to an EmberNodeId. */
  EMBER_OUTGOING_DIRECT,
  /** Unicast sent using an entry in the address table. */
  EMBER_OUTGOING_VIA_ADDRESS_TABLE,
  /** Unicast sent using an entry in the binding table. */
  EMBER_OUTGOING_VIA_BINDING,
  /** Multicast message.  This value is passed to emberMessageSentHandler() only.
   * It may not be passed to emberSendUnicast(). */
  EMBER_OUTGOING_MULTICAST,
  /** Broadcast message.  This value is passed to emberMessageSentHandler() only.
   * It may not be passed to emberSendUnicast(). */
  EMBER_OUTGOING_BROADCAST
};


/**
 * @brief Defines the possible join states for a node.
 */
#ifdef DOXYGEN_SHOULD_SKIP_THIS
enum EmberNetworkStatus
#else
typedef int8u EmberNetworkStatus;
enum
#endif
{
  /** The node is not associated with a network in any way. */
  EMBER_NO_NETWORK,
  /** The node is currently attempting to join a network. */
  EMBER_JOINING_NETWORK,
  /** The node is joined to a network. */
  EMBER_JOINED_NETWORK,
  /** The node is an end device joined to a network but its parent
      is not responding. */
  EMBER_JOINED_NETWORK_NO_PARENT,
  /** The node is in the process of leaving its current network. */
  EMBER_LEAVING_NETWORK
};


/**
 * @brief Type for a network scan.
 */
#ifdef DOXYGEN_SHOULD_SKIP_THIS
enum EmberNetworkScanType
#else
typedef int8u EmberNetworkScanType;
enum
#endif
{
  /** An energy scan scans each channel for its RSSI value. */
  EMBER_ENERGY_SCAN,
  /** An active scan scans each channel for available networks. */
  EMBER_ACTIVE_SCAN
};


/**
 * @brief Defines binding types.
 */
#ifdef DOXYGEN_SHOULD_SKIP_THIS
enum EmberBindingType
#else
typedef int8u EmberBindingType;
enum
#endif
{
  /** A binding that is currently not in use. */
  EMBER_UNUSED_BINDING         = 0,
  /** A unicast binding whose 64-bit identifier is the destination EUI64. */
  EMBER_UNICAST_BINDING        = 1,
  /** A unicast binding whose 64-bit identifier is the many-to-one
   * destination EUI64.  Route discovery should be disabled when sending
   * unicasts via many-to-one bindings. */
  EMBER_MANY_TO_ONE_BINDING    = 2,
  /** A multicast binding whose 64-bit identifier is the group address. A
   * multicast binding can be used to send messages to the group and to receive
   * messages sent to the group. */
  EMBER_MULTICAST_BINDING      = 3,
};


/**
 * @name Ember Concentrator Types
 *@{
 */

/** A concentrator with insufficient memory to store source routes for
 * the entire network. Route records are sent to the concentrator prior
 * to every inbound APS unicast. */
#define EMBER_LOW_RAM_CONCENTRATOR 0xFFF8
/** A concentrator with sufficient memory to store source routes for
 * the entire network. Remote nodes stop sending route records once
 * the concentrator has successfully received one.
 */
#define EMBER_HIGH_RAM_CONCENTRATOR 0xFFF9

//@} \\END Ember Concentrator Types


/**
 * @brief Decision made by the Trust Center when a node attempts to join.
 */
#ifdef DOXYGEN_SHOULD_SKIP_THIS
enum EmberJoinDecision
#else
typedef int8u EmberJoinDecision;
enum
#endif
{
  /** Allow the node to join. The node has the key. */
  EMBER_USE_PRECONFIGURED_KEY = 0,
  /** Allow the node to join. Send the key to the node. */
  EMBER_SEND_KEY_IN_THE_CLEAR,
  /** Deny join. */
  EMBER_DENY_JOIN,
  /** Take no action. */
  EMBER_NO_ACTION
};

/**
 * @ brief Defines the CLI enumerations for the ::EmberJoinDecision enum
 */
#define EMBER_JOIN_DECISION_STRINGS \
  "use preconfigured key",          \
  "send key in the clear",          \
  "deny join",                      \
  "no action",


/** @brief The Status of the Update Device message sent to the Trust Center.
 *  The device may have joined or rejoined insecurely, rejoined securely, or
 *  left.  MAC Security has been deprecated and therefore there is no secure
 *  join.
 */
// These map to the actual values within the APS Command frame so they cannot
// be arbitrarily changed.
#ifdef DOXYGEN_SHOULD_SKIP_THIS
enum EmberDeviceUpdate
#else
typedef int8u EmberDeviceUpdate;
enum
#endif
{
  EMBER_STANDARD_SECURITY_SECURED_REJOIN   = 0,
  EMBER_STANDARD_SECURITY_UNSECURED_JOIN   = 1,
  EMBER_DEVICE_LEFT                        = 2,
  EMBER_STANDARD_SECURITY_UNSECURED_REJOIN = 3,
  EMBER_HIGH_SECURITY_SECURED_REJOIN       = 4,
  EMBER_HIGH_SECURITY_UNSECURED_JOIN       = 5,
  /* 6 Reserved */
  EMBER_HIGH_SECURITY_UNSECURED_REJOIN     = 7,
  /* 8 - 15 Reserved */
};

/**
 * @ brief Defines the CLI enumerations for the ::EmberDeviceUpdate enum.
 */
#define EMBER_DEVICE_UPDATE_STRINGS                                         \
    "secured rejoin",                                                       \
    "UNsecured join",                                                       \
    "device left",                                                          \
    "UNsecured rejoin",                                                     \
    "high secured rejoin",                                                  \
    "high UNsecured join",                                                  \
    "RESERVED",                   /* reserved status code, per the spec. */ \
    "high UNsecured rejoin",

/**
 * @brief Notes the last rejoin reason.
 */
#ifdef DOXYGEN_SHOULD_SKIP_THIS
enum EmberRejoinReason
#else
typedef int8u EmberRejoinReason;
enum
#endif
{
  EMBER_REJOIN_REASON_NONE           = 0,
  EMBER_REJOIN_DUE_TO_NWK_KEY_UPDATE = 1,
  EMBER_REJOIN_DUE_TO_LEAVE_MESSAGE  = 2,
  EMBER_REJOIN_DUE_TO_NO_PARENT      = 3,
  EMBER_REJOIN_DUE_TO_ZLL_TOUCHLINK  = 4,

  // App. Framework events
  // 0xA0 - 0xE0

  // Customer Defined Events
  //   I numbered these backwards in case there is ever request
  //   for more application events. We can expand them
  //   without renumbering the previous ones.
  EMBER_REJOIN_DUE_TO_APP_EVENT_5       = 0xFB,
  EMBER_REJOIN_DUE_TO_APP_EVENT_4       = 0xFC,
  EMBER_REJOIN_DUE_TO_APP_EVENT_3       = 0xFD,
  EMBER_REJOIN_DUE_TO_APP_EVENT_2       = 0xFE,
  EMBER_REJOIN_DUE_TO_APP_EVENT_1       = 0xFF,
};

/**
 * @brief Defines the lists of clusters that must be provided for each endpoint.
 */
#ifdef DOXYGEN_SHOULD_SKIP_THIS
enum EmberClusterListId
#else
typedef int8u EmberClusterListId;
enum
#endif
{
  /** Input clusters the endpoint will accept. */
  EMBER_INPUT_CLUSTER_LIST            = 0,
  /** Output clusters the endpoint can send. */
  EMBER_OUTPUT_CLUSTER_LIST           = 1
};


/**
 * @brief Either marks an event as inactive or specifies the units for the
 * event execution time.
 */
#ifdef DOXYGEN_SHOULD_SKIP_THIS
enum EmberEventUnits
#else
typedef int8u EmberEventUnits;
enum
#endif
{
  /** The event is not scheduled to run. */
  EMBER_EVENT_INACTIVE = 0,
  /** The execution time is in approximate milliseconds.  */
  EMBER_EVENT_MS_TIME,
  /** The execution time is in 'binary' quarter seconds (256 approximate
      milliseconds each). */
  EMBER_EVENT_QS_TIME,
  /** The execution time is in 'binary' minutes (65536 approximate milliseconds
      each). */
  EMBER_EVENT_MINUTE_TIME,
  /** The event is scheduled to run at the earliest opportunity. */
  EMBER_EVENT_ZERO_DELAY
};


/** @brief The type of method used for joining.
 *
 */
#ifdef DOXYGEN_SHOULD_SKIP_THIS
enum EmberJoinMethod
#else
typedef int8u EmberJoinMethod;
enum
#endif
{
  /** Normally devices use MAC Association to join a network, which respects
   *  the "permit joining" flag in the MAC Beacon.  For mobile nodes this value
   *  causes the device to use an Ember Mobile Node Join, which is functionally
   *  equivalent to a MAC association.  This value should be used by default.
   */
  EMBER_USE_MAC_ASSOCIATION         = 0,

  /** For those networks where the "permit joining" flag is never turned
   *  on, they will need to use a ZigBee NWK Rejoin.  This value causes the 
   *  rejoin to be sent withOUT NWK security and the Trust Center will be
   *  asked to send the NWK key to the device.  The NWK key sent to the device
   *  can be encrypted with the device's corresponding Trust Center link key.
   *  That is determined by the ::EmberJoinDecision on the Trust Center
   *  returned by the ::emberTrustCenterJoinHandler().  
   *  For a mobile node this value will cause it to use an Ember Mobile node
   *  rejoin, which is functionally equivalent.
   */
  EMBER_USE_NWK_REJOIN              = 1,


  /* For those networks where the "permit joining" flag is never turned
   * on, they will need to use a NWK Rejoin.  If those devices have been 
   * preconfigured with the  NWK key (including sequence number) they can use
   * a secured rejoin.  This is only necessary for end devices since they need
   * a parent.  Routers can simply use the ::EMBER_USE_NWK_COMMISSIONING
   * join method below.
   */
  EMBER_USE_NWK_REJOIN_HAVE_NWK_KEY = 2,

  /** For those networks where all network and security information is known
      ahead of time, a router device may be commissioned such that it does
      not need to send any messages to begin communicating on the network.
  */
  EMBER_USE_NWK_COMMISSIONING       = 3,
};


/** @brief Holds network parameters.
 *
 * For information about power settings and radio channels, 
 * see the technical specification for the
 * RF communication module in your Developer Kit. 
 */
typedef struct {
  /** The network's extended PAN identifier.*/
  int8u   extendedPanId[8];
  /** The network's PAN identifier.*/
  int16u  panId;
  /** A power setting, in dBm.*/
  int8s   radioTxPower;
  /** A radio channel. Be sure to specify a channel supported by the radio. */
  int8u   radioChannel;
  /** Join method: The protocol messages used to establish an initial parent.  It is
   *  ignored when forming a ZigBee network, or when querying the stack for its
      network parameters.
   */
  EmberJoinMethod joinMethod;

  /** NWK Manager ID.  The ID of the network manager in the current network. 
      This may only be set at joining when using EMBER_USE_NWK_COMMISSIONING as
      the join method. 
  */
  EmberNodeId nwkManagerId;
  /** NWK Update ID.  The value of the ZigBee nwkUpdateId known by the stack.
      This is used to determine the newest instance of the network after a PAN
      ID or channel change.  This may only be set at joining when using 
      EMBER_USE_NWK_COMMISSIONING as the join method.
  */
  int8u nwkUpdateId;
  /** NWK channel mask.  The list of preferred channels that the NWK manager
      has told this device to use when searching for the network.
      This may only be set at joining when using EMBER_USE_NWK_COMMISSIONING as
      the join method. 
  */
  int32u channels;
} EmberNetworkParameters;


#ifdef DOXYGEN_SHOULD_SKIP_THIS
#define emberInitializeNetworkParameters(parameters) \
  (MEMSET(parameters, 0, sizeof(EmberNetworkParameters)))
#else
void emberInitializeNetworkParameters(EmberNetworkParameters* parameters);
#endif

/** @brief An in-memory representation of a ZigBee APS frame
 * of an incoming or outgoing message.
 */
typedef struct {
  /** The application profile ID that describes the format of the message. */
  int16u profileId;
  /** The cluster ID for this message. */
  int16u clusterId;
  /** The source endpoint. */
  int8u sourceEndpoint;
  /** The destination endpoint. */
  int8u destinationEndpoint;
  /** A bitmask of options from the enumeration above. */
  EmberApsOption options;
  /** The group ID for this message, if it is multicast mode. */
  int16u groupId;
  /** The sequence number. */
  int8u sequence;
} EmberApsFrame;


/** @brief Defines an entry in the binding table.
 *
 * A binding entry specifies a local endpoint, a remote endpoint, a
 * cluster ID and either the destination EUI64 (for unicast bindings) or the
 * 64-bit group address (for multicast bindings).
 */
typedef struct {
  /** The type of binding. */
  EmberBindingType type;
  /** The endpoint on the local node. */
  int8u local;
  /** A cluster ID that matches one from the local endpoint's simple descriptor.
   * This cluster ID is set by the provisioning application to indicate which
   * part an endpoint's functionality is bound to this particular remote node
   * and is used to distinguish between unicast and multicast bindings. Note
   * that a binding can be used to to send messages with any cluster ID, not
   * just that listed in the binding.
   */
  int16u clusterId;
  /** The endpoint on the remote node (specified by \c identifier). */
  int8u remote;
  /** A 64-bit identifier.  This is either:
   * - The destination EUI64, for unicasts
   * - A 16-bit multicast group address, for multicasts
   */
  EmberEUI64 identifier;
  /** The index of the network the binding belongs to. */
  int8u networkIndex;
} EmberBindingTableEntry;


/** @brief Defines an entry in the neighbor table.
 * 
 * A neighbor table entry stores information about the
 * reliability of RF links to and from neighboring nodes.
 */
typedef struct {
  /** The neighbor's two byte network id.*/
  int16u shortId;
  /** An exponentially weighted moving average of the link quality
   *  values of incoming packets from this neighbor as reported by the PHY.*/
  int8u  averageLqi;
  /** The incoming cost for this neighbor, computed from the average LQI. 
   *  Values range from 1 for a good link to 7 for a bad link.*/
  int8u  inCost;
  /** The outgoing cost for this neighbor, obtained from the most recently
   *  received neighbor exchange message from the neighbor.  A value of zero
   *  means that a neighbor exchange message from the neighbor has not been
   *  received recently enough, or that our id was not present in the most
   *  recently received one.  EmberZNet Pro only.
   */
  int8u  outCost;
  /** In EmberZNet Pro, the number of aging periods elapsed since a neighbor 
   *  exchange message was last received from this neighbor.  In stack profile 1,
   *  the number of aging periods since any packet was received.
   *  An entry with an age greater than 3 is considered stale and may be 
   *  reclaimed.  The aging period is 16 seconds.*/
  int8u  age;
  /** The 8 byte EUI64 of the neighbor. */
  EmberEUI64 longId;
} EmberNeighborTableEntry;

/** @brief Defines an entry in the route table.
 * 
 * A route table entry stores information about the next
 * hop along the route to the destination.
 */
typedef struct {
  /** The short id of the destination. */
  int16u destination;
  /** The short id of the next hop to this destination. */
  int16u nextHop;
  /** Indicates whether this entry is active (0), being discovered (1), 
   * or unused (3). */
  int8u status;
  /** The number of seconds since this route entry was last used to send
   * a packet. */
  int8u age;
  /** Indicates whether this destination is a High RAM Concentrator (2),
   * a Low RAM Concentrator (1), or not a concentrator (0). */
  int8u concentratorType;
  /** For a High RAM Concentrator, indicates whether a route record
   * is needed (2), has been sent (1), or is no long needed (0) because
   * a source routed message from the concentrator has been received.
   */
  int8u routeRecordState;
} EmberRouteTableEntry;

/** @brief Defines an entry in the multicast table.
 * 
 * A multicast table entry indicates that a particular
 * endpoint is a member of a particular multicast group.  Only devices
 * with an endpoint in a multicast group will receive messages sent to
 * that multicast group.
 */
typedef struct {
  /** The multicast group ID. */
  EmberMulticastId multicastId;
  /** The endpoint that is a member, or 0 if this entry is not in use
   *  (the ZDO is not a member of any multicast groups).
   */
  int8u endpoint;
  /** The network index of the network the entry is related to. */
  int8u networkIndex;
} EmberMulticastTableEntry;

/**
 * @brief Defines the events reported to the application
 * by the ::emberCounterHandler().
 */
#ifdef DOXYGEN_SHOULD_SKIP_THIS
enum EmberCounterType
#else
typedef int8u EmberCounterType;
enum
#endif
{
  /** The MAC received a broadcast. */
  EMBER_COUNTER_MAC_RX_BROADCAST = 0,
  /** The MAC transmitted a broadcast. */
  EMBER_COUNTER_MAC_TX_BROADCAST = 1,
  /** The MAC received a unicast. */
  EMBER_COUNTER_MAC_RX_UNICAST = 2,
  /** The MAC successfully transmitted a unicast. */
  EMBER_COUNTER_MAC_TX_UNICAST_SUCCESS = 3,
  /** The MAC retried a unicast. This is a placeholder
   * and is not used by the ::emberCounterHandler() callback.  Instead
   * the number of MAC retries are returned in the data parameter
   * of the callback for the @c ::EMBER_COUNTER_MAC_TX_UNICAST_SUCCESS
   * and @c ::EMBER_COUNTER_MAC_TX_UNICAST_FAILED types. */
  EMBER_COUNTER_MAC_TX_UNICAST_RETRY = 4,
  /** The MAC unsuccessfully transmitted a unicast. */
  EMBER_COUNTER_MAC_TX_UNICAST_FAILED = 5,

  /** The APS layer received a data broadcast. */
  EMBER_COUNTER_APS_DATA_RX_BROADCAST = 6,
  /** The APS layer transmitted a data broadcast. */
  EMBER_COUNTER_APS_DATA_TX_BROADCAST = 7,
  /** The APS layer received a data unicast. */
  EMBER_COUNTER_APS_DATA_RX_UNICAST = 8,
  /** The APS layer successfully transmitted a data unicast. */
  EMBER_COUNTER_APS_DATA_TX_UNICAST_SUCCESS = 9,
  /** The APS layer retried a data unicast.  This is a placeholder
   * and is not used by the @c ::emberCounterHandler() callback.  Instead
   * the number of APS retries are returned in the data parameter
   * of the callback for the @c ::EMBER_COUNTER_APS_DATA_TX_UNICAST_SUCCESS
   * and @c ::EMBER_COUNTER_APS_DATA_TX_UNICAST_FAILED types. */
  EMBER_COUNTER_APS_DATA_TX_UNICAST_RETRY = 10,  
  /** The APS layer unsuccessfully transmitted a data unicast. */
  EMBER_COUNTER_APS_DATA_TX_UNICAST_FAILED = 11,
  
  /** The network layer successfully submitted a new route discovery 
   * to the MAC. */
  EMBER_COUNTER_ROUTE_DISCOVERY_INITIATED = 12,

  /** An entry was added to the neighbor table. */
  EMBER_COUNTER_NEIGHBOR_ADDED = 13,
  /** An entry was removed from the neighbor table. */
  EMBER_COUNTER_NEIGHBOR_REMOVED = 14,
  /** A neighbor table entry became stale because it had not been heard from. */
  EMBER_COUNTER_NEIGHBOR_STALE = 15,
  
  /** A node joined or rejoined to the network via this node. */
  EMBER_COUNTER_JOIN_INDICATION = 16,
  /** An entry was removed from the child table. */
  EMBER_COUNTER_CHILD_REMOVED = 17,

  /** EZSP-UART only. An overflow error occurred in the UART. */
  EMBER_COUNTER_ASH_OVERFLOW_ERROR = 18,
  /** EZSP-UART only. A framing error occurred in the UART. */
  EMBER_COUNTER_ASH_FRAMING_ERROR = 19,
  /** EZSP-UART only. An overrun error occurred in the UART. */
  EMBER_COUNTER_ASH_OVERRUN_ERROR = 20,

  /** A message was dropped at the Network layer because the NWK frame
      counter was not higher than the last message seen from that source. */
  EMBER_COUNTER_NWK_FRAME_COUNTER_FAILURE = 21,

  /** A message was dropped at the APS layer because the APS frame counter
      was not higher than the last message seen from that source. */
  EMBER_COUNTER_APS_FRAME_COUNTER_FAILURE = 22,

  /** EZSP-UART only. An XOFF was transmitted by the UART. */
  EMBER_COUNTER_ASH_XOFF = 23,

  /** A message was dropped at the APS layer because it had APS encryption
      but the key associated with the sender has not been authenticated,
      and thus the key is not authorized for use in APS data messages. */
  EMBER_COUNTER_APS_LINK_KEY_NOT_AUTHORIZED = 24,

  /** A NWK encrypted message was received but dropped because decryption
      failed. */
  EMBER_COUNTER_NWK_DECRYPTION_FAILURE = 25,

  /** An APS encrypted message was received but dropped because decryption
      failed. */
  EMBER_COUNTER_APS_DECRYPTION_FAILURE = 26,
  
  /** The number of times we failed to allocate a set of linked packet
      buffers. This doesn't necessarily mean that the packet buffer count was
      0 at the time, but that the number requested was greater than the
      number free. */
  EMBER_COUNTER_ALLOCATE_PACKET_BUFFER_FAILURE = 27,

  /** The number of relayed unicast packets. */
  EMBER_COUNTER_RELAYED_UNICAST = 28,

  /** The number of times we dropped a packet due to reaching the preset
      PHY to MAC queue limit (emMaxPhyToMacQueueLength).  The limit will
      determine how many messages are accepted by the PHY between calls to
      emberTick(). After that limit is hit, packets will be dropped.  The
      number of dropped packets will be recorded in this counter.

      NOTE: For each call to emberCounterHandler() there may be more
      than 1 packet that was dropped due to the limit reached.  The
      actual number of packets dropped will be returned in the 'data'
      parameter passed to that function.
  */
  EMBER_COUNTER_PHY_TO_MAC_QUEUE_LIMIT_REACHED = 29,

  /** The number of times we dropped a packet due to the packet-validate
      library checking a packet and rejecting it due to length or
      other formatting problems.
  */
  EMBER_COUNTER_PACKET_VALIDATE_LIBRARY_DROPPED_COUNT = 30,

  /** The number of times the NWK retry queue is full and a new
      message failed to be added. 
  */
  EMBER_COUNTER_TYPE_NWK_RETRY_OVERFLOW = 31,

  /** The number of times the PHY layer was unable to transmit due to 
      a failed CCA
  */
  EMBER_COUNTER_PHY_CCA_FAIL_COUNT = 32,

  /** A placeholder giving the number of Ember counter types. */
  EMBER_COUNTER_TYPE_COUNT = 33
};

/**
 * @ brief Defines the CLI enumerations for the ::EmberCounterType enum. 
*/
#define EMBER_COUNTER_STRINGS                   \
    "Mac Rx Bcast",                             \
    "Mac Tx Bcast",                             \
    "Mac Rx Ucast",                             \
    "Mac Tx Ucast",                             \
    "Mac Tx Ucast Retry",                       \
    "Mac Tx Ucast Fail",                        \
    "APS Rx Bcast",                             \
    "APS Tx Bcast",                             \
    "APS Rx Ucast",                             \
    "APS Tx Ucast Success",                     \
    "APS Tx Ucast Retry",                       \
    "APS Tx Ucast Fail",                        \
    "Route Disc Initiated",                     \
    "Neighbor Added",                           \
    "Neighbor Removed",                         \
    "Neighbor Stale",                           \
    "Join Indication",                          \
    "Child Moved",                              \
    "ASH Overflow",                             \
    "ASH Frame Error",                          \
    "ASH Overrun Error",                        \
    "NWK FC Failure",                           \
    "APS FC Failure",                           \
    "ASH XOff",                                 \
    "APS Unauthorized Key",                     \
    "NWK Decrypt Failures",                     \
    "APS Decrypt Failures",                     \
    "Packet Buffer Allocate Failures",          \
    "Relayed Ucast",                            \
    "Phy to MAC queue limit reached",           \
    "Packet Validate drop count",               \
    "NWK retry overflow",                       \
    "CCA Failures",                             \
    NULL

/** brief An identifier for a task */
typedef int8u EmberTaskId;

#ifndef EZSP_HOST
  /** @brief Control structure for events.
   *
   * This structure should not be accessed directly.
   * This holds the event status (one of the @e EMBER_EVENT_ values)
   * and the time left before the event fires.
   */
  typedef struct {
    /** The event's status, either inactive or the units for timeToExecute. */
    EmberEventUnits status;
    /** The id of the task this event belongs to. */
    EmberTaskId taskid;
    /** How long before the event fires. 
     *  Units are always in milliseconds
     */
    int32u timeToExecute;
  } EmberEventControl;
#else
  // host applications use an older, basic form of the event system
  /** @brief Control structure for events.
   *
   * This structure should not be accessed directly.
   * This holds the event status (one of the @e EMBER_EVENT_ values)
   * and the time left before the event fires.
   */
  typedef struct {
    /** The event's status, either inactive or the units for timeToExecute. */
    EmberEventUnits status;
    /** How long before the event fires. 
     *  Units are determined by the event status. 
     */
    int16u timeToExecute;
  } EmberEventControl;
#endif

/** @brief Complete events with a control and a handler procedure.
 *
 * An application typically creates an array of events
 * along with their handlers.
 * The main loop passes the array to ::emberRunEvents() in order to call
 * the handlers of any events whose time has arrived.
 */
typedef PGM struct EmberEventData_S {
  /** The control structure for the event. */
  EmberEventControl *control;
  /** The procedure to call when the event fires. */
  void (*handler)(void);
} EmberEventData;

/** @brief Control structure for tasks.
 *
 * This structure should not be accessed directly.
 */
typedef struct {
  // The time when the next event associated with this task will fire
  int32u nextEventTime;
  // The list of events associated with this task
  EmberEventData *events;
  // A flag that indicates the task has something to do other than events
  boolean busy;
} EmberTaskControl;

/**
 * @name txPowerModes for emberSetTxPowerMode and mfglibSetPower 
 */
//@{

/** @brief The application should call ::emberSetTxPowerMode() with the
  * txPowerMode parameter set to this value to disable all power mode options,
  * resulting in normal power mode and bi-directional RF transmitter output.
  */
#define EMBER_TX_POWER_MODE_DEFAULT             0x0000
/** @brief The application should call ::emberSetTxPowerMode() with the
  * txPowerMode parameter set to this value to enable boost power mode.
  */
#define EMBER_TX_POWER_MODE_BOOST               0x0001
/** @brief The application should call ::emberSetTxPowerMode() with the
  * txPowerMode parameter set to this value to enable the alternate transmitter
  * output.
  */
#define EMBER_TX_POWER_MODE_ALTERNATE           0x0002
/** @brief The application should call ::emberSetTxPowerMode() with the
  * txPowerMode parameter set to this value to enable both boost mode and the
  * alternate transmitter output.
  */
#define EMBER_TX_POWER_MODE_BOOST_AND_ALTERNATE (EMBER_TX_POWER_MODE_BOOST     \
                                                |EMBER_TX_POWER_MODE_ALTERNATE)
#ifndef DOXYGEN_SHOULD_SKIP_THIS
// The application does not ever need to call emberSetTxPowerMode() with the
// txPowerMode parameter set to this value.  This value is used internally by
// the stack to indicate that the default token configuration has not been
// overridden by a prior call to emberSetTxPowerMode().
#define EMBER_TX_POWER_MODE_USE_TOKEN           0x8000
#endif//DOXYGEN_SHOULD_SKIP_THIS

//@} \\END  Definitions

/**
 * @name Alarm Message and Counters Request Definitions
 */
//@{

/** @brief This is a ZigBee application profile ID that has been
 * assigned to Ember Corporation.  
 *
 * It is used to send for sending messages
 * that have a specific, non-standard interaction with the Ember stack.
 * Its only current use is for alarm messages and stack counters requests.
 */
#define EMBER_PRIVATE_PROFILE_ID  0xC00E

/**
 * @brief Ember's first private profile ID.
 */
#define EMBER_PRIVATE_PROFILE_ID_START 0xC00D

/**
 * @brief Ember's last private profile ID.
 */
#define EMBER_PRIVATE_PROFILE_ID_END   0xC016

/** @brief Alarm messages provide a reliable means for communicating
 * with sleeping end devices.  
 *
 * A messages sent to a sleeping device is
 * normally buffered on the device's parent for a short time (the precise time
 * can be specified using the configuration parameter
 * ::EMBER_INDIRECT_TRANSMISSION_TIMEOUT).  If the child does not poll
 * its parent within that time the message is discarded.
 * 
 * In contrast, alarm messages are buffered by the parent indefinitely.
 * Because of the limited RAM available, alarm messages are necessarily
 * brief.  In particular, the parent only stores alarm payloads.  The
 * header information in alarm messages is not stored on the parent.
 * 
 * The memory used for buffering alarm messages is allocated statically.
 * The amount of memory set aside for alarms is controlled by two
 * configuration parameters:
 * - ::EMBER_BROADCAST_ALARM_DATA_SIZE
 * - ::EMBER_UNICAST_ALARM_DATA_SIZE
 *
 * Alarm messages must use the ::EMBER_PRIVATE_PROFILE_ID as the
 * application profile ID.  The source and destination endpoints
 * are ignored.
 *
 * Broadcast alarms must use ::EMBER_BROADCAST_ALARM_CLUSTER as the
 * cluster id and messages with this cluster ID must be sent to
 * ::EMBER_RX_ON_WHEN_IDLE_BROADCAST_ADDRESS.  A broadcast alarm may
 * not contain more than ::EMBER_BROADCAST_ALARM_DATA_SIZE bytes of
 * payload.
 *
 * Broadcast alarm messages arriving at a node are passed to the application
 * via ::emberIncomingMessageHandler().  If the receiving node has sleepy
 * end device children, the payload of the alarm is saved and then
 * forwarded to those children when they poll for data.  When a sleepy
 * child polls its parent, it receives only the most recently arrived
 * broadcast alarm.  If the child has already received the most recent
 * broadcast alarm it is not forwarded again.
 */
#define EMBER_BROADCAST_ALARM_CLUSTER       0x0000

/** @brief Unicast alarms must use ::EMBER_UNICAST_ALARM_CLUSTER as the
 * cluster id and messages with this cluster ID must be unicast.
 *
 * The payload of a unicast alarm consists of three one-byte length
 * fields followed by three variable length fields.
 * <ol>
 * <li> flags length
 * <li> priority length (must be 0 or 1)
 * <li> data length
 * <li> flags
 * <li> priority
 * <li> payload
 * </ol>
 *
 * The three lengths must total ::EMBER_UNICAST_ALARM_DATA_SIZE or less.
 *
 * When a unicast alarm message arrives at its destination it is passed
 * to the application via ::emberIncomingMessageHandler().
 * When a node receives a unicast alarm message whose destination is
 * a sleepy end device child of that node, the payload of the message
 * is saved until the child polls for data.  To conserve memory, the
 * values of the length fields are not saved.  The alarm will be forwarded
 * to the child using the ::EMBER_CACHED_UNICAST_ALARM_CLUSTER cluster ID.
 * 
 * If a unicast alarm arrives when a previous one is still pending, the
 * two payloads are combined.  This combining is controlled by the length
 * fields in the arriving message.  The incoming flag bytes are or'ed with
 * those of the pending message.  If the priority field is not present, or
 * if it is present and the incoming priority value is equal or greater
 * than the pending priority value, the pending data is replaced by the
 * incoming data.
 *
 * Because the length fields are not saved, the application designer must
 * fix on a set of field lengths that will be used for all unicast alarm
 * message sent to a particular device.
 */ 
#define EMBER_UNICAST_ALARM_CLUSTER         0x0001

/** @brief A unicast alarm that has been cached on the parent of
 * a sleepy end device is delivered to that device using the
 * ::EMBER_CACHED_UNICAST_ALARM_CLUSTER cluster ID.
 * The payload consists of three variable length fields.
 * <ol>
 * <li> flags
 * <li> priority
 * <li> payload
 * </ol>
 * The parent will pad the payload out to ::EMBER_UNICAST_ALARM_DATA_SIZE
 * bytes.
 *
 * The lengths of the these fields must be fixed by the application designer
 * and must be the same for all unicast alarms sent to a particular device.
 */ 
#define EMBER_CACHED_UNICAST_ALARM_CLUSTER  0x0002

/** The cluster id used to request that a node respond with a report of its
 * Ember stack counters.  See app/util/counters/counters-ota.h.
 */
#define EMBER_REPORT_COUNTERS_REQUEST 0x0003

/** The cluster id used to respond to an EMBER_REPORT_COUNTERS_REQUEST. */
#define EMBER_REPORT_COUNTERS_RESPONSE 0x8003

/** The cluster id used to request that a node respond with a report of its
 * Ember stack counters.  The node will also reset its clusters to zero after
 * a successful response.  See app/util/counters/counters-ota.h.
 */
#define EMBER_REPORT_AND_CLEAR_COUNTERS_REQUEST 0x0004

/** The cluster id used to respond to an EMBER_REPORT_AND_CLEAR_COUNTERS_REQUEST. */
#define EMBER_REPORT_AND_CLEAR_COUNTERS_RESPONSE 0x8004

/** The cluster id used to send and receive Over-the-air certificate messages.
 *  This is used to field upgrade devices with Smart Energy Certificates and
 *  other security data.
 */
#define EMBER_OTA_CERTIFICATE_UPGRADE_CLUSTER 0x0005

//@} \\END Alarm Message and Counters Request Definitions


/** @brief This data structure contains the key data that is passed
 *   into various other functions. */
typedef struct { 
  /** This is the key byte data. */
  int8u contents[EMBER_ENCRYPTION_KEY_SIZE];
} EmberKeyData;

/** @brief This data structure contains the certificate data that is used
    for Certificate Based Key Exchange (CBKE). */
typedef struct {
  int8u contents[EMBER_CERTIFICATE_SIZE];
} EmberCertificateData;

/** @brief This data structure contains the public key data that is used
    for Certificate Based Key Exchange (CBKE). */
typedef struct {
  int8u contents[EMBER_PUBLIC_KEY_SIZE];
} EmberPublicKeyData;

/** @brief This data structure contains the private key data that is used
    for Certificate Based Key Exchange (CBKE). */
typedef struct {
  int8u contents[EMBER_PRIVATE_KEY_SIZE];
} EmberPrivateKeyData;

/** @brief This data structure contains the Shared Message Authentication Code
    (SMAC) data that is used for Certificate Based Key Exchange (CBKE). */
typedef struct {
  int8u contents[EMBER_SMAC_SIZE];
} EmberSmacData;

/** @brief This data structure contains a DSA signature.  It is the bit
      concatenation of the 'r' and 's' components of the signature.
*/
typedef struct {
  int8u contents[EMBER_SIGNATURE_SIZE];
} EmberSignatureData;

/** @brief This data structure contains an AES-MMO Hash (the message digest).
 */
typedef struct {
  int8u contents[EMBER_AES_HASH_BLOCK_SIZE];
} EmberMessageDigest;

/** @brief This data structure contains the context data when calculating
 *  an AES MMO hash (message digest).
 */
typedef struct {
  int8u result[EMBER_AES_HASH_BLOCK_SIZE];
  int32u length;
} EmberAesMmoHashContext;

/** @brief This data structure contains the certificate data that is used
    for Certificate Based Key Exchange (CBKE) in SECT283k1 Elliptical Cryptography. */
typedef struct {
  /* This is the certificate byte data. */
  int8u contents[EMBER_CERTIFICATE_283K1_SIZE];
} EmberCertificate283k1Data;

/** @brief This data structure contains the public key data that is used
    for Certificate Based Key Exchange (CBKE) in SECT283k1 Elliptical Cryptography. */
typedef struct {
  int8u contents[EMBER_PUBLIC_KEY_283K1_SIZE];
} EmberPublicKey283k1Data;

/** @brief This data structure contains the private key data that is used
    for Certificate Based Key Exchange (CBKE) in SECT283k1 Elliptical Cryptography. */
typedef struct {
  int8u contents[EMBER_PRIVATE_KEY_283K1_SIZE];
} EmberPrivateKey283k1Data;

/** @brief This data structure contains a DSA signature used in SECT283k1 
    Elliptical Cryptography.
    It is the bit concatenation of the 'r' and 's' components of the signature.
*/
typedef struct {
  int8u contents[EMBER_SIGNATURE_283K1_SIZE];
} EmberSignature283k1Data;

/** @brief This is an ::EmberInitialSecurityBitmask value but it does not
 *    actually set anything.  It is the default mode used by the ZigBee Pro 
 *    stack.  It is defined here so that no legacy code is broken by referencing
 *    it.
 */
#define EMBER_STANDARD_SECURITY_MODE 0x0000

/** @brief This is the short address of the trust center.  It never changes 
 *    from this value throughout the life of the network.
 */
#define EMBER_TRUST_CENTER_NODE_ID 0x0000


/** @brief This is the Initial Security Bitmask that controls the use of various
 *  security features.
 */
#ifdef DOXYGEN_SHOULD_SKIP_THIS
enum EmberInitialSecurityBitmask
#else
typedef int16u EmberInitialSecurityBitmask;
enum
#endif
{
  /** This enables Distributed Trust Center Mode for the device forming the
      network. (Previously known as ::EMBER_NO_TRUST_CENTER_MODE) */
  EMBER_DISTRIBUTED_TRUST_CENTER_MODE       = 0x0002,
  /** This enables a Global Link Key for the Trust Center. All nodes will share
      the same Trust Center Link Key. */
  EMBER_TRUST_CENTER_GLOBAL_LINK_KEY        = 0x0004,
  /** This enables devices that perform MAC Association with a pre-configured 
      Network Key to join the network.  It is only set on the Trust Center. */
  EMBER_PRECONFIGURED_NETWORK_KEY_MODE      = 0x0008,

#if !defined DOXYGEN_SHOULD_SKIP_THIS
  // Hidden fields used internally.
  EMBER_HAVE_TRUST_CENTER_UNKNOWN_KEY_TOKEN = 0x0010,
  EMBER_HAVE_TRUST_CENTER_LINK_KEY_TOKEN    = 0x0020,
  EMBER_HAVE_TRUST_CENTER_MASTER_KEY_TOKEN  = 0x0030,
#endif
  /** This denotes that the ::EmberInitialSecurityState::preconfiguredTrustCenterEui64
      has a value in it containing the trust center EUI64.  The device will only
      join a network and accept commands from a trust center with that EUI64.  
      Normally this bit is NOT set, and the EUI64 of the trust center is learned
      during the join process.  When commissioning a device to join onto
      an existing network that is using a trust center, and without sending any
      messages, this bit must be set and the field
      ::EmberInitialSecurityState::preconfiguredTrustCenterEui64 must be
      populated with the appropriate EUI64.
  */
  EMBER_HAVE_TRUST_CENTER_EUI64             = 0x0040,

  /** This denotes that the ::EmberInitialSecurityState::preconfiguredKey
      is not the actual Link Key but a Root Key known only to the Trust Center.
      It is hashed with the IEEE Address of the destination device in order
      to create the actual Link Key used in encryption.  This is bit is only
      used by the Trust Center.  The joining device need not set this.
  */
  EMBER_TRUST_CENTER_USES_HASHED_LINK_KEY   = 0x0084,

  /** This denotes that the ::EmberInitialSecurityState::preconfiguredKey
   *  element has valid data that should be used to configure the initial
   *  security state. */
  EMBER_HAVE_PRECONFIGURED_KEY              = 0x0100,
  /** This denotes that the ::EmberInitialSecurityState::networkKey
   *  element has valid data that should be used to configure the initial
   *  security state. */
  EMBER_HAVE_NETWORK_KEY                    = 0x0200,
  /** This denotes to a joining node that it should attempt to 
   *  acquire a Trust Center Link Key during joining. This is
   *  only necessary if the device does not have a pre-configured
   *  key. */
  EMBER_GET_LINK_KEY_WHEN_JOINING           = 0x0400,
  /** This denotes that a joining device should only accept an encrypted
   *  network key from the Trust Center (using its pre-configured key).
   *  A key sent in-the-clear by the Trust Center will be rejected
   *  and the join will fail.  This option is only valid when utilizing
   *  a pre-configured key. */
  EMBER_REQUIRE_ENCRYPTED_KEY               = 0x0800,
  /** This denotes whether the device should NOT reset its outgoing frame
   *  counters (both NWK and APS) when ::emberSetInitialSecurityState() is
   *  called.  Normally it is advised to reset the frame counter before
   *  joining a new network.  However in cases where a device is joining
   *  to the same network again (but not using ::emberRejoinNetwork())
   *  it should keep the NWK and APS frame counters stored in its tokens.
   *
   *  NOTE: The application is allowed to dynamically change the behavior
   *  via EMBER_EXT_NO_FRAME_COUNTER_RESET field.
   */
  EMBER_NO_FRAME_COUNTER_RESET              = 0x1000,
  /** This denotes that the device should obtain its preconfigured key from
   *  an installation code stored in the manufacturing token.  The token
   *  contains a value that will be hashed to obtain the actual
   *  preconfigured key.  If that token is not valid than the call
   *  to ::emberSetInitialSecurityState() will fail. */
  EMBER_GET_PRECONFIGURED_KEY_FROM_INSTALL_CODE = 0x2000,

#if !defined DOXYGEN_SHOULD_SKIP_THIS
  // Internal data
  EM_SAVED_IN_TOKEN                         = 0x4000,
  #define EM_SECURITY_INITIALIZED           0x00008000L

  // This is only used internally.  High security is not released or supported
  // except for golden unit compliance.
  #define EMBER_HIGH_SECURITY_MODE          0x0001
#else 
  /* All other bits are reserved and must be zero. */
#endif
};

/** @brief This is the Extended Security Bitmask that controls the use
 *  of various extended security features.
 */
#ifdef DOXYGEN_SHOULD_SKIP_THIS
enum EmberExtendedSecurityBitmask
#else
typedef int16u EmberExtendedSecurityBitmask;
enum
#endif
{
#ifndef DOXYGEN_SHOULD_SKIP_THIS
  // If this bit is set, we set the 'key token data' field in the Initial
  // Security Bitmask to 0 (No Preconfig Key token), otherwise we leave the
  // field as it is.
  EMBER_PRECONFIG_KEY_NOT_VALID       = 0x0001,
#endif

  // bits 1-3 are unused.

  /** This denotes whether a joiner node (router or end-device) uses a Global
      Link Key or a Unique Link Key. */
  EMBER_JOINER_GLOBAL_LINK_KEY        = 0x0010,

  /** This denotes whether the device's outgoing frame counter is allowed to
      be reset during forming or joining. If flag is set, the outgoing frame
      counter is not allowed to be reset. If flag is not set, the frame
      counter is allowed to be reset. */

  EMBER_EXT_NO_FRAME_COUNTER_RESET    = 0x0020,

  // bit 6-7 reserved for future use (stored in TOKEN).

  /** This denotes whether a router node should discard or accept network Leave
      Commands. */
  EMBER_NWK_LEAVE_REQUEST_NOT_ALLOWED = 0x0100,

#ifndef DOXYGEN_SHOULD_SKIP_THIS
  /** This denotes whether a node is running the latest stack specification or
      is emulating the R18 specs behavior. If this flag is enabled, a router
      node should only send encrypted Update Device messages while the TC should
      only accept encrypted Updated Device messages.*/
  EMBER_R18_STACK_BEHAVIOR            = 0x0200,
#endif

  // bits 10-11 are reserved for future use (stored in RAM only).

  // bits 12-15 are unused.
};

/** @brief This is the legacy name for the Distributed Trust Center Mode.
 */
#define EMBER_NO_TRUST_CENTER_MODE   EMBER_DISTRIBUTED_TRUST_CENTER_MODE

/** @brief This is the legacy name for the Trust Center Global Link Key.
 */
#define EMBER_GLOBAL_LINK_KEY   EMBER_TRUST_CENTER_GLOBAL_LINK_KEY


#if !defined DOXYGEN_SHOULD_SKIP_THIS
  #define NO_TRUST_CENTER_KEY_TOKEN        0x0000
  #define TRUST_CENTER_KEY_TOKEN_MASK      0x0030
  #define SECURITY_BIT_TOKEN_MASK          0x71FF

  #define SECURITY_LOWER_BIT_MASK          0x000000FF  // ""
  #define SECURITY_UPPER_BIT_MASK          0x00FF0000L // ""
#endif

/** @brief This describes the Initial Security features and requirements that
 *  will be used when forming or joining the network.  */
typedef struct {
  /** This bitmask enumerates which security features should be used, as well
      as the presence of valid data within other elements of the
      ::EmberInitialSecurityState data structure.  For more details see the
      ::EmberInitialSecurityBitmask. */
  int16u bitmask;
  /** This is the pre-configured key that can used by devices when joining the 
   *  network if the Trust Center does not send the initial security data 
   *  in-the-clear.  
   *  For the Trust Center, it will be the global link key and <b>must</b> be set 
   *  regardless of whether joining devices are expected to have a pre-configured
   *  Link Key. 
   *  This parameter will only be used if the EmberInitialSecurityState::bitmask 
   *  sets the bit indicating ::EMBER_HAVE_PRECONFIGURED_KEY*/
  EmberKeyData preconfiguredKey;
  /** This is the Network Key used when initially forming the network.
   *  This must be set on the Trust Center.  It is not needed for devices
   *  joining the network.  This parameter will only be used if the 
   *  EmberInitialSecurityState::bitmask sets the bit indicating
   *  ::EMBER_HAVE_NETWORK_KEY.  */
  EmberKeyData networkKey;
  /** This is the sequence number associated with the network key.  It must
   *  be set if the Network Key is set.  It is used to indicate a particular
   *  of the network key for updating and switching.  This parameter will 
   *  only be used if the ::EMBER_HAVE_NETWORK_KEY is set. Generally it should
   *  be set to 0 when forming the network; joining devices can ignore 
   *  this value.  */
  int8u networkKeySequenceNumber;
  /** This is the long address of the trust center on the network that will
   *  be joined.  It is usually NOT set prior to joining the network and 
   *  instead it is learned during the joining message exchange.  This field
   *  is only examined if ::EMBER_HAVE_TRUST_CENTER_EUI64 is set in the 
   *  EmberInitialSecurityState::bitmask.  Most devices should clear that
   *  bit and leave this field alone.  This field must be set when using
   *  commissioning mode.  It is required to be in little-endian format. */
  EmberEUI64 preconfiguredTrustCenterEui64;
} EmberInitialSecurityState;


/** @brief This is the Current Security Bitmask that details the use of various
 *  security features.
 */
#ifdef DOXYGEN_SHOULD_SKIP_THIS
enum EmberCurrentSecurityBitmask
#else
typedef int16u EmberCurrentSecurityBitmask;
enum
#endif
{
#if defined DOXYGEN_SHOULD_SKIP_THIS
  // These options are the same for Initial and Current Security state

  /** This denotes that the device is running in a network with ZigBee 
   *  Standard Security. */
  EMBER_STANDARD_SECURITY_MODE_             = 0x0000,
  /** This denotes that the device is running in a network without 
   *  a centralized Trust Center. */
  EMBER_DISTRIBUTED_TRUST_CENTER_MODE_      = 0x0002,
  /** This denotes that the device has a Global Link Key.  The Trust Center
   *  Link Key is the same across multiple nodes. */  
  EMBER_TRUST_CENTER_GLOBAL_LINK_KEY_       = 0x0004,
#else
  // Bit 3 reserved
#endif
  /** This denotes that the node has a Trust Center Link Key. */
  EMBER_HAVE_TRUST_CENTER_LINK_KEY          = 0x0010,

  /** This denotes that the Trust Center is using a Hashed Link Key. */
  EMBER_TRUST_CENTER_USES_HASHED_LINK_KEY_   = 0x0084,

  // Bits 1,5,6, 8-15 reserved
};

#if !defined DOXYGEN_SHOULD_SKIP_THIS
  #define INITIAL_AND_CURRENT_BITMASK         0x00FF
#endif


/** @brief This describes the security features used by the stack for a
 *  joined device. */
typedef struct {
  /** This bitmask indicates the security features currently in use
   *  on this node. */
  EmberCurrentSecurityBitmask bitmask;
  /** This indicates the EUI64 of the Trust Center.  It will be all zeroes 
      if the Trust Center Address is not known (i.e. the device is in a 
      Distributed Trust Center network). */
  EmberEUI64 trustCenterLongAddress;
} EmberCurrentSecurityState;


/** @brief This bitmask describes the presence of fields within the 
 *  ::EmberKeyStruct. */

#ifdef DOXYGEN_SHOULD_SKIP_THIS
enum EmberKeyStructBitmask
#else
typedef int16u EmberKeyStructBitmask;
enum
#endif
{
  /** This indicates that the key has a sequence number associated with it.
      (i.e. a Network Key). */
  EMBER_KEY_HAS_SEQUENCE_NUMBER        = 0x0001,
  /** This indicates that the key has an outgoing frame counter and the 
      corresponding value within the ::EmberKeyStruct has been populated with 
      the data. */
  EMBER_KEY_HAS_OUTGOING_FRAME_COUNTER = 0x0002,
  /** This indicates that the key has an incoming frame counter and the 
      corresponding value within the ::EmberKeyStruct has been populated with 
      the data. */
  EMBER_KEY_HAS_INCOMING_FRAME_COUNTER = 0x0004,
  /** This indicates that the key has an associated Partner EUI64 address and  
      the corresponding value within the ::EmberKeyStruct has been populated  
      with the data. */
  EMBER_KEY_HAS_PARTNER_EUI64          = 0x0008,
  /** This indicates the key is authorized for use in APS data messages.
      If the key is not authorized for use in APS data messages it has not
      yet gone through a key agreement protocol, such as CBKE (i.e. ECC) */
  EMBER_KEY_IS_AUTHORIZED              = 0x0010,
  /** This indicates that the partner associated with the link is a sleepy
      end device.  This bit is set automatically if the local device
      hears a device announce from the partner indicating it is not
      an 'RX on when idle' device.  */
  EMBER_KEY_PARTNER_IS_SLEEPY          = 0x0020,
      
};

/** @brief This denotes the type of security key. */
#ifdef DOXYGEN_SHOULD_SKIP_THIS
enum EmberKeyType
#else
typedef int8u EmberKeyType;
enum
#endif
{
  /** This denotes that the key is a Trust Center Link Key. */
  EMBER_TRUST_CENTER_LINK_KEY         = 1,
  /** This denotes that the key is a Trust Center Master Key. */
  EMBER_TRUST_CENTER_MASTER_KEY       = 2,
  /** This denotes that the key is the Current Network Key. */
  EMBER_CURRENT_NETWORK_KEY           = 3,
  /** This denotes that the key is the Next Network Key. */
  EMBER_NEXT_NETWORK_KEY              = 4,
  /** This denotes that the key is an Application Link Key */
  EMBER_APPLICATION_LINK_KEY          = 5,
  /** This denotes that the key is an Application Master Key */
  EMBER_APPLICATION_MASTER_KEY        = 6,
};

/** @brief This describes a one of several different types of keys and its
 *   associated data.
 */
typedef struct {
  /** This bitmask indicates whether various fields in the structure
   *  contain valid data. It also contains the index of the network
   *  the key belongs to. */
  EmberKeyStructBitmask bitmask;
  /** This indicates the type of the security key. */
  EmberKeyType type;
  /** This is the actual key data. */
  EmberKeyData key;
  /** This is the outgoing frame counter associated with the key.  It will 
   *  contain valid data based on the ::EmberKeyStructBitmask. */
  int32u outgoingFrameCounter;
  /** This is the incoming frame counter associated with the key.  It will 
   *  contain valid data based on the ::EmberKeyStructBitmask. */
  int32u incomingFrameCounter;
  /** This is the sequence number associated with the key.  It will 
   *  contain valid data based on the ::EmberKeyStructBitmask. */
  int8u sequenceNumber;
  /** This is the Partner EUI64 associated with the key.  It will 
   *  contain valid data based on the ::EmberKeyStructBitmask. */
  EmberEUI64 partnerEUI64;
} EmberKeyStruct;


/** @brief This denotes the status of an attempt to establish
 *  a key with another device.
 */
#ifdef DOXYGEN_SHOULD_SKIP_THIS
enum EmberKeyStatus
#else 
typedef int8u EmberKeyStatus;
enum
#endif
{
  EMBER_APP_LINK_KEY_ESTABLISHED           = 1,
  EMBER_APP_MASTER_KEY_ESTABLISHED         = 2,
  EMBER_TRUST_CENTER_LINK_KEY_ESTABLISHED  = 3,
  EMBER_KEY_ESTABLISHMENT_TIMEOUT          = 4,
  EMBER_KEY_TABLE_FULL                     = 5,

  // These are success status values applying only to the
  // Trust Center answering key requests
  EMBER_TC_RESPONDED_TO_KEY_REQUEST        = 6,
  EMBER_TC_APP_KEY_SENT_TO_REQUESTER       = 7,

  // These are failure status values applying only to the
  // Trust Center answering key requests
  EMBER_TC_RESPONSE_TO_KEY_REQUEST_FAILED  = 8,
  EMBER_TC_REQUEST_KEY_TYPE_NOT_SUPPORTED  = 9,
  EMBER_TC_NO_LINK_KEY_FOR_REQUESTER       = 10,
  EMBER_TC_REQUESTER_EUI64_UNKNOWN         = 11,
  EMBER_TC_RECEIVED_FIRST_APP_KEY_REQUEST  = 12,
  EMBER_TC_TIMEOUT_WAITING_FOR_SECOND_APP_KEY_REQUEST = 13,
  EMBER_TC_NON_MATCHING_APP_KEY_REQUEST_RECEIVED      = 14,
  EMBER_TC_FAILED_TO_SEND_APP_KEYS         = 15,
  EMBER_TC_FAILED_TO_STORE_APP_KEY_REQUEST = 16,
  EMBER_TC_REJECTED_APP_KEY_REQUEST        = 17,
};

/** @brief This enumeration determines whether or not a Trust Center
 *  answers link key requests.
 */
#ifdef DOXYGEN_SHOULD_SKIP_THIS
enum EmberLinkKeyRequestPolicy
#else 
typedef int8u EmberLinkKeyRequestPolicy;
enum
#endif
{
  EMBER_DENY_KEY_REQUESTS  = 0x00,  
  EMBER_ALLOW_KEY_REQUESTS = 0x01,
};


/** @brief This function allows the programmer to gain access
 *  to the actual key data bytes of the EmberKeyData struct.
 *
 * @param key  A Pointer to an EmberKeyData structure.
 *
 * @return int8u* Returns a pointer to the first byte of the Key data.
 */
#if defined DOXYGEN_SHOULD_SKIP_THIS
int8u* emberKeyContents(EmberKeyData* key);
#else
#define emberKeyContents(key) ((key)->contents)
#endif

/** @brief This function allows the programmer to gain access
 *  to the actual certificate data bytes of the EmberCertificateData struct.
 *
 * @param cert  A Pointer to an ::EmberCertificateData structure.
 *
 * @return int8u* Returns a pointer to the first byte of the certificate data.
 */
#if defined DOXYGEN_SHOULD_SKIP_THIS
int8u* emberCertificateContents(EmberCertificateData* cert);
#else
#define emberCertificateContents(cert) ((cert)->contents)
#endif

/** @brief This function allows the programmer to gain access
 *  to the actual public key data bytes of the EmberPublicKeyData struct.
 *
 * @param key  A Pointer to an EmberPublicKeyData structure.
 *
 * @return int8u* Returns a pointer to the first byte of the public key data.
 */
#if defined DOXYGEN_SHOULD_SKIP_THIS
int8u* emberPublicKeyContents(EmberPublicKeyData* key);
#else
#define emberPublicKeyContents(key) ((key)->contents)
#endif

/** @brief This function allows the programmer to gain access
 *  to the actual private key data bytes of the EmberPrivateKeyData struct.
 *
 * @param key  A Pointer to an EmberPrivateKeyData structure.
 *
 * @return int8u* Returns a pointer to the first byte of the private key data.
 */
#if defined DOXYGEN_SHOULD_SKIP_THIS
int8u* emberPrivateKeyContents(EmberPrivateKeyData* key);
#else
#define emberPrivateKeyContents(key) ((key)->contents)
#endif

/** @brief This function allows the programmer to gain access to the 
 *  actual SMAC (Secured Message Authentication Code) data of the 
 *  EmberSmacData struct.
 */
#if defined DOXYGEN_SHOULD_SKIP_THIS
int8u* emberSmacContents(EmberSmacData* key);
#else
#define emberSmacContents(key) ((key)->contents)
#endif

/** @brief This function allows the programmer to gain access to the
 *  actual ECDSA signature data of the EmberSignatureData struct.
 */
#if defined DOXYGEN_SHOULD_SKIP_THIS
int8u* emberSignatureContents(EmberSignatureData* sig);
#else
#define emberSignatureContents(sig) ((sig)->contents)
#endif

/** @brief This function allows the programmer to gain access
 *  to the actual certificate data bytes of the Ember283k1CertificateData struct.
 *
 * @param cert  A Pointer to an ::Ember283k1CertificateData structure.
 *
 * @return int8u* Returns a pointer to the first byte of the certificate data.
 */
#if defined DOXYGEN_SHOULD_SKIP_THIS
int8u* emberCertificate283k1Contents(EmberCertificate283k1Data* cert);
#else
#define emberCertificate283k1Contents(cert) ((cert)->contents)
#endif

/** @brief This function allows the programmer to gain access
 *  to the actual public key data bytes of the Ember283k1PublicKeyData struct.
 *
 * @param key  A Pointer to an Ember283k1PublicKeyData structure.
 *
 * @return int8u* Returns a pointer to the first byte of the public key data.
 */
#if defined DOXYGEN_SHOULD_SKIP_THIS
int8u* emberPublicKey283k1Contents(EmberPublicKey283k1Data* key);
#else
#define emberPublicKey283k1Contents(key) ((key)->contents)
#endif

/** @brief This function allows the programmer to gain access
 *  to the actual private key data bytes of the Ember283k1PrivateKeyData struct.
 *
 * @param key  A Pointer to an Ember283k1PrivateKeyData structure.
 *
 * @return int8u* Returns a pointer to the first byte of the private key data.
 */
#if defined DOXYGEN_SHOULD_SKIP_THIS
int8u* emberPrivateKey283k1Contents(EmberPrivateKey283k1Data* key);
#else
#define emberPrivateKey283k1Contents(key) ((key)->contents)
#endif

/** @brief This function allows the programmer to gain access to the
 *  actual ECDSA signature data of the Ember283k1SignatureData struct.
 */
#if defined DOXYGEN_SHOULD_SKIP_THIS
int8u* ember283k1SignatureContents(Ember283k1SignatureData* sig);
#else
#define ember283k1SignatureContents(sig) ((sig)->contents)
#endif

#ifdef DOXYGEN_SHOULD_SKIP_THIS
enum EmberKeySettings
#else
typedef int16u EmberKeySettings;
enum
#endif
{
  EMBER_KEY_PERMISSIONS_NONE            = 0x0000,
  EMBER_KEY_PERMISSIONS_READING_ALLOWED = 0x0001,
  EMBER_KEY_PERMISSIONS_HASHING_ALLOWED = 0x0002,
};


/** @brief This structure is used to get/set the security config that
 *    is stored in manufacturing tokens.
 */
typedef struct {
  EmberKeySettings keySettings;
} EmberMfgSecurityStruct;


/** @brief This magic number prevents accidentally changing the key settings.
 *    The ::emberSetMfgSecurityConfig() API will return EMBER_INVALID_CALL 
 *    unless it is passed in.
 */
#define EMBER_MFG_SECURITY_CONFIG_MAGIC_NUMBER 0xCABAD11FUL


/**
 * @brief The types of MAC passthrough messages that an application may
 * receive.  This is a bitmask.
 */
#ifdef DOXYGEN_SHOULD_SKIP_THIS
enum EmberMacPassthroughType
#else
typedef int8u EmberMacPassthroughType;
enum
#endif
{
  /** No MAC passthrough messages */
  EMBER_MAC_PASSTHROUGH_NONE             = 0x00,
  /** SE InterPAN messages */
  EMBER_MAC_PASSTHROUGH_SE_INTERPAN      = 0x01,
  /** EmberNet and first generation (v1) standalone bootloader messages */
  EMBER_MAC_PASSTHROUGH_EMBERNET         = 0x02,
  /** EmberNet messages filtered by their source address. */
  EMBER_MAC_PASSTHROUGH_EMBERNET_SOURCE  = 0x04,
  /** Application-specific passthrough messages. */
  EMBER_MAC_PASSTHROUGH_APPLICATION      = 0x08,
  /** Custom inter-pan filter */
  EMBER_MAC_PASSTHROUGH_CUSTOM           = 0x10,

#if !defined DOXYGEN_SHOULD_SKIP_THIS
  /** Internal Stack pass-through */
  EM_MAC_PASSTHROUGH_INTERNAL            = 0x80
#endif
};

/** @brief This is a bitmask describing a filter for MAC 
 *    data messages that the stack should accept and passthrough
 *    to the application.
 */
typedef int16u EmberMacFilterMatchData;

#define EMBER_MAC_FILTER_MATCH_ENABLED_MASK            0x0001
#define EMBER_MAC_FILTER_MATCH_ON_PAN_DEST_MASK        0x0003
#define EMBER_MAC_FILTER_MATCH_ON_PAN_SOURCE_MASK      0x000C
#define EMBER_MAC_FILTER_MATCH_ON_DEST_MASK            0x0030
#define EMBER_MAC_FILTER_MATCH_ON_SOURCE_MASK          0x0080

// Globally turn on/off this filter
#define EMBER_MAC_FILTER_MATCH_ENABLED                 0x0000
#define EMBER_MAC_FILTER_MATCH_DISABLED                0x0001

// Pick either one of these
#define EMBER_MAC_FILTER_MATCH_ON_PAN_DEST_NONE        0x0000
#define EMBER_MAC_FILTER_MATCH_ON_PAN_DEST_LOCAL       0x0001
#define EMBER_MAC_FILTER_MATCH_ON_PAN_DEST_BROADCAST   0x0002

// and one of these
#define EMBER_MAC_FILTER_MATCH_ON_PAN_SOURCE_NONE      0x0000
#define EMBER_MAC_FILTER_MATCH_ON_PAN_SOURCE_NON_LOCAL 0x0004
#define EMBER_MAC_FILTER_MATCH_ON_PAN_SOURCE_LOCAL     0x0008

// and one of these
#define EMBER_MAC_FILTER_MATCH_ON_DEST_BROADCAST_SHORT 0x0000
#define EMBER_MAC_FILTER_MATCH_ON_DEST_UNICAST_SHORT   0x0010
#define EMBER_MAC_FILTER_MATCH_ON_DEST_UNICAST_LONG    0x0020

// and one of these
#define EMBER_MAC_FILTER_MATCH_ON_SOURCE_LONG          0x0000
#define EMBER_MAC_FILTER_MATCH_ON_SOURCE_SHORT         0x0080

// Last entry should set this and nothing else.  No other bits will be examined.
#define EMBER_MAC_FILTER_MATCH_END                     0x8000

/** @brief This structure indicates a matching raw MAC message has been
 *    received by the application configured MAC filters.
 */
typedef struct {
  int8u filterIndexMatch;
  EmberMacPassthroughType legacyPassthroughType;
  EmberMessageBuffer message;
} EmberMacFilterMatchStruct;


/** @brief This indicates the presence, absence, or status of an Ember
 *    stack library.
 */
typedef int8u EmberLibraryStatus;

/**
 * @name ZigBee Device Object (ZDO) Definitions
 */
//@{

/** @name ZDO response status. 
 *
 * Most responses to ZDO commands contain a status byte. 
 * The meaning of this byte is defined by the ZigBee Device Profile.
 */
#ifdef DOXYGEN_SHOULD_SKIP_THIS
enum EmberZdoStatus
#else
typedef int8u EmberZdoStatus;
enum
#endif
{
  // These values are taken from Table 48 of ZDP Errata 043238r003 and Table 2
  // of NWK 02130r10.
  EMBER_ZDP_SUCCESS              = 0x00,
  // 0x01 to 0x7F are reserved
  EMBER_ZDP_INVALID_REQUEST_TYPE = 0x80,
  EMBER_ZDP_DEVICE_NOT_FOUND     = 0x81,
  EMBER_ZDP_INVALID_ENDPOINT     = 0x82,
  EMBER_ZDP_NOT_ACTIVE           = 0x83,
  EMBER_ZDP_NOT_SUPPORTED        = 0x84,
  EMBER_ZDP_TIMEOUT              = 0x85,
  EMBER_ZDP_NO_MATCH             = 0x86,
  // 0x87 is reserved            = 0x87,
  EMBER_ZDP_NO_ENTRY             = 0x88,
  EMBER_ZDP_NO_DESCRIPTOR        = 0x89,
  EMBER_ZDP_INSUFFICIENT_SPACE   = 0x8a,
  EMBER_ZDP_NOT_PERMITTED        = 0x8b,
  EMBER_ZDP_TABLE_FULL           = 0x8c,
  EMBER_ZDP_NOT_AUTHORIZED       = 0x8d,

  EMBER_NWK_ALREADY_PRESENT      = 0xC5,
  EMBER_NWK_TABLE_FULL           = 0xC7,
  EMBER_NWK_UNKNOWN_DEVICE       = 0xC8
};

/** 
 * Defines for ZigBee device profile cluster IDs follow. These
 * include descriptions of the formats of the messages.
 *
 * Note that each message starts with a 1-byte transaction sequence
 * number. This sequence number is used to match a response command frame
 * to the request frame that it is replying to. The application shall 
 * maintain a 1-byte counter that is copied into this field and incremented 
 * by one for each command sent. When a value of 0xff is reached, the next 
 * command shall re-start the counter with a value of 0x00
 */

  /// @name Network and IEEE Address Request/Response 
  /// <br> @{ 
  ///  
  /// @code
  /// Network request: <transaction sequence number: 1>
  ///                  <EUI64:8>   <type:1> <start index:1> 
  /// IEEE request:    <transaction sequence number: 1>
  ///                  <node ID:2> <type:1> <start index:1> 
  ///                  <type> = 0x00 single address response, ignore the start index 
  ///                  = 0x01 extended response -> sends kid's IDs as well 
  /// Response: <transaction sequence number: 1>
  ///           <status:1> <EUI64:8> <node ID:2> 
  ///           <ID count:1> <start index:1> <child ID:2>*
  /// @endcode
#define NETWORK_ADDRESS_REQUEST      0x0000
#define NETWORK_ADDRESS_RESPONSE     0x8000
#define IEEE_ADDRESS_REQUEST         0x0001
#define IEEE_ADDRESS_RESPONSE        0x8001
  /// @}

  /// @name Node Descriptor Request/Response
  /// <br> @{
  ///  
  /// @code
  /// Request:  <transaction sequence number: 1> <node ID:2> 
  /// Response: <transaction sequence number: 1> <status:1> <node ID:2> 
  //            <node descriptor: 13>
  //
  //  Node Descriptor field is divided into subfields of bitmasks as follows:
  //      (Note: All lengths below are given in bits rather than bytes.)
  //            Logical Type:                     3
  //            Complex Descriptor Available:     1  
  //            User Descriptor Available:        1
  //            (reserved/unused):                3
  //            APS Flags:                        3
  //            Frequency Band:                   5
  //            MAC capability flags:             8
  //            Manufacturer Code:               16
  //            Maximum buffer size:              8
  //            Maximum incoming transfer size:  16
  //            Server mask:                     16
  //            Maximum outgoing transfer size:  16
  //            Descriptor Capability Flags:      8
  //     See ZigBee document 053474, Section 2.3.2.3 for more details.
  /// @endcode
#define NODE_DESCRIPTOR_REQUEST      0x0002
#define NODE_DESCRIPTOR_RESPONSE     0x8002
  /// @}

  /// @name Power Descriptor Request / Response
  /// <br> @{
  ///  
  /// @code
  /// Request:  <transaction sequence number: 1> <node ID:2> 
  /// Response: <transaction sequence number: 1> <status:1> <node ID:2> 
  ///           <current power mode, available power sources:1> 
  ///           <current power source, current power source level:1>
  //     See ZigBee document 053474, Section 2.3.2.4 for more details.
  /// @endcode
#define POWER_DESCRIPTOR_REQUEST     0x0003
#define POWER_DESCRIPTOR_RESPONSE    0x8003
  /// @}

  /// @name Simple Descriptor Request / Response
  /// <br> @{
  /// 
  /// @code
  /// Request:  <transaction sequence number: 1>
  ///           <node ID:2> <endpoint:1> 
  /// Response: <transaction sequence number: 1>
  ///           <status:1> <node ID:2> <length:1> <endpoint:1> 
  ///           <app profile ID:2> <app device ID:2> 
  ///           <app device version, app flags:1> 
  ///           <input cluster count:1> <input cluster:2>* 
  ///           <output cluster count:1> <output cluster:2>*
  /// @endcode
#define SIMPLE_DESCRIPTOR_REQUEST    0x0004
#define SIMPLE_DESCRIPTOR_RESPONSE   0x8004
  /// @}

  /// @name Active Endpoints Request / Response
  /// <br> @{
  ///  
  /// @code
  /// Request:  <transaction sequence number: 1> <node ID:2> 
  /// Response: <transaction sequence number: 1> 
  ///           <status:1> <node ID:2> <endpoint count:1> <endpoint:1>*
  /// @endcode   
#define ACTIVE_ENDPOINTS_REQUEST     0x0005
#define ACTIVE_ENDPOINTS_RESPONSE    0x8005
  /// @}

  /// @name Match Descriptors Request / Response
  /// <br> @{
  ///  
  /// @code
  /// Request:  <transaction sequence number: 1> 
  ///           <node ID:2> <app profile ID:2> 
  ///           <input cluster count:1> <input cluster:2>* 
  ///           <output cluster count:1> <output cluster:2>* 
  /// Response: <transaction sequence number: 1> 
  ///           <status:1> <node ID:2> <endpoint count:1> <endpoint:1>*
  /// @endcode   
#define MATCH_DESCRIPTORS_REQUEST    0x0006
#define MATCH_DESCRIPTORS_RESPONSE   0x8006
  /// @}

  /// @name Discovery Cache Request / Response
  /// <br> @{
  ///  
  /// @code
  /// Request:  <transaction sequence number: 1> 
  ///           <source node ID:2> <source EUI64:8>
  /// Response: <transaction sequence number: 1>
  ///           <status (== EMBER_ZDP_SUCCESS):1>
  /// @endcode 
#define DISCOVERY_CACHE_REQUEST      0x0012
#define DISCOVERY_CACHE_RESPONSE     0x8012
  /// @}

  /// @name End Device Announce and End Device Announce Response
  /// <br> @{
  /// 
  /// @code
  /// Request: <transaction sequence number: 1> 
  ///          <node ID:2> <EUI64:8> <capabilities:1> 
  /// No response is sent.
  /// @endcode   
#define END_DEVICE_ANNOUNCE          0x0013
#define END_DEVICE_ANNOUNCE_RESPONSE 0x8013
  /// @}

  /// @name System Server Discovery Request / Response
  /// <br> @{
  ///  This is broadcast and only servers which have matching services 
  /// respond.  The response contains the request services that the 
  /// recipient provides.
  /// 
  /// @code
  /// Request:  <transaction sequence number: 1> <server mask:2> 
  /// Response: <transaction sequence number: 1>
  ///           <status (== EMBER_ZDP_SUCCESS):1> <server mask:2>
  /// @endcode
#define SYSTEM_SERVER_DISCOVERY_REQUEST  0x0015
#define SYSTEM_SERVER_DISCOVERY_RESPONSE 0x8015
  /// @}

  /// @name ZDO server mask bits
  /// <br> @{
  ///  These are used in server discovery
  /// requests and responses.
#ifdef DOXYGEN_SHOULD_SKIP_THIS
enum EmberZdoServerMask
#else
typedef int16u EmberZdoServerMask;
enum
#endif
{
  EMBER_ZDP_PRIMARY_TRUST_CENTER          = 0x0001,
  EMBER_ZDP_SECONDARY_TRUST_CENTER        = 0x0002,
  EMBER_ZDP_PRIMARY_BINDING_TABLE_CACHE   = 0x0004,
  EMBER_ZDP_SECONDARY_BINDING_TABLE_CACHE = 0x0008,
  EMBER_ZDP_PRIMARY_DISCOVERY_CACHE       = 0x0010,
  EMBER_ZDP_SECONDARY_DISCOVERY_CACHE     = 0x0020,
  EMBER_ZDP_NETWORK_MANAGER               = 0x0040,
  // Bits 0x0080 to 0x8000 are reserved.
};

  /// @name Find Node Cache Request / Response 
  /// <br> @{
  /// This is broadcast and only discovery servers which have the information
  /// for the device of interest, or the device of interest itself, respond.
  /// The requesting device can then direct any service discovery requests
  /// to the responder.
  /// 
  /// @code
  /// Request:  <transaction sequence number: 1>
  ///           <device of interest ID:2> <d-of-i EUI64:8> 
  /// Response: <transaction sequence number: 1>
  ///           <responder ID:2> <device of interest ID:2> <d-of-i EUI64:8>
  /// @endcode
#define FIND_NODE_CACHE_REQUEST         0x001C
#define FIND_NODE_CACHE_RESPONSE        0x801C
  /// @}

  /// @name End Device Bind Request / Response
  /// <br> @{
  ///  
  /// @code
  /// Request:  <transaction sequence number: 1>
  ///           <node ID:2> <EUI64:8> <endpoint:1> <app profile ID:2> 
  ///           <input cluster count:1> <input cluster:2>* 
  ///           <output cluster count:1> <output cluster:2>* 
  /// Response: <transaction sequence number: 1> <status:1>
  /// @endcode   
#define END_DEVICE_BIND_REQUEST      0x0020
#define END_DEVICE_BIND_RESPONSE     0x8020
  /// @}

  /// @name Binding types and Request / Response
  /// <br> @{
  ///  Bind and unbind have the same formats.  There are two possible
  /// formats, depending on whether the destination is a group address
  /// or a device address.  Device addresses include an endpoint, groups
  /// don't.
  ///
  /// @code
  /// Request:  <transaction sequence number: 1>
  ///           <source EUI64:8> <source endpoint:1> 
  ///           <cluster ID:2> <destination address:3 or 10> 
  /// Destination address:
  ///           <0x01:1> <destination group:2> 
  /// Or:
  ///           <0x03:1> <destination EUI64:8> <destination endpoint:1> 
  /// Response: <transaction sequence number: 1> <status:1>
  /// @endcode   
#define UNICAST_BINDING             0x03
#define UNICAST_MANY_TO_ONE_BINDING 0x83
#define MULTICAST_BINDING           0x01

#define BIND_REQUEST                 0x0021
#define BIND_RESPONSE                0x8021
#define UNBIND_REQUEST               0x0022
#define UNBIND_RESPONSE              0x8022
  /// @}

  /// @name LQI Table Request / Response
  /// <br> @{
  ///  
  ///
  /// @code
  /// Request:  <transaction sequence number: 1> <start index:1>
  /// Response: <transaction sequence number: 1> <status:1> 
  ///           <neighbor table entries:1> <start index:1>
  ///           <entry count:1> <entry:22>*
  ///   <entry> = <extended PAN ID:8> <EUI64:8> <node ID:2>
  ///             <device type, rx on when idle, relationship:1>
  ///             <permit joining:1> <depth:1> <LQI:1> 
  /// @endcode
  /// 
  /// The device-type byte has the following fields:
  ///  
  /// @code
  ///      Name          Mask        Values
  /// 
  ///   device type      0x03     0x00 coordinator
  ///                             0x01 router
  ///                             0x02 end device
  ///                             0x03 unknown
  ///
  ///   rx mode          0x0C     0x00 off when idle
  ///                             0x04 on when idle
  ///                             0x08 unknown
  ///
  ///   relationship     0x70     0x00 parent
  ///                             0x10 child
  ///                             0x20 sibling
  ///                             0x30 other
  ///                             0x40 previous child
  ///   reserved         0x10
  /// @endcode
  /// 
  /// The permit-joining byte has the following fields
  ///
  /// @code
  ///      Name          Mask        Values
  /// 
  ///   permit joining   0x03     0x00 not accepting join requests
  ///                             0x01 accepting join requests
  ///                             0x02 unknown
  ///   reserved         0xFC
  /// @endcode
  ///
#define LQI_TABLE_REQUEST            0x0031
#define LQI_TABLE_RESPONSE           0x8031
  /// @}

  /// @name Routing Table Request / Response
  /// <br> @{
  ///  
  ///
  /// @code
  /// Request:  <transaction sequence number: 1> <start index:1>
  /// Response: <transaction sequence number: 1> <status:1> 
  ///           <routing table entries:1> <start index:1>
  ///           <entry count:1> <entry:5>*
  ///   <entry> = <destination address:2>
  ///             <status:1>
  ///             <next hop:2>
  /// 
  /// @endcode
  ///
  /// The status byte has the following fields:
  /// @code
  ///      Name          Mask        Values
  /// 
  ///   status           0x07     0x00 active
  ///                             0x01 discovery underway
  ///                             0x02 discovery failed
  ///                             0x03 inactive
  ///                             0x04 validation underway
  ///
  ///   flags            0x38
  ///                             0x08 memory constrained 
  ///                             0x10 many-to-one 
  ///                             0x20 route record required
  ///
  ///   reserved         0xC0
  /// @endcode
#define ROUTING_TABLE_REQUEST        0x0032
#define ROUTING_TABLE_RESPONSE       0x8032
  /// @}

  /// @name Binding Table Request / Response
  /// <br> @{
  ///  
  /// 
  /// @code
  /// Request:  <transaction sequence number: 1> <start index:1>
  /// Response: <transaction sequence number: 1>
  ///           <status:1> <binding table entries:1> <start index:1>
  ///           <entry count:1> <entry:14/21>*
  ///   <entry> = <source EUI64:8> <source endpoint:1> <cluster ID:2>
  ///             <dest addr mode:1> <dest:2/8> <dest endpoint:0/1>
  /// @endcode
  /// <br>
  /// @note If Dest. Address Mode = 0x03, then the Long Dest. Address will be 
  /// used and Dest. endpoint will be included.  If Dest. Address Mode = 0x01,
  /// then the Short Dest. Address will be used and there will be no Dest.
  /// endpoint.
  ///
#define BINDING_TABLE_REQUEST        0x0033
#define BINDING_TABLE_RESPONSE       0x8033
  /// @}

  /// @name Leave Request / Response
  /// <br> @{
  ///  
  /// @code
  /// Request:  <transaction sequence number: 1> <EUI64:8> <flags:1>
  ///          The flag bits are:
  ///          0x40 remove children
  ///          0x80 rejoin
  /// Response: <transaction sequence number: 1> <status:1>
  /// @endcode
#define LEAVE_REQUEST                0x0034
#define LEAVE_RESPONSE               0x8034

#define LEAVE_REQUEST_REMOVE_CHILDREN_FLAG 0x40
#define LEAVE_REQUEST_REJOIN_FLAG          0x80
  /// @}

  /// @name Permit Joining Request / Response
  /// <br> @{
  ///  
  /// @code
  /// Request:  <transaction sequence number: 1> 
  ///           <duration:1> <permit authentication:1>
  /// Response: <transaction sequence number: 1> <status:1>
  /// @endcode
#define PERMIT_JOINING_REQUEST       0x0036
#define PERMIT_JOINING_RESPONSE      0x8036
  /// @}

  /// @name Network Update Request / Response
  /// <br> @{
  ///   
  /// @code
  /// Request:  <transaction sequence number: 1> 
  ///           <scan channels:4> <duration:1> <count:0/1> <manager:0/2>
  ///
  ///   If the duration is in 0x00 ... 0x05, then 'count' is present but
  ///   not 'manager'.  Perform 'count' scans of the given duration on the
  ///   given channels.
  ///
  ///   If duration is 0xFE, then 'channels' should have a single channel
  ///   and 'count' and 'manager' are not present.  Switch to the indicated
  ///   channel.
  /// 
  ///   If duration is 0xFF, then 'count' is not present.  Set the active
  ///   channels and the network manager ID to the values given.
  ///
  ///   Unicast requests always get a response, which is INVALID_REQUEST if the
  ///   duration is not a legal value.
  ///
  /// Response: <transaction sequence number: 1> <status:1> 
  ///   <scanned channels:4> <transmissions:2> <failures:2>
  ///   <energy count:1> <energy:1>*
  /// @endcode
#define NWK_UPDATE_REQUEST           0x0038
#define NWK_UPDATE_RESPONSE          0x8038
  /// @}

  /// @name Unsupported
  /// <br> @{
  ///  Not mandatory and not supported.
#define COMPLEX_DESCRIPTOR_REQUEST   0x0010
#define COMPLEX_DESCRIPTOR_RESPONSE  0x8010
#define USER_DESCRIPTOR_REQUEST      0x0011
#define USER_DESCRIPTOR_RESPONSE     0x8011
#define DISCOVERY_REGISTER_REQUEST   0x0012
#define DISCOVERY_REGISTER_RESPONSE  0x8012
#define USER_DESCRIPTOR_SET          0x0014
#define USER_DESCRIPTOR_CONFIRM      0x8014
#define NETWORK_DISCOVERY_REQUEST    0x0030
#define NETWORK_DISCOVERY_RESPONSE   0x8030
#define DIRECT_JOIN_REQUEST          0x0035
#define DIRECT_JOIN_RESPONSE         0x8035


#define CLUSTER_ID_RESPONSE_MINIMUM  0x8000
  /// @}

/** @name ZDO configuration flags.
 *
 * For controlling which ZDO requests are passed to the application.
 * These are normally controlled via the following configuration definitions:
 *
 * EMBER_APPLICATION_RECEIVES_SUPPORTED_ZDO_REQUESTS
 * EMBER_APPLICATION_HANDLES_UNSUPPORTED_ZDO_REQUESTS
 * EMBER_APPLICATION_HANDLES_ENDPOINT_ZDO_REQUESTS
 * EMBER_APPLICATION_HANDLES_BINDING_ZDO_REQUESTS
 *
 * See ember-configuration.h for more information.
 */
#ifdef DOXYGEN_SHOULD_SKIP_THIS
enum EmberZdoConfigurationFlags
#else
typedef int8u EmberZdoConfigurationFlags;
enum
#endif

{
 EMBER_APP_RECEIVES_SUPPORTED_ZDO_REQUESTS  = 0x01,
 EMBER_APP_HANDLES_UNSUPPORTED_ZDO_REQUESTS = 0x02,
 EMBER_APP_HANDLES_ZDO_ENDPOINT_REQUESTS    = 0x04,
 EMBER_APP_HANDLES_ZDO_BINDING_REQUESTS     = 0x08
};

//@} \\END ZigBee Device Object (ZDO) Definitions

#endif // EMBER_TYPES_H

/** @} // END addtogroup
 */

#include "stack/include/zll-types.h"
#include "stack/include/rf4ce-types.h"

