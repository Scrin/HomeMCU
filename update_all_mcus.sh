#!/bin/bash

[ -z "$3" ] && echo "Usage: $0 <mqtt-host> <ota-user> <ota-password>" && exit 1

MQTT_HOST="$1"
OTA_USER="$2"
OTA_PASS="$3"

FIRMWARE_FILE="$(dirname $0)/.pio/build/nodemcuv2/firmware.bin"

[ ! -f "$FIRMWARE_FILE" ] && echo "Can't find firmware at $FIRMWARE_FILE" && exit 1

command -v mosquitto_pub >/dev/null 2>&1 || { echo >&2 "'mosquitto_pub' is required but not found"; exit 1; }
command -v mosquitto_sub >/dev/null 2>&1 || { echo >&2 "'mosquitto_sub' is required but not found"; exit 1; }
command -v curl >/dev/null 2>&1 || { echo >&2 "'curl' is required but not found"; exit 1; }
command -v jq >/dev/null 2>&1 || { echo >&2 "'jq' is required but not found"; exit 1; }
command -v egrep >/dev/null 2>&1 || { echo >&2 "'egrep' is required but not found"; exit 1; }

function process() {
  IP="$(echo "$MCU_STATUS" | jq -r .ip)"
  MAC="$(echo "$MCU_STATUS" | jq -r .mac)"
  echo "Processing HomeMCU with MAC: $MAC, IP: $IP, sending stop command"
  mosquitto_pub -h "$MQTT_HOST" -t "HomeMCU/$MAC/command" -m stop
  sleep 2
  echo "Uploading firmware..."
  RESULT="$(curl -X POST -u "$OTA_USER:$OTA_PASS" -F "upload=@$FIRMWARE_FILE" http://$IP/update | sed 's/^<META http-equiv="refresh" content="15;URL=\/">//')"
  echo "Done! Response: $RESULT"
}

echo "Connecting to $MQTT_HOST and waiting for messages on HomeMCU/+/status"

while read -r MCU_STATUS; do
    process
done <<< "$(mosquitto_sub -h "$MQTT_HOST" -t HomeMCU/+/status --retained-only | egrep -v '^{"status":"offline"')"

echo "All done!"
