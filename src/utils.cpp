#include "utils.h"

#include <ESP8266WiFi.h>

void Utils::getStateTopic(char *topic, const char *type)
{
  snprintf(topic, MQTT_MAX_TOPIC_LENGTH, MQTT_PREFIX "%s/%s", WiFi.macAddress().c_str(), type);
}

void Utils::getDiscoveryTopic(char *topic, const char *component, const char *type, const char *field, const char *topicType)
{
  uint8_t mac[6];
  WiFi.macAddress(mac);
  char macStr[13] = {0};
  snprintf(macStr, sizeof(mac), "%02X%02X%02X%02X%02X%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  snprintf(topic, MQTT_MAX_TOPIC_LENGTH, MQTT_HA_DISCOVERY_PREFIX "%s/homemcu_%s_%s_%s/%s", component, macStr, type, field, topicType);
}

void Utils::getUniqueID(char *text, const char *type, const char *field)
{
  uint8_t mac[6];
  WiFi.macAddress(mac);
  char macStr[13] = {0};
  snprintf(macStr, sizeof(mac), "%02X%02X%02X%02X%02X%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  snprintf(text, MQTT_MAX_TOPIC_LENGTH, "homemcu_%s_%s_%s", macStr, type, field);
}

uint32_t millisHigh = 0;
unsigned long lastMillis = 0;
uint64_t Utils::uptime()
{
  int64_t millisLow = millis();

  if (lastMillis > millisLow)
    millisHigh++;

  lastMillis = millisLow;

  return ((uint64_t)millisHigh << 32) + millisLow;
}
