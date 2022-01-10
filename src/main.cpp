#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPUpdateServer.h>

#include "homemcu.h"

ESP8266WebServer httpUpdateServer(80);
ESP8266HTTPUpdateServer httpUpdater;

void setup()
{
  Serial.begin(115200);
  while (!Serial)
    ;
  Serial.print("\nConnecting");
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);

  uint8_t mac[6];
  WiFi.macAddress(mac);
  char name[13] = {0};
  sprintf(name, "homemcu_%02X%02X%02X%02X%02X%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

  WiFi.setSleepMode(WIFI_NONE_SLEEP);
  WiFi.mode(WIFI_STA);
  WiFi.hostname(name);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(100);
    Serial.print(".");
  }
  Serial.printf("\nWifi connected. MAC: %s, IP: %s\n", WiFi.macAddress().c_str(), WiFi.localIP().toString().c_str());

  MDNS.begin(name);
  MDNS.addService("http", "tcp", 80);

  httpUpdater.setup(&httpUpdateServer, OTA_UPDATE_USERNAME, OTA_UPDATE_PASSWORD);
  httpUpdateServer.begin();

  HomeMCU::setup();
}

void loop()
{
  httpUpdateServer.handleClient();
  HomeMCU::loop();
}
