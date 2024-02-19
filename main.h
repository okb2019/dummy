#pragma once

/// @brief The command field (message-type) defines the overall properties of a message
typedef enum {
	C_PRESENTATION = 0,	//!< Sent by a node when they present attached sensors. This is usually done in presentation() at startup.
	C_SET          = 1,	//!< This message is sent from or to a sensor when a sensor value should be updated.
	C_REQ          = 2,	//!< Requests a variable value (usually from an actuator destined for controller).
	C_INTERNAL     = 3,	//!< Internal MySensors messages (also include common messages provided/generated by the library).
	C_STREAM       = 4,	//!< For firmware and other larger chunks of data that need to be divided into pieces.
	C_RESERVED_5   = 5,	//!< C_RESERVED_5
	C_RESERVED_6   = 6,	//!< C_RESERVED_6
	C_INVALID_7    = 7	//!< C_INVALID_7
} MyNetworkRFM69_command_t;


/// @brief Type of sensor (used when presenting sensors)
typedef enum {
	S_DOOR					= 0,	//!< Door sensor, V_TRIPPED, V_ARMED
	S_MOTION				= 1,	//!< Motion sensor, V_TRIPPED, V_ARMED
	S_SMOKE					= 2,	//!< Smoke sensor, V_TRIPPED, V_ARMED
	S_BINARY				= 3,	//!< Binary light or relay, V_STATUS, V_WATT
	S_DIMMER				= 4,	//!< Dimmable light or fan device, V_STATUS (on/off), V_PERCENTAGE (dimmer level 0-100), V_WATT
	S_COVER					= 5,	//!< Blinds or window cover, V_UP, V_DOWN, V_STOP, V_PERCENTAGE (open/close to a percentage)
	S_TEMP					= 6,	//!< Temperature sensor, V_TEMP
	S_HUM					= 7,	//!< Humidity sensor, V_HUM
	S_BARO					= 8,	//!< Barometer sensor, V_PRESSURE, V_FORECAST
	S_WIND					= 9,	//!< Wind sensor, V_WIND, V_GUST
	S_RAIN					= 10,	//!< Rain sensor, V_RAIN, V_RAINRATE
	S_UV					= 11,	//!< Uv sensor, V_UV
	S_WEIGHT				= 12,	//!< Personal scale sensor, V_WEIGHT, V_IMPEDANCE
	S_POWER					= 13,	//!< Power meter, V_WATT, V_KWH, V_VAR, V_VA, V_POWER_FACTOR
	S_HEATER				= 14,	//!< Header device, V_HVAC_SETPOINT_HEAT, V_HVAC_FLOW_STATE, V_TEMP
	S_DISTANCE				= 15,	//!< Distance sensor, V_DISTANCE
	S_LIGHT_LEVEL			= 16,	//!< Light level sensor, V_LIGHT_LEVEL (uncalibrated in percentage),  V_LEVEL (light level in lux)
	S_ARDUINO_NODE			= 17,	//!< Used (internally) for presenting a non-repeating Arduino node
	S_ARDUINO_REPEATER_NODE	= 18,	//!< Used (internally) for presenting a repeating Arduino node
	S_LOCK					= 19,	//!< Lock device, V_LOCK_STATUS
	S_IR					= 20,	//!< IR device, V_IR_SEND, V_IR_RECEIVE
	S_WATER					= 21,	//!< Water meter, V_FLOW, V_VOLUME
	S_AIR_QUALITY			= 22,	//!< Air quality sensor, V_LEVEL
	S_CUSTOM				= 23,	//!< Custom sensor
	S_DUST					= 24,	//!< Dust sensor, V_LEVEL
	S_SCENE_CONTROLLER		= 25,	//!< Scene controller device, V_SCENE_ON, V_SCENE_OFF.
	S_RGB_LIGHT				= 26,	//!< RGB light. Send color component data using V_RGB. Also supports V_WATT
	S_RGBW_LIGHT			= 27,	//!< RGB light with an additional White component. Send data using V_RGBW. Also supports V_WATT
	S_COLOR_SENSOR			= 28,	//!< Color sensor, send color information using V_RGB
	S_HVAC					= 29,	//!< Thermostat/HVAC device. V_HVAC_SETPOINT_HEAT, V_HVAC_SETPOINT_COLD, V_HVAC_FLOW_STATE, V_HVAC_FLOW_MODE, V_TEMP
	S_MULTIMETER			= 30,	//!< Multimeter device, V_VOLTAGE, V_CURRENT, V_IMPEDANCE
	S_SPRINKLER				= 31,	//!< Sprinkler, V_STATUS (turn on/off), V_TRIPPED (if fire detecting device)
	S_WATER_LEAK			= 32,	//!< Water leak sensor, V_TRIPPED, V_ARMED
	S_SOUND					= 33,	//!< Sound sensor, V_TRIPPED, V_ARMED, V_LEVEL (sound level in dB)
	S_VIBRATION				= 34,	//!< Vibration sensor, V_TRIPPED, V_ARMED, V_LEVEL (vibration in Hz)
	S_MOISTURE				= 35,	//!< Moisture sensor, V_TRIPPED, V_ARMED, V_LEVEL (water content or moisture in percentage?)
	S_TEXT					= 36,	//!< LCD text device / Simple information device on controller, V_TEXT
	S_GAS					= 37,	//!< Gas meter, V_FLOW, V_VOLUME
	S_GPS					= 38,	//!< GPS Sensor, V_POSITION
	S_WATER_QUALITY			= 39,	//!< V_TEMP, V_PH, V_ORP, V_EC, V_STATUS
	S_NUMBER				= 40,	//!< For any use of numeric values input/output
  S_SWITCH        = 41,   // only ON / OFF Switch
  S_LIGHT         = 42,    // only ON / OFF Light
  S_SENSOR        = 43    // Any Sensor
} MyNetworkRFM69_sensor_t;

/// @brief Type of internal messages (for internal messages)
typedef enum {
	I_BATTERY_LEVEL				= 0,	//!< Battery level
	I_TIME						= 1,	//!< Time (request/response)
	I_VERSION					= 2,	//!< Version
	I_ID_REQUEST				= 3,	//!< ID request
	I_ID_RESPONSE				= 4,	//!< ID response
	I_INCLUSION_MODE			= 5,	//!< Inclusion mode
	I_CONFIG					= 6,	//!< Config (request/response)
	I_FIND_PARENT_REQUEST		= 7,	//!< Find parent
	I_FIND_PARENT_RESPONSE		= 8,	//!< Find parent response
	I_LOG_MESSAGE				= 9,	//!< Log message
	I_CHILDREN					= 10,	//!< Children
	I_SKETCH_NAME				= 11,	//!< Sketch name
	I_SKETCH_VERSION			= 12,	//!< Sketch version
	I_REBOOT					= 13,	//!< Reboot request
	I_GATEWAY_READY				= 14,	//!< Gateway ready
	I_SIGNING_PRESENTATION		= 15,	//!< Provides signing related preferences (first byte is preference version)
	I_NONCE_REQUEST				= 16,	//!< Request for a nonce
	I_NONCE_RESPONSE			= 17,	//!< Payload is nonce data
	I_HEARTBEAT_REQUEST			= 18,	//!< Heartbeat request
	I_PRESENTATION				= 19,	//!< Presentation message
	I_DISCOVER_REQUEST			= 20,	//!< Discover request
	I_DISCOVER_RESPONSE			= 21,	//!< Discover response
	I_HEARTBEAT_RESPONSE		= 22,	//!< Heartbeat response
	I_LOCKED					= 23,	//!< Node is locked (reason in string-payload)
	I_PING						= 24,	//!< Ping sent to node, payload incremental hop counter
	I_PONG						= 25,	//!< In return to ping, sent back to sender, payload incremental hop counter
	I_REGISTRATION_REQUEST		= 26,	//!< Register request to GW
	I_REGISTRATION_RESPONSE		= 27,	//!< Register response from GW
	I_DEBUG						= 28,	//!< Debug message
	I_SIGNAL_REPORT_REQUEST		= 29,	//!< Device signal strength request
	I_SIGNAL_REPORT_REVERSE		= 30,	//!< Internal
	I_SIGNAL_REPORT_RESPONSE	= 31,	//!< Device signal strength response (RSSI)
	I_PRE_SLEEP_NOTIFICATION	= 32,	//!< Message sent before node is going to sleep
	I_POST_SLEEP_NOTIFICATION	= 33	//!< Message sent after node woke up (if enabled)
} MyNetworkRFM69_internal_t;

/// @brief Type of data stream (for streamed message)
typedef enum {
	ST_FIRMWARE_CONFIG_REQUEST	= 0,	//!< Request new FW, payload contains current FW details
	ST_FIRMWARE_CONFIG_RESPONSE	= 1,	//!< New FW details to initiate OTA FW update
	ST_FIRMWARE_REQUEST			= 2,	//!< Request FW block
	ST_FIRMWARE_RESPONSE		= 3,	//!< Response FW block
	ST_SOUND					= 4,	//!< Sound
	ST_IMAGE					= 5,	//!< Image
	ST_FIRMWARE_CONFIRM	= 6, //!< Mark running firmware as valid (MyOTAFirmwareUpdateNVM + mcuboot)
	ST_FIRMWARE_RESPONSE_RLE = 7,	//!< Response FW block with run length encoded data
} MyNetworkRFM69_stream_t;

/// @brief Type of payload
typedef enum {
	P_STRING				= 0,	//!< Payload type is string
	P_BYTE					= 1,	//!< Payload type is byte
	P_INT16					= 2,	//!< Payload type is INT16
	P_UINT16				= 3,	//!< Payload type is UINT16
	P_LONG32				= 4,	//!< Payload type is INT32
	P_ULONG32				= 5,	//!< Payload type is UINT32
	P_CUSTOM				= 6,	//!< Payload type is binary
	P_FLOAT32				= 7		//!< Payload type is float32
} MyNetworkRFM69_payload_t;

struct dev_type {
  char    name[58];
};

dev_type device_type[] = {


  "S_DOOR",
	"S_MOTION",
	"S_SMOKE",
	"S_BINARY",
	"S_DIMMER",
	"S_COVER",
	"S_TEMP",
	"S_HUM",
	"S_BARO",
	"S_WIND",
	"S_RAIN",
	"S_UV",
	"S_WEIGHT",
	"S_POWER",
	"S_HEATER",
	"S_DISTANCE",
	"S_LIGHT_LEVEL",
	"S_ARDUINO_NODE",
	"S_ARDUINO_REPEATER_NODE",
	"S_LOCK",
	"S_IR",
	"S_WATER",
	"S_AIR_QUALITY",
	"S_CUSTOM",
	"S_DUST",
	"S_SCENE_CONTROLLER",
	"S_RGB_LIGHT",
	"S_RGBW_LIGHT",
	"S_COLOR_SENSOR",
	"S_HVAC",
	"S_MULTIMETER",
	"S_SPRINKLER",
	"S_WATER_LEAK",
	"S_SOUND",
	"S_VIBRATION",
	"S_MOISTURE",
	"text",
	"S_GAS",
	"S_GPS",
	"S_WATER_QUALITY",
	"number",
  "switch",
  "light",
  "sensor"

} ;