#include "homemcu.h"

#include <ESP8266WiFi.h>

void HomeMCU::updateDiscovery(SensorDiscoveryData &data)
{
  char configTopic[MQTT_MAX_TOPIC_LENGTH];
  char attributesTopic[MQTT_MAX_TOPIC_LENGTH];
  char id[MQTT_MAX_TOPIC_LENGTH];
  Utils::getDiscoveryTopic(configTopic, "sensor", data.type, data.field, "config");
  Utils::getDiscoveryTopic(attributesTopic, "sensor", data.type, data.field, "attributes");
  Utils::getUniqueID(id, data.type, data.field);

  if (!data.enabled)
  {
    HomeMCU::client.publish(configTopic, "");
    HomeMCU::client.publish(attributesTopic, "");
    return;
  }

  { // attributes
    DynamicJsonDocument json(2048);
    json["mcu_name"] = HomeMCU::name;
    json["sensor_type"] = data.sensorName;
    json["ip"] = WiFi.localIP();
    json["mac"] = WiFi.macAddress();
    json["config_checksum"] = HomeMCU::currentConfigChecksum;
    json["build_ts"] = BUILD_TIMESTAMP;
    for (auto const &pair : data.attributes)
      json[pair.first] = pair.second;

    String msg;
    serializeJson(json, msg);
    HomeMCU::client.publish(attributesTopic, msg.c_str());
  }
  { // discovery
    char name[256];
    snprintf(name, sizeof(name), "%s %s", data.name, data.fieldName);

    DynamicJsonDocument json(1024);
    json["unique_id"] = id;
    if (data.deviceClass)
      json["device_class"] = data.deviceClass;
    json["state_topic"] = data.stateTopic;
    json["json_attributes_topic"] = attributesTopic;
    json["availability_topic"] = HomeMCU::statusTopic;
    json["availability_template"] = "{{ 'online' if value_json.status == 'online' else 'offline' }}";
    json["name"] = name;
    if (data.unitOfMeasurement)
      json["unit_of_measurement"] = data.unitOfMeasurement;

    char valueTemplate[256];
    snprintf(valueTemplate, sizeof(valueTemplate), "{{ (value_json.%s%s) | round(2) }}", data.field, data.valueMutator ? data.valueMutator : "");
    json["value_template"] = valueTemplate;

    if (data.icon)
      json["icon"] = data.icon;

    JsonArray identifiers = json["device"].createNestedArray("identifiers");
    identifiers.add(WiFi.macAddress());
    json["device"]["name"] = HomeMCU::name;
    json["device"]["model"] = "HomeMCU";

    String msg;
    serializeJson(json, msg);
    HomeMCU::client.publish(configTopic, msg.c_str());
  }
}

void HomeMCU::updateDiscovery(LightDiscoveryData &data)
{
  char configTopic[MQTT_MAX_TOPIC_LENGTH];
  char attributesTopic[MQTT_MAX_TOPIC_LENGTH];
  char id[MQTT_MAX_TOPIC_LENGTH];
  Utils::getDiscoveryTopic(configTopic, "light", data.type, data.field, "config");
  Utils::getDiscoveryTopic(attributesTopic, "light", data.type, data.field, "attributes");
  Utils::getUniqueID(id, data.type, data.field);

  if (!data.enabled)
  {
    HomeMCU::client.publish(configTopic, "");
    HomeMCU::client.publish(attributesTopic, "");
    return;
  }

  { // attributes
    DynamicJsonDocument json(2048);
    json["mcu_name"] = HomeMCU::name;
    json["ip"] = WiFi.localIP();
    json["mac"] = WiFi.macAddress();
    json["config_checksum"] = HomeMCU::currentConfigChecksum;
    json["build_ts"] = BUILD_TIMESTAMP;
    for (auto const &pair : data.attributes)
      json[pair.first] = pair.second;

    String msg;
    serializeJson(json, msg);
    HomeMCU::client.publish(attributesTopic, msg.c_str());
  }
  { // discovery
    char name[256];
    snprintf(name, sizeof(name), "%s %s", data.name, data.fieldName);

    DynamicJsonDocument json(1024);
    json["unique_id"] = id;
    if (data.deviceClass)
      json["device_class"] = data.deviceClass;
    json["state_topic"] = data.stateTopic;
    json["json_attributes_topic"] = attributesTopic;
    json["availability_topic"] = HomeMCU::statusTopic;
    json["availability_template"] = "{{ 'online' if value_json.status == 'online' else 'offline' }}";
    json["name"] = name;

    if (data.icon)
      json["icon"] = data.icon;

    if (data.commandTopic)
      json["command_topic"] = data.commandTopic;

    if (data.colorMode)
    {
      json["schema"] = "json";
      json["color_mode"] = true;
      json["effect"] = true;

      JsonArray effects = json.createNestedArray("effect_list");
      effects.add("stable");
      effects.add("gradient");
      effects.add("custom");
      effects.add("sunrise");
      effects.add("colorloop");

      JsonArray colorModes = json.createNestedArray("supported_color_modes");
      colorModes.add(data.colorMode);
    }

    JsonArray identifiers = json["device"].createNestedArray("identifiers");
    identifiers.add(WiFi.macAddress());
    json["device"]["name"] = HomeMCU::name;
    json["device"]["model"] = "HomeMCU";

    String msg;
    serializeJson(json, msg);
    HomeMCU::client.publish(configTopic, msg.c_str());
  }
}

void HomeMCU::updateDiscovery(NumberDiscoveryData &data)
{
  char configTopic[MQTT_MAX_TOPIC_LENGTH];
  char attributesTopic[MQTT_MAX_TOPIC_LENGTH];
  char id[MQTT_MAX_TOPIC_LENGTH];
  Utils::getDiscoveryTopic(configTopic, "number", data.type, data.field, "config");
  Utils::getDiscoveryTopic(attributesTopic, "number", data.type, data.field, "attributes");
  Utils::getUniqueID(id, data.type, data.field);

  if (!data.enabled)
  {
    HomeMCU::client.publish(configTopic, "");
    HomeMCU::client.publish(attributesTopic, "");
    return;
  }

  { // attributes
    DynamicJsonDocument json(2048);
    json["mcu_name"] = HomeMCU::name;
    json["ip"] = WiFi.localIP();
    json["mac"] = WiFi.macAddress();
    json["config_checksum"] = HomeMCU::currentConfigChecksum;
    json["build_ts"] = BUILD_TIMESTAMP;
    for (auto const &pair : data.attributes)
      json[pair.first] = pair.second;

    String msg;
    serializeJson(json, msg);
    HomeMCU::client.publish(attributesTopic, msg.c_str());
  }
  { // discovery
    char name[256];
    snprintf(name, sizeof(name), "%s %s", data.name, data.fieldName);

    DynamicJsonDocument json(1024);
    json["unique_id"] = id;
    json["state_topic"] = data.stateTopic;
    json["json_attributes_topic"] = attributesTopic;
    json["availability_topic"] = HomeMCU::statusTopic;
    json["availability_template"] = "{{ 'online' if value_json.status == 'online' else 'offline' }}";
    json["name"] = name;
    if (data.unitOfMeasurement)
      json["unit_of_measurement"] = data.unitOfMeasurement;

    if (data.valueTemplate != nullptr)
      json["value_template"] = data.valueTemplate;

    if (data.icon)
      json["icon"] = data.icon;

    if (data.commandTopic)
      json["command_topic"] = data.commandTopic;

    if (data.commandTemplate)
      json["command_template"] = data.commandTemplate;

    if (data.min)
      json["min"] = data.min;

    if (data.max)
      json["max"] = data.max;

    if (data.options)
    {
      JsonArray options = json.createNestedArray("options");
      for (const char *opt : *data.options)
      {
        options.add(opt);
      }
    }

    JsonArray identifiers = json["device"].createNestedArray("identifiers");
    identifiers.add(WiFi.macAddress());
    json["device"]["name"] = HomeMCU::name;
    json["device"]["model"] = "HomeMCU";

    String msg;
    serializeJson(json, msg);
    HomeMCU::client.publish(configTopic, msg.c_str());
  }
}

void HomeMCU::updateDiscovery(SelectDiscoveryData &data)
{
  char configTopic[MQTT_MAX_TOPIC_LENGTH];
  char attributesTopic[MQTT_MAX_TOPIC_LENGTH];
  char id[MQTT_MAX_TOPIC_LENGTH];
  Utils::getDiscoveryTopic(configTopic, "select", data.type, data.field, "config");
  Utils::getDiscoveryTopic(attributesTopic, "select", data.type, data.field, "attributes");
  Utils::getUniqueID(id, data.type, data.field);

  if (!data.enabled)
  {
    HomeMCU::client.publish(configTopic, "");
    HomeMCU::client.publish(attributesTopic, "");
    return;
  }

  { // attributes
    DynamicJsonDocument json(2048);
    json["mcu_name"] = HomeMCU::name;
    json["ip"] = WiFi.localIP();
    json["mac"] = WiFi.macAddress();
    json["config_checksum"] = HomeMCU::currentConfigChecksum;
    json["build_ts"] = BUILD_TIMESTAMP;
    for (auto const &pair : data.attributes)
      json[pair.first] = pair.second;

    String msg;
    serializeJson(json, msg);
    HomeMCU::client.publish(attributesTopic, msg.c_str());
  }
  { // discovery
    char name[256];
    snprintf(name, sizeof(name), "%s %s", data.name, data.fieldName);

    DynamicJsonDocument json(1024);
    json["unique_id"] = id;
    json["state_topic"] = data.stateTopic;
    json["json_attributes_topic"] = attributesTopic;
    json["availability_topic"] = HomeMCU::statusTopic;
    json["availability_template"] = "{{ 'online' if value_json.status == 'online' else 'offline' }}";
    json["name"] = name;

    if (data.valueTemplate != nullptr)
      json["value_template"] = data.valueTemplate;

    if (data.icon)
      json["icon"] = data.icon;

    if (data.commandTopic)
      json["command_topic"] = data.commandTopic;

    if (data.commandTemplate)
      json["command_template"] = data.commandTemplate;

    if (data.options)
    {
      JsonArray options = json.createNestedArray("options");
      for (const char *opt : *data.options)
      {
        options.add(opt);
      }
    }

    JsonArray identifiers = json["device"].createNestedArray("identifiers");
    identifiers.add(WiFi.macAddress());
    json["device"]["name"] = HomeMCU::name;
    json["device"]["model"] = "HomeMCU";

    String msg;
    serializeJson(json, msg);
    HomeMCU::client.publish(configTopic, msg.c_str());
  }
}
