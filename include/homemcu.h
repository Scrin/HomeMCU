#ifndef _homemcu
#define _homemcu

#include "common.h"

#include "mhz19.h"
#include "bme680.h"

namespace HomeMCU
{
  // Data for creating Home Assistant MQTT Discovery messages. Optional fields can be omitted with nullptr
  struct DiscoveryData
  {
    const char *component;                // Required. One of the supported HA MQTT components, eg. binary_sensor or sensor
    const char *type;                     // Required. Type of the state, typically type of the sensor such as "mhz19" or "bme680"
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

  extern PubSubClient client;
  extern char *name;
  extern char statusTopic[MQTT_MAX_TOPIC_LENGTH];

  void setup();
  void loop();

  void updateState();
  void mqttCallback(char *topic, byte *payload, unsigned int length);
  void checkConnection();

  void updateDiscovery(DiscoveryData &data);
  void deleteDiscovery(const char *component, const char *type, const char *field);
};

#endif
