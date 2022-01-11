#include "homemcu.h"

#include <ESP8266WiFi.h>

char *HomeMCU::name = strdup(WiFi.macAddress().c_str());

WiFiClient espClient;
PubSubClient HomeMCU::client = PubSubClient(espClient);

MHZ19 mhz19;
BME680 bme680;

bool restarting = false;
bool configLoaded = false;
char HomeMCU::statusTopic[MQTT_MAX_TOPIC_LENGTH];
char configTopic[MQTT_MAX_TOPIC_LENGTH];

uint64_t lastStatusUpdateLog = 0;
uint64_t lastStatusUpdateMqtt = 0;

void HomeMCU::setup()
{
  Utils::getStateTopic(statusTopic, "status");
  Utils::getStateTopic(configTopic, "config");

  client.setServer(MQTT_SERVER, MQTT_PORT);
  client.setCallback(mqttCallback);

  checkConnection();
}

void HomeMCU::loop()
{
  if (restarting)
  {
    updateState();
    client.loop();
    delay(100);
    ESP.restart();
    return;
  }

  checkConnection();
  client.loop();

  if (mhz19.enabled)
    mhz19.loop();

  if (bme680.enabled)
    bme680.loop();

  uint64_t now = Utils::uptime();
  if (lastStatusUpdateMqtt + 10 * 1000 < now)
  {
    lastStatusUpdateMqtt = now;
    updateState();
  }
  if (lastStatusUpdateLog + 60 * 60 * 1000 < now)
  {
    lastStatusUpdateLog = now;
    Log::info("Uptime is now " + String(now / 1000.0) + " seconds");
  }
}

void HomeMCU::updateState()
{
  DynamicJsonDocument json(1024);
  if (restarting)
  {
    json["status"] = "restarting";
  }
  else if (!configLoaded)
  {
    json["status"] = "waiting_for_config";
  }
  else
  {
    json["status"] = "online";
  }
  json["ip"] = WiFi.localIP();
  if (configLoaded)
  {
    json[MHZ19::type] = mhz19.enabled;
    json[BME680::type] = bme680.enabled;
  }
  json["uptime"] = Utils::uptime();

  String msg;
  serializeJson(json, msg);
  client.publish(statusTopic, msg.c_str(), true);
}

void HomeMCU::mqttCallback(char *topic, uint8_t *payload, unsigned int length)
{
  Log::info("Message arrived at " + String(topic));

  if (strcmp(topic, configTopic) == 0)
  {
    DynamicJsonDocument json(1024);
    DeserializationError error = deserializeJson(json, (const uint8_t *)payload);
    if (error)
    {
      Log::warn("Config deserialization failed: " + String(error.c_str()));
      return;
    }

    if (configLoaded)
    {
      // config update, restart to re-initialize stuff
      Log::info("New config! restarting...");
      restarting = true;
      return;
    }

    if (json["name"])
    {
      free(name);
      HomeMCU::name = strdup(json["name"]);
    }

    mhz19.setup(json[MHZ19::type]);
    bme680.setup(json[BME680::type]);

    configLoaded = true;
    updateState();
  }
}

void HomeMCU::checkConnection()
{
  if (client.connected())
  {
    return;
  }
  digitalWrite(LED_BUILTIN, LOW);
  int retries = 0;
  while (!client.connected())
  {
    if (retries < 150)
    {
      Log::info("Attempting MQTT connection...");
      if (client.connect(WiFi.macAddress().c_str(), MQTT_USERNAME, MQTT_PASSWORD, statusTopic, 0, true, "{\"status\":\"offline\"}"))
      {
        Log::info("MQTT connected");
        client.subscribe(configTopic);
        updateState();
      }
      else
      {
        Log::error("MQTT connect failed, rc=" + String(client.state()) + " try again in 5 seconds");
        retries++;
        delay(5000);
      }
    }
    else
    {
      ESP.restart();
    }
  }
  digitalWrite(LED_BUILTIN, HIGH);
}
