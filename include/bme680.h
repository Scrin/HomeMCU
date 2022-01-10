#ifndef _bme680
#define _bme680

#include "common.h"

#include <EEPROM.h>
#include "bsec.h"

#define STATE_SAVE_PERIOD UINT32_C(12 * 60 * 60 * 1000) // 12h

class BME680
{
public:
  static const char *type;
  bool enabled = false;

  void setup(JsonObject config);
  void loop();

private:
  char *name;
  char topic[MQTT_MAX_TOPIC_LENGTH];

  Bsec iaqSensor;
  uint8_t bsecState[BSEC_MAX_STATE_BLOB_SIZE] = {0};
  uint16_t stateUpdateCounter = 0;

  void publishHomeassistant();
  void unpublishHomeassistant();
  void loadState();
  void updateState();
};

#endif
