# HomeMCU

HomeMCU is a firmware for a ESP8266 based NodeMCU with support for a few different sensors and devices, to be used with Home Assistant. The goal of this firmware is to be a "one binary fits all NodeMCUs around the house" so that updates and adding new sensors and devices around the house requires a minimal amount of work, ie. no need to compile different binaries with different settings for each NodeMCU, instead just flash the same binary to every MCU and set or change the config via MQTT.

#### Supported sensors/devices

- Bosch BME680 (temperature, humidity, air pressure, indoor air quality index, voc, etc)
- MH-Z19 Carbon dioxide sensor
- Addressable RGBW led strip

## Setup

- Clone/download this repository
- Copy `include/config.h.example` as `include/config.h`
- Set your configuration in `config.h`
- Build/flash like any other PlatformIO project
- Power on the MCU and wait for a status topic to appear at `HomeMCU/<mac>/status` where `status` is `waiting_for_config`
- Publish a configuration as json (see below for format) to `HomeMCU/<mac>/config` **with the retain flag set**
- The MCU will pick up the config and configure itself accordingly
- Subsequent config changes can be done by simply publishing a new config, the MCU will automatically reboot to the new config
- The MCU will automatically publish MQTT discovery topics (and undiscovery if you disable a device) so that no config is needed in Home Assistant (MQTT discovery is enabled by default in HA)

#### Config json

Full example with all devices and customized entity names for each device

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
  },
  "ledstrip": {
    "enabled": true,
    "name": "Livingroom bookshelf",
    "led_count": 50
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
- `ledstrip`: Config for the addressable RGBW ledstrip
  - `enabled`: whether to enable the ledstrip. false by default.
  - `name`: Override the name for entities based on this ledstrip. Useful if the ledstrip is in a different place but connected to a shared MCU. By default the name is the same as the top level name.
  - `led_count`: The number of leds in the led strip. You probably want to set this. 100 by default

## MQTT topics

- `HomeMCU/<mac>/status` retained status topic set by the HomeMCU
- `HomeMCU/<mac>/config` retained config topic set by the user for the HomeMCU
- `HomeMCU/<mac>/command` commands to send to the HomeMCU
- `HomeMCU/<mac>/command/<device>` commands to send to the device
- `HomeMCU/<mac>/<device>` device-specific state topic
- `homeassistant/<component>/homemcu_<mac>_<device>_<field>/config` [MQTT Discovery topics](https://www.home-assistant.io/docs/mqtt/discovery/) for Home Assistant entities
- `homeassistant/<component>/homemcu_<mac>_<device>_<field>/attributes` [json_attributes topic](https://www.home-assistant.io/integrations/sensor.mqtt/#json_attributes_topic) for Home Assistant entities

## MQTT commands

These can be sent to the HomeMCU command topic. Note that a stopped HomeMCU will not process any other commands except restart.

- `restart` restart the MCU, rarely needed but sometimes useful to re-initialize misbehaving devices or to restart processing after a stop command
- `stop` stops processing devices and calls for a graceful halt of the devices (includes things like saving the bsec state to eeprom for BME680 sensors). You need to restart the MCU to resume normal execution

These can be sent to the device command topic. Note that these are processed only if the corresponding device handler is already enabled and setup has finished (check the status topic if you need to be sure).

- `mhz19` commands
  - `calibrate` issue a "zero calibration" command for the MH-Z19 which calibrates the current reading as 400ppm
- `bme680` commands
  - `save` forces immediate saving of the bsec state to EEPROM (normally this happens at stop, restart, and automatically every 7 days)
- `ledstrip` commands
  - json, with HA MQTT light commands, including the custom features. For example `{"state":"ON","effect":"sunrise","sunrise_duration":30}` would trigger a 30 minute long sunrise effect for turning on the led strip

## Pin assignment

| Device   | Device pin | NodeMCU pin label | ESP8266 pin |
| -------- | ---------- | ----------------- | ----------- |
| BME680   | VCC        | 3V                |             |
| BME680   | GND        | G                 |             |
| BME680   | SCL        | D1                | GPIO5       |
| BME680   | SDA        | D2                | GPIO4       |
| MH-Z19   | VIN        | VU or VIN _(1)_   |             |
| MH-Z19   | GND        | G                 |             |
| MH-Z19   | TX         | D7                | GPIO13      |
| MH-Z19   | RX         | D8                | GPIO15      |
| Ledstrip | +5V        | [none] _(2)_      |             |
| Ledstrip | Din        | RX                | GPIO3       |
| Ledstrip | Gnd        | G                 |             |

_(1)_ VU if you use the usb socket to power the MCU, VIN if you feed external 5V
_(2)_ Don't feed 5V to the strip through the NodeMCU, the leds will draw too much current for the NodeMCU to handle! Use an external power supply directly.

## Over The Air update

Documentation: https://arduino-esp8266.readthedocs.io/en/latest/ota_updates/readme.html#web-browser

Basic steps:

- Use PlatformIO: Build
- Browse to http://IP_ADDRESS/update
- Select .pio/build/nodemcuv2/firmware.bin from work directory as Firmware and press Update Firmware

## Rough architecture (for developers)

- `main.cpp` will set up basic wifi connectivity and start the OTA update server, and will in turn call `homemcu`
- `homemcu` will set up the "application level features" such as the MQTT client
- Each execution loop goes from main to homemcu, which will call the loop of all enabled "device handlers"
- Each device has it's own "handler", wrapped in a class
- `homemcu` will listen for configuration changes over MQTT, and when one is received:
  - For each device, call the `setup()` of the corresponding device handler with the config for that device
  - If a configuration was previously loaded, instead reboot the MCU to ensure the new config is applied from a clean state
- Each "device handler class" should have and do the following:
  - `static const char *type` which is the lowercase alphanumeric "devicetype", ie. "mhz19" for the MH-Z19 sensor
  - `bool enabled` whether this device is enabled and setup has finished
  - `setup()` which will check the passed config and either configure the physical device and at the end flip the `enabled` boolean to true or, call `publishHomeassistant()` to un-discover the entities from HA
  - `loop()` which will do whatever operations are required for the device. This is called rapidly since some devices have their own timing constraints so each handler should have its own throttle implementation to adjust the interval of measurements. When updating measurements, the loop function should first publish the state data and then call `publishHomeassistant()` if the attributes have changed
  - `stop()` optionally, which will do any "controlled stopping", such as writing the current bsec state to EEPROM for BME680
  - `command()` optionally, which will handle any device specific commands, such "calibrate" command for the MH-Z19 handler
  - `publishHomeassistant()` which will publish the MQTT discovery topics for the data config
- Each device should use its own MQTT topic to prevent collisions:
  - Actual measurement data: `HomeMCU/<mac>/<devicetype>`
  - HA discovery: `homeassistant/<component>/homemcu_<mac>_<devicetype>_<field>` (see [this](https://www.home-assistant.io/docs/mqtt/discovery/) for details)
    - `<component>` is one of the supported HA MQTT components, eg. binary_sensor or sensor
    - `<mac>` is the mac address of the mcu wifi
    - `<devicetype>` is the lowercase alphanumeric "type", such as "mhz19" for the MH-Z19 due to HA limitations and for consistency
    - `<field>` is the actual measurement field unique to the device, ie `co2` or `temperature`
