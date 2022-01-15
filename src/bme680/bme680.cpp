#include "bme680.h"

const char *BME680::type = "bme680";

const uint8_t bsecConfigDynamic[] = {
#include "config/generic_33v_3s_4d/bsec_iaq.txt"
};

const uint8_t bsecConfigStatic[] = {
#include "config/generic_33v_3s_28d/bsec_iaq.txt"
};

BME680::BME680(JsonObject &config)
{
  if (!config["enabled"])
  {
    // publishHomeassistant();
    Log::info("bme680 disabled");
    return;
  }
  Log::info("bme680 enabled");
  const char *name = config["name"];
  bool staticIaq = config["static_iaq"];
  if (name != nullptr)
  {
    this->name = strdup(name);
  }
  else
  {
    this->name = strdup(HomeMCU::name);
  }
  Utils::getStateTopic(this->topic, BME680::type);

  EEPROM.begin(BSEC_MAX_STATE_BLOB_SIZE + 1); // 1st address for the length
  Wire.begin();
  iaqSensor.begin(BME680_I2C_ADDR_SECONDARY, Wire);

  iaqSensor.setConfig(staticIaq ? bsecConfigStatic : bsecConfigDynamic);

  Log::info("BSEC library version " + String(iaqSensor.version.major) + "." + String(iaqSensor.version.minor) + "." + String(iaqSensor.version.major_bugfix) + "." + String(iaqSensor.version.minor_bugfix));
  Log::info(staticIaq ? "using static iaq" : "using dynamic iaq");

  loadState();

  std::array<bsec_virtual_sensor_t, 12> outputs = {
      BSEC_OUTPUT_RAW_GAS,
      BSEC_OUTPUT_IAQ,
      BSEC_OUTPUT_STATIC_IAQ,
      BSEC_OUTPUT_CO2_EQUIVALENT,
      BSEC_OUTPUT_BREATH_VOC_EQUIVALENT,
      BSEC_OUTPUT_STABILIZATION_STATUS,
      BSEC_OUTPUT_RUN_IN_STATUS,
      BSEC_OUTPUT_RAW_TEMPERATURE,
      BSEC_OUTPUT_RAW_PRESSURE,
      BSEC_OUTPUT_RAW_HUMIDITY,
      BSEC_OUTPUT_SENSOR_HEAT_COMPENSATED_TEMPERATURE,
      BSEC_OUTPUT_SENSOR_HEAT_COMPENSATED_HUMIDITY,
  };

  iaqSensor.updateSubscription(outputs.data(), outputs.size(), BSEC_SAMPLE_RATE_LP);
}

BME680::~BME680()
{
  saveState();
  free(name);
}

void BME680::loop()
{
  if (iaqSensor.run())
  { // If new data is available
    {
      DynamicJsonDocument json(1024);

      json["temperature"] = iaqSensor.temperature;
      json["humidity"] = iaqSensor.humidity;
      json["pressure"] = iaqSensor.pressure;

      json["iaq"] = iaqSensor.iaq;
      json["staticIaq"] = iaqSensor.staticIaq;
      json["co2Equivalent"] = iaqSensor.co2Equivalent;
      json["breathVocEquivalent"] = iaqSensor.breathVocEquivalent;

      json["iaqAccuracy"] = iaqSensor.iaqAccuracy;
      json["staticIaqAccuracy"] = iaqSensor.staticIaqAccuracy;
      json["co2Accuracy"] = iaqSensor.co2Accuracy;
      json["breathVocAccuracy"] = iaqSensor.breathVocAccuracy;

      json["gasResistance"] = iaqSensor.gasResistance;
      json["rawTemperature"] = iaqSensor.rawTemperature;
      json["rawHumidity"] = iaqSensor.rawHumidity;

      json["status"] = iaqSensor.status;
      json["bme680Status"] = iaqSensor.bme680Status;
      json["stabStatus"] = iaqSensor.stabStatus;
      json["runInStatus"] = iaqSensor.runInStatus;

      String msg;
      serializeJson(json, msg);
      HomeMCU::client.publish(topic, msg.c_str(), true);
    }
    publishHomeassistant();
    uint64_t now = Utils::uptime();
    if (lastSave + STATE_SAVE_PERIOD < now)
    {
      lastSave = now;
      saveState();
    }
  }
}

void BME680::command(const char *cmd)
{
  if (strcmp(cmd, "save") == 0)
  {
    Log::info("Saving bsec state");
    saveState();
  }
  else
  {
    Log::error("Unknown " + String(BME680::type) + " command: " + cmd);
  }
}

void BME680::loadState()
{
  if (EEPROM.read(0) == BSEC_MAX_STATE_BLOB_SIZE)
  {
    Log::info("Reading state from EEPROM");

    for (uint8_t i = 0; i < BSEC_MAX_STATE_BLOB_SIZE; i++)
    {
      bsecState[i] = EEPROM.read(i + 1);
    }

    iaqSensor.setState(bsecState);

    char str[BSEC_MAX_STATE_BLOB_SIZE * 2 + 1];
    str[BSEC_MAX_STATE_BLOB_SIZE * 2] = 0;
    for (size_t i = 0; i < BSEC_MAX_STATE_BLOB_SIZE; i++)
      sprintf(&str[2 * i], "%02X", bsecState[i]);
    Log::debug("bsec state loaded: " + String(str));
  }
  else
  {
    Log::info("Erasing EEPROM");

    for (uint8_t i = 0; i < BSEC_MAX_STATE_BLOB_SIZE + 1; i++)
      EEPROM.write(i, 0);

    EEPROM.commit();
  }
}

void BME680::saveState()
{
  iaqSensor.getState(bsecState);

  Log::info("Writing state to EEPROM");

  for (uint8_t i = 0; i < BSEC_MAX_STATE_BLOB_SIZE; i++)
  {
    EEPROM.write(i + 1, bsecState[i]);
  }

  EEPROM.write(0, BSEC_MAX_STATE_BLOB_SIZE);
  EEPROM.commit();

  char str[BSEC_MAX_STATE_BLOB_SIZE * 2 + 1];
  str[BSEC_MAX_STATE_BLOB_SIZE * 2] = 0;
  for (size_t i = 0; i < BSEC_MAX_STATE_BLOB_SIZE; i++)
    sprintf(&str[2 * i], "%02X", bsecState[i]);
  Log::debug("bsec state saved: " + String(str));
}
