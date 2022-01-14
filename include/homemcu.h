#ifndef _homemcu
#define _homemcu

#include "common.h"

namespace HomeMCU
{
  // Data for creating Home Assistant MQTT Discovery messages for sensor entities. Optional fields can be omitted with nullptr
  struct SensorDiscoveryData
  {
    bool enabled;                         // Required. If true, the entity will be created in HA, if false, the device will be removed from HA
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
    bool enabled;                         // Required. If true, the entity will be created in HA, if false, the device will be removed from HA
    const char *type;                     // Required. Device type of the state, use the Class::type when setting this
    const char *field;                    // Required. Field name as it appears in the state json
    const char *stateTopic;               // Required. State topic
    const char *name;                     // Required. Human friendly name of device or sensor-specific name
    const char *fieldName;                // Required. Human friendly name of the sensor reading
    const char *deviceClass;              // Optional. One of the supported device classes for the used HA MQTT component
    const char *icon;                     // Optional. Icon for the entity
    const char *colorMode;                // Optional. Sets the supported color mode for a light
    const char *commandTopic;             // Optional. Sets the command topic
    std::map<String, String> &attributes; // Required, but can be empty. Additional attributes for the entity attributes
  };
  // Data for creating Home Assistant MQTT Discovery messages for number input entities. Optional fields can be omitted with nullptr
  struct NumberDiscoveryData
  {
    bool enabled;                         // Required. If true, the entity will be created in HA, if false, the device will be removed from HA
    const char *type;                     // Required. Device type of the state, use the Class::type when setting this
    const char *field;                    // Required. Field name as it appears in the state json
    const char *stateTopic;               // Required. State topic
    const char *name;                     // Required. Human friendly name of device or sensor-specific name
    const char *fieldName;                // Required. Human friendly name of the sensor reading
    int min;                              // Required. Minimum value
    int max;                              // Required. Maximum value
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
    bool enabled;                         // Required. If true, the entity will be created in HA, if false, the device will be removed from HA
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
  void checkConnection();

  void updateDiscovery(SensorDiscoveryData &data);
  void updateDiscovery(LightDiscoveryData &data);
  void updateDiscovery(NumberDiscoveryData &data);
  void updateDiscovery(SelectDiscoveryData &data);
};

#endif
