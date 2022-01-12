#include "mhz19.h"

const char *MHZ19::type = "mhz19";

void MHZ19::setup(JsonObject config)
{
  if (!config["enabled"])
  {
    unpublishHomeassistant();
    Log::info("mhz19 disabled");
    return;
  }
  Log::info("mhz19 enabled");
  const char *name = config["name"];
  if (name != nullptr)
  {
    this->name = strdup(name);
  }
  else
  {
    this->name = HomeMCU::name;
  }
  Utils::getStateTopic(this->topic, MHZ19::type);
  enabled = true;
}

void MHZ19::loop()
{
  uint64_t now = Utils::uptime();
  if (lastUpdate + 10000 > now)
    return;

  lastUpdate = now;

  measurement_t m = sensor.getMeasurement();

  if (!sensorReady)
  {
    if (m.co2_ppm == 410    /* MH-Z19B magic number during warmup */
        || m.co2_ppm == 500 /* MH-Z19C magic number during warmup */
        || m.co2_ppm == 512 /* MH-Z19C magic number on first query */
        || m.co2_ppm == -1) /* Invalid data, happens when the MCU on the sensor is still booting */
    {
      Log::info("CO2 sensor not ready...");
      return;
    }
    else
    {
      sensorReady = true;
    }
  }

  if (m.state == -1)
  {
    Log::error("CO2 sensor in invalid state!");
    return;
  }

  DynamicJsonDocument json(1024);
  json["co2"] = m.co2_ppm;
  json["temperature"] = m.temperature;
  json["state"] = m.state;

  String msg;
  serializeJson(json, msg);
  HomeMCU::client.publish(topic, msg.c_str());
  publishHomeassistant(m.state);
}

void MHZ19::command(const char *cmd)
{
  if (strcmp(cmd, "calibrate") == 0)
  {
    Log::info("Calibrating MH-Z19 level to 400 ppm");
    sensor.calibrateZero();
  }
  else
  {
    Log::error("Unknown " + String(MHZ19::type) + " command: " + cmd);
  }
}
