#include "homemcu.h"

#include <ESP8266WiFi.h>
#include <CRC32.h>

char *HomeMCU::name = strdup(WiFi.macAddress().c_str());

WiFiClient espClient;
PubSubClient HomeMCU::client = PubSubClient(espClient);

bool stopped = false;
bool stopping = false;
bool restarting = false;
bool configLoaded = false;
uint32_t HomeMCU::currentConfigChecksum = 0;
char HomeMCU::statusTopic[MQTT_MAX_TOPIC_LENGTH];
char configTopic[MQTT_MAX_TOPIC_LENGTH];
char commandTopic[MQTT_MAX_TOPIC_LENGTH];
char deviceCommandTopicPrefix[MQTT_MAX_TOPIC_LENGTH];
char deviceStateTopicPrefix[MQTT_MAX_TOPIC_LENGTH];

uint64_t lastStatusUpdateLog = 0;
uint64_t lastStatusUpdateMqtt = 0;

void HomeMCU::setup()
{
  Utils::getStateTopic(statusTopic, "status");
  Utils::getStateTopic(configTopic, "config");
  Utils::getStateTopic(commandTopic, "command");
  Utils::getStateTopic(deviceCommandTopicPrefix, "command/");
  Utils::getStateTopic(deviceStateTopicPrefix, "");

  client.setServer(MQTT_SERVER, MQTT_PORT);
  client.setCallback(mqttCallback);

  checkConnection();
}

void HomeMCU::loop()
{
  if (restarting)
  {
    updateState(); // update status to "restarting"
    stopDevices();
    client.loop(); // process the outbound mqtt message(s) immediately
    delay(100);    // wait for the underlying network stack to actually send the data
    ESP.restart();
    return;
  }
  else if (stopping)
  {
    updateState(); // update status to "stopping"
    client.loop(); // process the outbound mqtt message(s) immediately
    stopDevices();
    stopping = false;
    stopped = true;
    updateState(); // update status to "stopped"
    return;
  }

  checkConnection();
  client.loop();

  if (!stopped && configLoaded)
  {
    loopDevices();
  }

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
  else if (stopped)
  {
    json["status"] = "stopped";
  }
  else if (stopping)
  {
    json["status"] = "stopping";
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
  json["mac"] = WiFi.macAddress();
  json["build_ts"] = BUILD_TIMESTAMP;
  json["uptime"] = Utils::uptime();
  if (configLoaded)
  {
    json["config_checksum"] = currentConfigChecksum;
    JsonArray devices = json.createNestedArray("devices");
    setDeviceStates(devices);
  }

  String msg;
  serializeJson(json, msg);
  client.publish(statusTopic, msg.c_str(), true);
}

void HomeMCU::mqttCallback(char *topic, uint8_t *payload, unsigned int length)
{
  if (strcmp(topic, configTopic) == 0)
  {
    DynamicJsonDocument json(1024);
    DeserializationError error = deserializeJson(json, (const uint8_t *)payload);
    if (error)
    {
      Log::warn("Config deserialization failed: " + String(error.c_str()));
      return;
    }
    uint32_t checksum = CRC32::calculate(payload, length);

    if (configLoaded)
    {
      if (checksum != currentConfigChecksum)
      {
        // config update, restart to re-initialize stuff
        Log::info("New config! restarting...");
        restarting = true;
      }
      else
      {
        Log::info("Got config message, but it's identical to the current config. Assuming it's a duplicate");
      }
      return;
    }

    if (json["name"])
    {
      free(name);
      HomeMCU::name = strdup(json["name"]);
    }

    setupDevices(json);

    configLoaded = true;
    currentConfigChecksum = checksum;
    setSubscriptions();
    updateState();
  }
  else if (strcmp(topic, commandTopic) == 0)
  {
    if (length > MQTT_MAX_PACKET_SIZE)
    {
      Log::error("Command length exceeded the maximum");
      return;
    }
    char charPayload[MQTT_MAX_PACKET_SIZE + 1];
    unsigned int len = min(static_cast<unsigned int>(sizeof(charPayload)), length);
    memcpy(charPayload, payload, len);
    charPayload[len] = '\0';

    if (strcmp(charPayload, "restart") == 0)
    {
      Log::info("Got restart command");
      restarting = true;
    }
    else if (stopping || stopped)
    {
      // noop, don't process any other commands if the mcu is stopping or already stopped
    }
    else if (strcmp(charPayload, "stop") == 0)
    {
      Log::info("Got stop command");
      stopping = true;
    }
    else
    {
      Log::warn("Got unknown command: " + String(charPayload));
    }
  }
  else if (stopping || stopped)
  {
    // noop, don't process any other messages if the mcu is stopping or already stopped
  }
  else if (strncmp(deviceCommandTopicPrefix, topic, strlen(deviceCommandTopicPrefix)) == 0)
  {
    if (length > MQTT_MAX_PACKET_SIZE)
    {
      Log::error("Command length exceeded the maximum");
      return;
    }
    char charPayload[MQTT_MAX_PACKET_SIZE + 1];
    unsigned int len = min(static_cast<unsigned int>(sizeof(charPayload)), length);
    memcpy(charPayload, payload, len);
    charPayload[len] = '\0';
    handleDeviceCommand(&topic[strlen(deviceCommandTopicPrefix)], charPayload);
  }
  else if (strncmp(deviceStateTopicPrefix, topic, strlen(deviceStateTopicPrefix)) == 0)
  {
    if (length > MQTT_MAX_PACKET_SIZE)
    {
      Log::error("State length exceeded the maximum");
      return;
    }
    char charPayload[MQTT_MAX_PACKET_SIZE + 1];
    unsigned int len = min(static_cast<unsigned int>(sizeof(charPayload)), length);
    memcpy(charPayload, payload, len);
    charPayload[len] = '\0';
    handleDeviceState(&topic[strlen(deviceStateTopicPrefix)], charPayload);
  }
}

void HomeMCU::setSubscriptions()
{
  if (!configLoaded) // MQTT connected but no config loaded yet, subscribe to config and main command topic
  {
    client.subscribe(configTopic);
    client.subscribe(commandTopic);
  }
  else // Config loaded and devices set up, subscribe to device specific topics
  {
    char deviceCommandTopic[MQTT_MAX_TOPIC_LENGTH];
    Utils::getStateTopic(deviceCommandTopic, "command/+");
    client.subscribe(deviceCommandTopic);

    setDeviceSubscriptions();
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
        setSubscriptions();
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
