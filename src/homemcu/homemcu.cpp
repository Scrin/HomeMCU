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

void HomeMCU::setup()
{
  utils::getStateTopic(statusTopic, "status");
  utils::getStateTopic(configTopic, "config");

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

  String msg;
  serializeJson(json, msg);
  client.publish(statusTopic, msg.c_str(), true);
}

void HomeMCU::mqttCallback(char *topic, uint8_t *payload, unsigned int length)
{
  Serial.print("Message arrived at ");
  Serial.println(topic);

  if (strcmp(topic, configTopic) == 0)
  {
    DynamicJsonDocument json(1024);
    DeserializationError error = deserializeJson(json, payload);
    if (error)
    {
      Serial.print("config deserialization failed: ");
      Serial.println(error.f_str());
      return;
    }

    if (configLoaded)
    {
      // config update, restart to re-initialize stuff
      Serial.println("New config! restarting...");
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
      Serial.print("Attempting MQTT connection...");
      if (client.connect(WiFi.macAddress().c_str(), MQTT_USERNAME, MQTT_PASSWORD, statusTopic, 0, true, "{\"status\":\"offline\"}"))
      {
        Serial.println("connected");
        client.subscribe(configTopic);
        updateState();
      }
      else
      {
        Serial.print("failed, rc=");
        Serial.print(client.state());
        Serial.println(" try again in 5 seconds");
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
