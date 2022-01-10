#include "utils.h"

#include <ESP8266WiFi.h>

void utils::getStateTopic(char *topic, const char *type)
{
  snprintf(topic, MQTT_MAX_TOPIC_LENGTH, MQTT_PREFIX "%s/%s", WiFi.macAddress().c_str(), type);
}

void utils::getDiscoveryTopic(char *topic, const char *component, const char *type, const char *field, const char *topicType)
{
  uint8_t mac[6];
  WiFi.macAddress(mac);
  char macStr[13] = {0};
  sprintf(macStr, "%02X%02X%02X%02X%02X%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  snprintf(topic, MQTT_MAX_TOPIC_LENGTH, MQTT_HA_DISCOVERY_PREFIX "%s/homemcu_%s_%s_%s/%s", component, macStr, type, field, topicType);
}

void utils::getUniqueID(char *text, const char *type, const char *field)
{
  uint8_t mac[6];
  WiFi.macAddress(mac);
  char macStr[13] = {0};
  sprintf(macStr, "%02X%02X%02X%02X%02X%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  snprintf(text, MQTT_MAX_TOPIC_LENGTH, "homemcu_%s_%s_%s", macStr, type, field);
}
