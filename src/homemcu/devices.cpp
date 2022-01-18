#include "homemcu.h"

#include "mhz19.h"
#include "bme680.h"
#include "ledstrip.h"

MHZ19 *mhz19 = nullptr;
BME680 *bme680 = nullptr;
Ledstrip *ledstrip = nullptr;

/**
 * @brief Set up devices. This is called once a config is received
 * 
 * @param json The device config
 */
void HomeMCU::setupDevices(JsonDocument &json)
{
  {
    JsonObject config = json[MHZ19::type];
    if (config["enabled"])
      mhz19 = new MHZ19(config);
    else
      MHZ19::unpublishHomeassistant();
  }
  {
    JsonObject config = json[BME680::type];
    if (config["enabled"])
      bme680 = new BME680(config);
    else
      BME680::unpublishHomeassistant();
  }
  {
    JsonObject config = json[Ledstrip::type];
    if (config["enabled"])
      ledstrip = new Ledstrip(config);
    else
      Ledstrip::unpublishHomeassistant();
  }
}

/**
 * @brief This is called every loop. This will process the device specific loops for enabled devices
 */
void HomeMCU::loopDevices()
{
  if (mhz19)
    mhz19->loop();

  if (bme680)
    bme680->loop();

  if (ledstrip)
    ledstrip->loop();
}

/**
 * @brief This is called when the MCU enters a stop state, such as pending a restart. 
 */
void HomeMCU::stopDevices()
{
  delete mhz19;
  mhz19 = nullptr;
  delete bme680;
  bme680 = nullptr;
  delete ledstrip;
  ledstrip = nullptr;
}

/**
 * @brief Set the device enabled status for the HomeMCU state
 * 
 * @param json The state object
 */
void HomeMCU::setDeviceStates(JsonArray &arr)
{
  if (mhz19)
    arr.add(MHZ19::type);
  if (bme680)
    arr.add(BME680::type);
  if (ledstrip)
    arr.add(Ledstrip::type);
}

/**
 * @brief Set device-specific subscriptions, such as subscribing to their own state for restoring state after a reboot
 */
void HomeMCU::setDeviceSubscriptions()
{
  if (ledstrip)
  {
    char deviceStateTopic[MQTT_MAX_TOPIC_LENGTH];
    Utils::getStateTopic(deviceStateTopic, Ledstrip::type);
    client.subscribe(deviceStateTopic);
  }
}

/**
 * @brief Handle command directed to a specific device
 * 
 * @param device devicetype this command is intended for
 * @param charPayload the command payload itself
 */
void HomeMCU::handleDeviceCommand(char *device, char *charPayload)
{
  if (strcmp(MHZ19::type, device) == 0 && mhz19)
  {
    Log::debug("Got " + String(MHZ19::type) + " command: " + charPayload);
    mhz19->command(charPayload);
  }
  else if (strcmp(BME680::type, device) == 0 && bme680)
  {
    Log::debug("Got " + String(BME680::type) + " command: " + charPayload);
    bme680->command(charPayload);
  }
  else if (strcmp(Ledstrip::type, device) == 0 && ledstrip)
  {
    Log::debug("Got " + String(Ledstrip::type) + " command: " + charPayload);
    ledstrip->command(charPayload);
  }
}

/**
 * @brief Handle device states, can be used for restoring the state after a reboot
 * 
 * @param device devicetype this state is intended for
 * @param charPayload the state payload itself
 */
void HomeMCU::handleDeviceState(char *device, char *charPayload)
{
  if (strcmp(Ledstrip::type, device) == 0 && ledstrip)
  {
    ledstrip->state(charPayload);
    char deviceStateTopic[MQTT_MAX_TOPIC_LENGTH];
    Utils::getStateTopic(deviceStateTopic, Ledstrip::type);
    client.unsubscribe(deviceStateTopic);
  }
}
