#ifndef _utils
#define _utils
#include "common.h"

namespace Utils
{
  /** 
   * @brief Gets the MQTT topic for states
   * 
   * @param topic pointer where to store the topic, capped at MQTT_MAX_TOPIC_LENGTH
   * @param type type of the state, typically type of the device such as "mhz19" or "bme680"
   */
  void getStateTopic(char *topic, const char *type);

  /**
   * @brief Get the MQTT topic for device commands
   * 
   * @param topic pointer where to store the topic, capped at MQTT_MAX_TOPIC_LENGTH
   * @param type type of the device
   */
  void getCommandTopic(char *topic, const char *type);

  /** 
   * @brief Gets the MQTT topic for Home Assistant MQTT Discovery
   * 
   * @param topic pointer where to store the topic, capped at MQTT_MAX_TOPIC_LENGTH
   * @param component One of the supported HA MQTT components, eg. binary_sensor or sensor
   * @param type type of the state, typically type of the sensor such as "mhz19" or "bme680"
   * @param field field name, such as temperature or co2
   * @param topicType discovery topic type, eg. "config" or "attributes"
   */
  void getDiscoveryTopic(char *topic, const char *component, const char *type, const char *field, const char *topicType);

  /**
   * @brief Get a unique ID for creating a HA entity
   * 
   * @param text pointer where to store the ID
   * @param type type of the state, typically type of the sensor such as "mhz19" or "bme680"
   * @param field field name, such as temperature or co2
   */
  void getUniqueID(char *text, const char *type, const char *field);

  /**
 * @brief Get the uptime in milliseconds. Effectively a 64bit version of the millis() function
 * 
 * @return int64_t the current uptime in milliseconds.
 */
  uint64_t uptime();

  /**
   * @brief Converts a hex char to int
   * 
   * @param input a hex char
   * @return int the numeric value of the char
   */
  int char2int(char input);
};

#endif
