# HomeMCU

HomeMCU is a firmware for a ESP8266 based NodeMCU with support for a few different sensors, to be used with Home Assistant. The goal of this firmware is to be a "one binary fits all NodeMCUs around the house" so that updates and adding new sensors and devices around the house requires a minimal amount of work, ie. no need to compile different binaries with different settings for each NodeMCU, instead just flash the same binary to every MCU and set or change the config via MQTT.

#### Supported sensors/devices

- Bosch BME680 (temperature, humidity, air pressure, indoor air quality index, voc, etc)
- MH-Z19 Carbon dioxide sensor

## Setup

- Clone/download this repository
- Copy `include/config.h.example` as `include/config.h`
- Set your configuration in `config.h`
- Build/flash like any other PlatformIO project
- Power on the MCU and wait for a status topic to appear at `HomeMCU/<mac>/status` where `status` is `waiting_for_config`
- Publish a configuration as json (see below for format) to `HomeMCU/<mac>/config` **with the retain flag set**
- The MCU will pick up the config and configure itself accordingly
- Subsequent config changes can be done by simply publishing a new config, the MCU will automatically reboot to the new config
- The MCU will automatically publish MQTT discovery topics (and undiscovery if you disable a sensor) so that no config is needed in Home Assistant (MQTT discovery is enabled by default in HA)

#### Config json

Full example with all sensors and customized entity names for each sensor

```json
{
  "name": "Livingroom",
  "bme680": {
    "enabled": true,
    "name": "Livingroom doorway",
    "static_iaq": false
  },
  "mhz19": {
    "enabled": true,
    "name": "Livingroom ceiling"
  }
}
```

Minimal example with only a MH-Z19 sensor in a bedroom

```json
{
  "name": "Bedroom",
  "mhz19": {
    "enabled": true
  }
}
```

All fields are optional. Field description:

- `name`: Top level name. By default the MAC address of the NodeMCU. You probably want to set this
- `bme680`: Config for the BME680 sensor
  - `enabled`: whether to enable BME680 sensor. false by default.
  - `name`: Override the name for entities based on this sensor. Useful if the sensor is in a different place but connected to a shared MCU. By default the name is the same as the top level name.
  - `static_iaq`: Hint for the Bosch BSEC algorithm whether the sensor is in a static environment, such as in a fixed place indoors. false by default.
- `mhz19`: Config for the MH-Z19 sensor
  - `enabled`: whether to enable MH-Z19 sensor. false by default.
  - `name`: Override the name for entities based on this sensor. Useful if the sensor is in a different place but connected to a shared MCU. By default the name is the same as the top level name.

## MQTT topics

- `HomeMCU/<mac>/status` retained status topic set by the HomeMCU
- `HomeMCU/<mac>/config` retained config topic set by the user for the HomeMCU
- `HomeMCU/<mac>/command` commands to send to the HomeMCU
- `HomeMCU/<mac>/<sensortype>` sensor-specific state topics for the enabled sensors
- `homeassistant/<component>/homemcu_<mac>_<sensortype>_<field>/config` [MQTT Discovery topics](https://www.home-assistant.io/docs/mqtt/discovery/) for Home Assistant entities
- `homeassistant/<component>/homemcu_<mac>_<sensortype>_<field>/attributes` [json_attributes topic](https://www.home-assistant.io/integrations/sensor.mqtt/#json_attributes_topic) for Home Assistant entities

## MQTT commands

These can be sent to the command topic. The commands prefixed with a sensor handler such as "mhz19" or "bme680" are processed only if the corresponding sensor handler is already enabled and setup has finished (check the status topic if you need to be sure). Note that a stopped HomeMCU will not process any other commands except restart.

- `restart` restart the MCU, rarely needed but sometimes useful to re-initialize misbehaving sensors or to restart processing after a stop command
- `stop` stops processing sensors and calls for a graceful halt of the sensors (includes things like saving the bsec state to eeprom for BME680 sensors). You need to restart the MCU to resume normal execution
- `mhz19 calibrate` issue a "zero calibration" command for the MH-Z19 which calibrates the current reading as 400ppm
- `bme680 save` forces immediate saving of the bsec state to EEPROM (normally this happens at stop, restart, and automatically every 7 days)

## Pin assignment

| Device | Device pin | NodeMCU pin label | ESP8266 pin |
| ------ | ---------- | ----------------- | ----------- |
| BME680 | VCC        | 3V                |             |
| BME680 | GND        | G                 |             |
| BME680 | SCL        | D1                | GPIO5       |
| BME680 | SDA        | D2                | GPIO4       |
| MH-Z19 | VIN        | VU or VIN _(1)_   |             |
| MH-Z19 | GND        | G                 |             |
| MH-Z19 | TX         | D7                | GPIO13      |
| MH-Z19 | RX         | D8                | GPIO15      |

_(1)_ VU if you use the usb socket to power the MCU, VIN if you feed external 5V

## Over The Air update

Documentation: https://arduino-esp8266.readthedocs.io/en/latest/ota_updates/readme.html#web-browser

Basic steps:

- Use PlatformIO: Build
- Browse to http://IP_ADDRESS/update
- Select .pio/build/nodemcuv2/firmware.bin from work directory as Firmware and press Update Firmware

## Rough architecture (for developers)

- `main.cpp` will set up basic wifi connectivity and start the OTA update server, and will in turn call `homemcu.cpp`
- `homemcu.cpp` will set up the "application level features" such as the MQTT client
- Each execution loop goes from main to homemcu, which will call the loop of all enabled "sensor handlers"
- Each sensor has it's own "handler", wrapped in a class
- `homemcu.cpp` will listen for configuration changes over MQTT, and when one is received:
  - For each sensor, call the `setup()` of the corresponding sensor handler with the config for that sensor
  - If a configuration was previously loaded, instead reboot the MCU to ensure the new config is applied from a clean state
- Each "sensor handler class" should have and do the following:
  - `static const char *type` which is the lowercase alphanumeric "sensortype", ie. "mhz19" for the MH-Z19 sensor
  - `bool enabled` whether this sensor is enabled and setup has finished
  - `setup()` which will check the passed config and either configure the physical sensor and at the end flip the `enabled` boolean to true or, call `unpublishHomeassistant()`
  - `loop()` which will do whatever operations are required for the sensor. This is called rapidly since some sensors have their own timing constraints so each handler should have its own throttle implementation to adjust the interval of measurements. When updating measurements, the loop function should first publish the state data and then call `publishHomeassistant()`
  - `stop()` optionally, which will do any "controlled stopping", such as writing the current bsec state to EEPROM for BME680
  - `command()` optionally, which will handle any sensor specific commands, such as "mhz19 calibrate" would call "calibrate" command for the MH-Z19 handler
  - `publishHomeassistant()` which will publish the MQTT discovery topics for the data
  - `unpublishHomeassistant()` which will publish an empty message for the same topics as publish, this is used to remove entities from HA when they are removed from the HomeMCU config
- Each sensor should use its own MQTT topic to prevent collisions:
  - Actual measurement data: `HomeMCU/<mac>/<sensortype>`
  - HA discovery: `homeassistant/<component>/homemcu_<mac>_<sensortype>_<field>` (see [this](https://www.home-assistant.io/docs/mqtt/discovery/) for details)
  - `<component>` is one of the supported HA MQTT components, eg. binary_sensor or sensor
  - `<mac>` is the mac address of the mcu wifi
  - `<sensortype>` is the lowercase alphanumeric "type", such as "mhz19" for the MH-Z19 due to HA limitations and for consistency
  - `<field>` is the actual measurement field unique to the sensor, ie `co2` or `temperature`
