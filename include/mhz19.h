#ifndef _mhz19
#define _mhz19

#include "common.h"
#include <MHZ19_sensor.h>

class MHZ19
{
public:
  static const char *type;
  bool enabled = false;

  void setup(JsonObject config);
  void loop();

private:
  char *name;
  char topic[MQTT_MAX_TOPIC_LENGTH];

  MHZ19_sensor sensor = MHZ19_sensor(13, 15);
  bool sensorReady = false;
  unsigned long lastUpdate = 0;

  void publishHomeassistant(int sensorState);
  void unpublishHomeassistant();
};

#endif
