#ifndef _mhz19
#define _mhz19

#include "common.h"
#include <MHZ19_sensor.h>

class MHZ19
{
public:
  static const char *type;

  MHZ19(JsonObject &config);
  ~MHZ19();

  void loop();
  void command(const char *cmd);
  static void unpublishHomeassistant();

private:
  char *name;
  char topic[MQTT_MAX_TOPIC_LENGTH];

  MHZ19_sensor sensor = MHZ19_sensor(13, 15);
  bool sensorReady = false;
  int sensorState = -1;
  uint64_t lastUpdate = 0;

  void publishHomeassistant();
};

#endif
