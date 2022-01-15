#ifndef _bme680
#define _bme680

#include "common.h"

#include <EEPROM.h>
#include "bsec.h"

#define STATE_SAVE_PERIOD UINT64_C(7 * 24 * 60 * 60 * 1000) // 7 days

class BME680
{
public:
  static const char *type;

  BME680(JsonObject &config);
  ~BME680();

  void loop();
  void command(const char *cmd);
  static void unpublishHomeassistant();

private:
  char *name;
  char topic[MQTT_MAX_TOPIC_LENGTH];

  Bsec iaqSensor;
  uint8_t bsecState[BSEC_MAX_STATE_BLOB_SIZE] = {0};
  uint64_t lastSave = 0;

  void publishHomeassistant();
  void loadState();
  void saveState();
};

#endif
