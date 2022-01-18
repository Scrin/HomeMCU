#ifndef _homemcu
#define _homemcu

#include "common.h"

namespace HomeMCU
{
  // Data for creating Home Assistant MQTT Discovery messages for sensor entities. Optional fields can be omitted with nullptr
  struct SensorDiscoveryData
  {
    const char *type;                     // Required. Device type of the state, use the Class::type when setting this
    const char *field;                    // Required. Field name as it appears in the state json
    const char *stateTopic;               // Required. State topic
    const char *name;                     // Required. Human friendly name of device or sensor-specific name
    const char *fieldName;                // Required. Human friendly name of the sensor reading
    const char *sensorName;               // Required. Full name of the physical sensor providing the data, such as "MH-Z19" or "BME680"
    const char *deviceClass;              // Optional. One of the supported device classes for the used HA MQTT component
    const char *unitOfMeasurement;        // Optional. Unit of measurement. Note that HA will only show a line graph when unit of measurement is specified
    const char *valueMutator;             // Optional. Value mutator for the json template, for example " / 1000.0"
    const char *icon;                     // Optional. Icon for the entity
    std::map<String, String> &attributes; // Required, but can be empty. Additional attributes for the entity attributes
  };
  // Data for creating Home Assistant MQTT Discovery messages for light entities. Optional fields can be omitted with nullptr
  struct LightDiscoveryData
  {
    const char *type;                     // Required. Device type of the state, use the Class::type when setting this
    const char *field;                    // Required. Field name as it appears in the state json
    const char *stateTopic;               // Required. State topic
    const char *name;                     // Required. Human friendly name of device or sensor-specific name
    const char *fieldName;                // Required. Human friendly name of the sensor reading
    const char *deviceClass;              // Optional. One of the supported device classes for the used HA MQTT component
    const char *icon;                     // Optional. Icon for the entity
    const char *commandTopic;             // Optional. Sets the command topic
    std::map<String, String> &attributes; // Required, but can be empty. Additional attributes for the entity attributes
  };
  // Data for creating Home Assistant MQTT Discovery messages for number input entities. Optional fields can be omitted with nullptr
  struct NumberDiscoveryData
  {
    const char *type;                     // Required. Device type of the state, use the Class::type when setting this
    const char *field;                    // Required. Field name as it appears in the state json
    const char *stateTopic;               // Required. State topic
    const char *name;                     // Required. Human friendly name of device or sensor-specific name
    const char *fieldName;                // Required. Human friendly name of the sensor reading
    int min;                              // Required. Minimum value
    int max;                              // Required. Maximum value
    int step;                             // Required. Stepping for the value
    const char *unitOfMeasurement;        // Optional. Unit of measurement. Note that HA will only show a line graph when unit of measurement is specified
    const char *icon;                     // Optional. Icon for the entity
    const char *commandTopic;             // Optional. Sets the command topic
    const char *commandTemplate;          // Optional. Sets the command template
    const char *valueTemplate;            // Optional. Sets the value template
    std::vector<const char *> *options;   // Optional. Sets the options
    std::map<String, String> &attributes; // Required, but can be empty. Additional attributes for the entity attributes
  };
  // Data for creating Home Assistant MQTT Discovery messages for select input entities. Optional fields can be omitted with nullptr
  struct SelectDiscoveryData
  {
    const char *type;                     // Required. Device type of the state, use the Class::type when setting this
    const char *field;                    // Required. Field name as it appears in the state json
    const char *stateTopic;               // Required. State topic
    const char *name;                     // Required. Human friendly name of device or sensor-specific name
    const char *fieldName;                // Required. Human friendly name of the sensor reading
    const char *icon;                     // Optional. Icon for the entity
    const char *commandTopic;             // Optional. Sets the command topic
    const char *commandTemplate;          // Optional. Sets the command template
    const char *valueTemplate;            // Optional. Sets the value template
    std::vector<const char *> *options;   // Optional. Sets the options
    std::map<String, String> &attributes; // Required, but can be empty. Additional attributes for the entity attributes
  };

  extern PubSubClient client;
  extern char *name;
  extern uint32_t currentConfigChecksum;
  extern char statusTopic[MQTT_MAX_TOPIC_LENGTH];

  void setup();
  void loop();

  void updateState();
  void mqttCallback(char *topic, byte *payload, unsigned int length);
  void setSubscriptions();
  void checkConnection();

  void updateDiscovery(SensorDiscoveryData &data);
  void updateDiscovery(LightDiscoveryData &data);
  void updateDiscovery(NumberDiscoveryData &data);
  void updateDiscovery(SelectDiscoveryData &data);
  void deleteDiscovery(const char *deviceclass, const char *type, const char *field);

  void setupDevices(JsonDocument &json);
  void loopDevices();
  void stopDevices();
  void setDeviceStates(JsonArray &arr);
  void setDeviceSubscriptions();
  void handleDeviceCommand(char *device, char *charPayload);
  void handleDeviceState(char *device, char *charPayload);
};

#endif
