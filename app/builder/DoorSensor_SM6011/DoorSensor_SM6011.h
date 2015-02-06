// This file is generated by Ember Desktop.  Please do not edit manually.
//
//

// Enclosing macro to prevent multiple inclusion
#ifndef __APP_DOORSENSOR_SM6011_H__
#define __APP_DOORSENSOR_SM6011_H__


/**** Included Header Section ****/

/**** ZCL Section ****/
#define ZA_PROMPT "DoorSensor_SM6011"
#define ZCL_USING_BASIC_CLUSTER_SERVER
#define ZCL_USING_IDENTIFY_CLUSTER_CLIENT
#define ZCL_USING_IDENTIFY_CLUSTER_SERVER
#define ZCL_USING_OTA_BOOTLOAD_CLUSTER_CLIENT
#define ZCL_USING_IAS_ZONE_CLUSTER_SERVER
#define EMBER_AF_DEFAULT_RESPONSE_POLICY_ALWAYS

/**** Cluster endpoint counts ****/
#define EMBER_AF_BASIC_CLUSTER_SERVER_ENDPOINT_COUNT 1
#define EMBER_AF_IDENTIFY_CLUSTER_CLIENT_ENDPOINT_COUNT 1
#define EMBER_AF_IDENTIFY_CLUSTER_SERVER_ENDPOINT_COUNT 1
#define EMBER_AF_OTA_BOOTLOAD_CLUSTER_CLIENT_ENDPOINT_COUNT 1
#define EMBER_AF_IAS_ZONE_CLUSTER_SERVER_ENDPOINT_COUNT 1

/**** CLI Section ****/
#define EMBER_AF_GENERATE_CLI
#define EMBER_COMMAND_INTEPRETER_HAS_DESCRIPTION_FIELD

/**** Security Section ****/
#define EMBER_KEY_TABLE_SIZE 0
#define EMBER_AF_HAS_SECURITY_PROFILE_HA

/**** Network Section ****/
#define EMBER_SUPPORTED_NETWORKS 1
#define EMBER_AF_NETWORK_INDEX_PRIMARY 0
#define EMBER_AF_DEFAULT_NETWORK_INDEX EMBER_AF_NETWORK_INDEX_PRIMARY
#define EMBER_AF_HAS_END_DEVICE_NETWORK
#define EMBER_AF_HAS_SLEEPY_NETWORK
#define EMBER_AF_TX_POWER_MODE EMBER_TX_POWER_MODE_USE_TOKEN

/*** Bindings section ****/
#define EMBER_BINDING_TABLE_SIZE 2

/**** HAL Section ****/
#define ZA_CLI_FULL

/**** Callback Section ****/
#define EMBER_CALLBACK_COUNTER_HANDLER
#define EMBER_APPLICATION_HAS_COUNTER_HANDLER
#define EMBER_CALLBACK_EZSP_COUNTER_ROLLOVER_HANDLER
#define EZSP_APPLICATION_HAS_COUNTER_ROLLOVER_HANDLER
#define EMBER_CALLBACK_EEPROM_INIT
#define EMBER_CALLBACK_EEPROM_NOTE_INITIALIZED_STATE
#define EMBER_CALLBACK_EEPROM_SHUTDOWN
#define EMBER_CALLBACK_SCHEDULE_POLL_EVENT
#define EMBER_CALLBACK_POLL_COMPLETE
#define EMBER_APPLICATION_HAS_POLL_COMPLETE_HANDLER
#define EMBER_CALLBACK_PRE_NCP_RESET
#define EMBER_CALLBACK_EZSP_POLL_COMPLETE
#define EZSP_APPLICATION_HAS_POLL_COMPLETE_HANDLER
#define EMBER_CALLBACK_ADD_TO_CURRENT_APP_TASKS
#define EMBER_CALLBACK_REMOVE_FROM_CURRENT_APP_TASKS
#define EMBER_CALLBACK_GET_CURRENT_APP_TASKS
#define EMBER_CALLBACK_GET_LONG_POLL_INTERVAL_MS
#define EMBER_CALLBACK_GET_LONG_POLL_INTERVAL_QS
#define EMBER_CALLBACK_SET_LONG_POLL_INTERVAL_MS
#define EMBER_CALLBACK_SET_LONG_POLL_INTERVAL_QS
#define EMBER_CALLBACK_GET_SHORT_POLL_INTERVAL_MS
#define EMBER_CALLBACK_GET_SHORT_POLL_INTERVAL_QS
#define EMBER_CALLBACK_SET_SHORT_POLL_INTERVAL_MS
#define EMBER_CALLBACK_SET_SHORT_POLL_INTERVAL_QS
#define EMBER_CALLBACK_GET_CURRENT_POLL_INTERVAL_MS
#define EMBER_CALLBACK_GET_CURRENT_POLL_INTERVAL_QS
#define EMBER_CALLBACK_GET_WAKE_TIMEOUT_MS
#define EMBER_CALLBACK_GET_WAKE_TIMEOUT_QS
#define EMBER_CALLBACK_SET_WAKE_TIMEOUT_MS
#define EMBER_CALLBACK_SET_WAKE_TIMEOUT_QS
#define EMBER_CALLBACK_GET_WAKE_TIMEOUT_BITMASK
#define EMBER_CALLBACK_SET_WAKE_TIMEOUT_BITMASK
#define EMBER_CALLBACK_GET_CURRENT_POLL_CONTROL
#define EMBER_CALLBACK_GET_DEFAULT_POLL_CONTROL
#define EMBER_CALLBACK_SET_DEFAULT_POLL_CONTROL
#define EMBER_CALLBACK_START_MOVE
#define EMBER_CALLBACK_STOP_MOVE
#define EMBER_CALLBACK_IAS_ZONE_CLUSTER_IAS_ZONE_CLUSTER_SERVER_ATTRIBUTE_CHANGED
#define EMBER_CALLBACK_IAS_ZONE_CLUSTER_ZONE_ENROLL_RESPONSE
#define EMBER_CALLBACK_IAS_ZONE_CLUSTER_IAS_ZONE_CLUSTER_SERVER_INIT
#define EMBER_CALLBACK_IAS_ZONE_CLUSTER_IAS_ZONE_CLUSTER_SERVER_TICK
#define EMBER_CALLBACK_IDENTIFY_CLUSTER_IDENTIFY_CLUSTER_SERVER_INIT
#define EMBER_CALLBACK_IDENTIFY_CLUSTER_IDENTIFY_CLUSTER_SERVER_TICK
#define EMBER_CALLBACK_IDENTIFY_CLUSTER_IDENTIFY_CLUSTER_SERVER_ATTRIBUTE_CHANGED
#define EMBER_CALLBACK_IDENTIFY_CLUSTER_IDENTIFY
#define EMBER_CALLBACK_IDENTIFY_CLUSTER_IDENTIFY_QUERY
#define EMBER_CALLBACK_CHECK_FOR_SLEEP
#define EMBER_CALLBACK_NCP_IS_AWAKE_ISR
#define EMBER_CALLBACK_GET_CURRENT_SLEEP_CONTROL
#define EMBER_CALLBACK_GET_DEFAULT_SLEEP_CONTROL
#define EMBER_CALLBACK_SET_DEFAULT_SLEEP_CONTROL
#define EMBER_CALLBACK_UNUSED_PAN_ID_FOUND
#define EMBER_CALLBACK_JOINABLE_NETWORK_FOUND
#define EMBER_CALLBACK_SCAN_ERROR
#define EMBER_CALLBACK_FIND_UNUSED_PAN_ID_AND_FORM
#define EMBER_CALLBACK_START_SEARCH_FOR_JOINABLE_NETWORK
#define EMBER_CALLBACK_GET_FORM_AND_JOIN_EXTENDED_PAN_ID
#define EMBER_CALLBACK_SET_FORM_AND_JOIN_EXTENDED_PAN_ID
#define EMBER_CALLBACK_CONFIGURE_REPORTING_COMMAND
#define EMBER_CALLBACK_READ_REPORTING_CONFIGURATION_COMMAND
#define EMBER_CALLBACK_CLEAR_REPORT_TABLE
#define EMBER_CALLBACK_REPORTING_ATTRIBUTE_CHANGE
#define EMBER_CALLBACK_HAL_BUTTON_ISR
/**** Debug printing section ****/

// Global switch
#define EMBER_AF_PRINT_ENABLE
// Individual areas
#define EMBER_AF_PRINT_CORE 0x0001
#define EMBER_AF_PRINT_DEBUG 0x0002
#define EMBER_AF_PRINT_APP 0x0004
#define EMBER_AF_PRINT_SECURITY 0x0008
#define EMBER_AF_PRINT_ATTRIBUTES 0x0010
#define EMBER_AF_PRINT_REPORTING 0x0020
#define EMBER_AF_PRINT_SERVICE_DISCOVERY 0x0040
#define EMBER_AF_PRINT_REGISTRATION 0x0080
#define EMBER_AF_PRINT_ZDO 0x0101
#define EMBER_AF_PRINT_CUSTOM1 0x0102
#define EMBER_AF_PRINT_CUSTOM2 0x0104
#define EMBER_AF_PRINT_CUSTOM3 0x0108
#define EMBER_AF_PRINT_BASIC_CLUSTER 0x0110
#define EMBER_AF_PRINT_IDENTIFY_CLUSTER 0x0120
#define EMBER_AF_PRINT_OTA_BOOTLOAD_CLUSTER 0x0140
#define EMBER_AF_PRINT_IAS_ZONE_CLUSTER 0x0180
#define EMBER_AF_PRINT_BITS { 0xFD, 0xFF }
#define EMBER_AF_PRINT_NAMES { \
  "Core",\
  "Debug",\
  "Application",\
  "Security",\
  "Attributes",\
  "Reporting",\
  "Service discovery",\
  "Registration",\
  "ZDO (ZigBee Device Object)",\
  "Custom messages (1)",\
  "Custom messages (2)",\
  "Custom messages (3)",\
  "Basic",\
  "Identify",\
  "Over the Air Bootloading",\
  "IAS Zone",\
  NULL\
}
#define EMBER_AF_PRINT_NAME_NUMBER 16


#define EMBER_AF_SUPPORT_COMMAND_DISCOVERY
/**** App plugins section ****/
#define EMBER_AF_PLUGIN_ADDRESS_TABLE
// User options for plugin Address Table
#define EMBER_AF_PLUGIN_ADDRESS_TABLE_SIZE 2
#define EMBER_AF_PLUGIN_ADDRESS_TABLE_TRUST_CENTER_CACHE_SIZE 2
#define EMBER_AF_PLUGIN_BATTERY_MONITOR
// User options for plugin Battery Monitor Code
#define EMBER_AF_PLUGIN_BATTERY_MONITOR_MONITOR_TIMEOUT 30
#define EMBER_AF_PLUGIN_COUNTERS
// User options for plugin Counters
#define EMBER_AF_PLUGIN_EEPROM
// User options for plugin EEPROM
#define EMBER_AF_PLUGIN_EEPROM_PARTIAL_WORD_STORAGE_COUNT 2
#define EMBER_AF_PLUGIN_END_DEVICE_SUPPORT
// User options for plugin End Device Support
#define EMBER_AF_PLUGIN_END_DEVICE_SUPPORT_SHORT_POLL_INTERVAL_SECONDS 1
#define EMBER_AF_PLUGIN_END_DEVICE_SUPPORT_LONG_POLL_INTERVAL_SECONDS 300
#define EMBER_AF_PLUGIN_END_DEVICE_SUPPORT_WAKE_TIMEOUT_SECONDS 3
#define EMBER_AF_PLUGIN_END_DEVICE_SUPPORT_WAKE_TIMEOUT_BITMASK 24
#define EMBER_AF_PLUGIN_END_DEVICE_SUPPORT_MAX_MISSED_POLLS 10
#define EMBER_AF_REJOIN_ATTEMPTS_MAX 3
#define EMBER_AF_PLUGIN_IAS_ZONE_SERVER
// User options for plugin IAS Zone Server
#define EMBER_AF_PLUGIN_IAS_ZONE_SERVER_ZONE_TYPE 21
#define EMBER_AF_PLUGIN_IDENTIFY
#define EMBER_AF_PLUGIN_IDENTIFY_FEEDBACK
// User options for plugin Identify Feedback
#define EMBER_AF_PLUGIN_IDENTIFY_FEEDBACK_LED_FEEDBACK
#define EMBER_AF_PLUGIN_IDLE_SLEEP
// User options for plugin Idle/Sleep
#define EMBER_AF_PLUGIN_IDLE_SLEEP_STAY_AWAKE_WHEN_NOT_JOINED
#define EMBER_AF_PLUGIN_LED_BLINK
#define EMBER_AF_PLUGIN_NETWORK_FIND
#define EMBER_AF_DISABLE_FORM_AND_JOIN_TICK
// User options for plugin Network Find
#define EMBER_AF_PLUGIN_NETWORK_FIND_CHANNEL_MASK 0x0318C800UL
#define EMBER_AF_PLUGIN_NETWORK_FIND_RADIO_TX_POWER 3
#define EMBER_AF_PLUGIN_NETWORK_FIND_EXTENDED_PAN_ID { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }
#define EMBER_AF_PLUGIN_NETWORK_FIND_DURATION 5
#define EMBER_AF_PLUGIN_NETWORK_FIND_JOINABLE_SCAN_TIMEOUT_MINUTES 1
#define EMBER_AF_PLUGIN_REPORTING
// User options for plugin Reporting
#define EMBER_AF_PLUGIN_REPORTING_TABLE_SIZE 5
#define EMBER_AF_PLUGIN_SENSOR_IFACE
// User options for plugin Sensor Interface Code
#define EMBER_AF_PLUGIN_SENSOR_IFACE_BUTTON_TIMEOUT 1000
#define EMBER_AF_PLUGIN_SENSOR_IFACE_ASSERT_DEBOUNCE 100
#define EMBER_AF_PLUGIN_SENSOR_IFACE_DEASSERT_DEBOUNCE 100
#define EMBER_AF_PLUGIN_SENSOR_IFACE_SENSOR_POLARITY 0
#define EMBER_AF_PLUGIN_SENSOR_IFACE_REJOIN_TIME 30
#define EMBER_AF_PLUGIN_SENSOR_IFACE_BUTTON_GATE 0


// Custom macros                //Ray
#ifdef APP_SERIAL
#undef APP_SERIAL
#endif
#define APP_SERIAL 1
//
#ifdef EMBER_ASSERT_SERIAL_PORT
#undef EMBER_ASSERT_SERIAL_PORT
#endif
#define EMBER_ASSERT_SERIAL_PORT 1
//
#ifdef EMBER_AF_BAUD_RATE
#undef EMBER_AF_BAUD_RATE
#endif
#define EMBER_AF_BAUD_RATE 115200
//
#ifdef EMBER_SERIAL0_MODE
#undef EMBER_SERIAL0_MODE
#endif
#define EMBER_SERIAL0_MODE EMBER_SERIAL_FIFO
//
#ifdef EMBER_SERIAL0_RX_QUEUE_SIZE
#undef EMBER_SERIAL0_RX_QUEUE_SIZE
#endif
#define EMBER_SERIAL0_RX_QUEUE_SIZE 128
//
#ifdef EMBER_SERIAL0_TX_QUEUE_SIZE
#undef EMBER_SERIAL0_TX_QUEUE_SIZE
#endif
#define EMBER_SERIAL0_TX_QUEUE_SIZE 128
//
#ifdef EMBER_SERIAL0_BLOCKING
#undef EMBER_SERIAL0_BLOCKING
#endif
#define EMBER_SERIAL0_BLOCKING
//
#ifdef EMBER_SERIAL1_MODE
#undef EMBER_SERIAL1_MODE
#endif
#define EMBER_SERIAL1_MODE EMBER_SERIAL_FIFO
//
#ifdef EMBER_SERIAL1_RX_QUEUE_SIZE
#undef EMBER_SERIAL1_RX_QUEUE_SIZE
#endif
#define EMBER_SERIAL1_RX_QUEUE_SIZE 128
//
#ifdef EMBER_SERIAL1_TX_QUEUE_SIZE
#undef EMBER_SERIAL1_TX_QUEUE_SIZE
#endif
#define EMBER_SERIAL1_TX_QUEUE_SIZE 128
//
#ifdef EMBER_SERIAL1_BLOCKING
#undef EMBER_SERIAL1_BLOCKING
#endif
#define EMBER_SERIAL1_BLOCKING
//
#ifdef EMBER_AF_ENABLE_CUSTOM_COMMANDS
#undef EMBER_AF_ENABLE_CUSTOM_COMMANDS
#endif
#define EMBER_AF_ENABLE_CUSTOM_COMMANDS VALUE
//
#ifdef CUSTOM_TOKEN_HEADER
#undef CUSTOM_TOKEN_HEADER
#endif
#define CUSTOM_TOKEN_HEADER "app/framework/plugin/sensor-iface/sensor-iface-tokens.h"


#endif // __APP_DOORSENSOR_SM6011_H__
