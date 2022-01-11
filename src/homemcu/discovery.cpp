#include "homemcu.h"

#include <ESP8266WiFi.h>

void HomeMCU::updateDiscovery(DiscoveryData &data)
{
  char configTopic[MQTT_MAX_TOPIC_LENGTH];
  char attributesTopic[MQTT_MAX_TOPIC_LENGTH];
  char id[MQTT_MAX_TOPIC_LENGTH];
  Utils::getDiscoveryTopic(configTopic, data.component, data.type, data.field, "config");
  Utils::getDiscoveryTopic(attributesTopic, data.component, data.type, data.field, "attributes");
  Utils::getUniqueID(id, data.type, data.field);

  { // attributes
    DynamicJsonDocument json(1024);
    json["mcu_name"] = HomeMCU::name;
    if (strcmp(HomeMCU::name, data.name) != 0)
      json["sensor_name"] = data.name;
    json["sensor_type"] = data.sensorName;
    json["ip"] = WiFi.localIP();
    json["mac"] = WiFi.macAddress();
    for (auto const &pair : data.attributes)
      json[pair.first] = pair.second;

    String msg;
    serializeJson(json, msg);
    HomeMCU::client.publish(attributesTopic, msg.c_str());
  }
  { // discovery
    char name[256];
    char valueTemplate[256];
    snprintf(name, sizeof(name), "%s %s", data.name, data.fieldName);
    snprintf(valueTemplate, sizeof(valueTemplate), "{{ (value_json.%s%s) | round(2) }}", data.field, data.valueMutator ? data.valueMutator : "");

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

void HomeMCU::deleteDiscovery(const char *component, const char *type, const char *field)
{
  char configTopic[MQTT_MAX_TOPIC_LENGTH];
  Utils::getDiscoveryTopic(configTopic, component, type, field, "config");
  HomeMCU::client.publish(configTopic, "");
}
