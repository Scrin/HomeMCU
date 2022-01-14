#include "homemcu.h"

#include "mhz19.h"
#include "bme680.h"
#include "ledstrip.h"

MHZ19 mhz19;
BME680 bme680;
Ledstrip ledstrip;

/**
 * @brief Set up devices. This is called once a config is received
 * 
 * @param json The device config
 */
void setupDevices(JsonDocument &json)
{
  mhz19.setup(json[MHZ19::type]);
  bme680.setup(json[BME680::type]);
  ledstrip.setup(json[Ledstrip::type]);
}

/**
 * @brief This is called every loop. This will process the device specific loops for enabled devices
 */
void loopDevices()
{
  if (mhz19.enabled)
    mhz19.loop();

  if (bme680.enabled)
    bme680.loop();

  if (ledstrip.enabled)
    ledstrip.loop();
}

/**
 * @brief This is called when the MCU enters a stop state, such as pending a restart. 
 * This is used to do a "graceful stop" to devices, such as saving the bsec state to EEPROM on the BME680
 */
void stopDevices()
{
  if (bme680.enabled)
    bme680.stop();

  if (ledstrip.enabled)
    ledstrip.stop();
}

/**
 * @brief Set the device enabled status for the HomeMCU state
 * 
 * @param json The state object
 */
void setDeviceStates(JsonDocument &json)
{
  json[MHZ19::type] = mhz19.enabled;
  json[BME680::type] = bme680.enabled;
  json[Ledstrip::type] = ledstrip.enabled;
}

/**
 * @brief Handle command directed to a specific device
 * 
 * @param device devicetype this command is intended for
 * @param charPayload the command payload itself
 */
void handleDeviceCommand(char *device, char *charPayload)
{
  if (strcmp(MHZ19::type, device) == 0)
  {
    Log::debug("Got " + String(MHZ19::type) + " command: " + charPayload);
    mhz19.command(charPayload);
  }
  else if (strcmp(BME680::type, device) == 0)
  {
    Log::debug("Got " + String(BME680::type) + " command: " + charPayload);
    bme680.command(charPayload);
  }
  else if (strcmp(Ledstrip::type, device) == 0)
  {
    Log::debug("Got " + String(Ledstrip::type) + " command: " + charPayload);
    ledstrip.command(charPayload);
  }
  else
  {
    Log::warn("Got unknown device command for " + String(device) + ": " + String(charPayload));
  }
}
