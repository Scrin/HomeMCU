#include "log.h"

#include <ESP8266WiFi.h>
#include <Syslog.h>
#include <WiFiUdp.h>

#ifdef SYSLOG_SERVER
#ifdef SYSLOG_PORT
#define SYSLOG_ENABLED
#endif
#endif

WiFiUDP udpClient;

Syslog syslog(udpClient, SYSLOG_PROTO_IETF);

char name[21] = {0};

void Log::setup()
{
#ifdef SYSLOG_ENABLED
  uint8_t mac[6];
  WiFi.macAddress(mac);
  snprintf(name, sizeof(name), "homemcu_%02X%02X%02X%02X%02X%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  syslog.server(SYSLOG_SERVER, SYSLOG_PORT);
  syslog.deviceHostname(name);
  syslog.appName("HomeMCU");
  syslog.defaultPriority(LOG_USER);
#endif
}

void doLog(uint16_t pri, String msg)
{
  Serial.print(Utils::uptime() / 1000.0);
  switch (pri)
  {
  case LOG_DEBUG:
    Serial.print(" DEBUG: ");
    break;
  case LOG_INFO:
    Serial.print(" INFO: ");
    break;
  case LOG_WARNING:
    Serial.print(" WARN: ");
    break;
  case LOG_ERR:
    Serial.print(" ERROR: ");
    break;
  }
  Serial.print(msg);
  Serial.println();
#ifdef SYSLOG_ENABLED
  syslog.log(pri, msg);
#endif
}

void Log::debug(String msg)
{
  doLog(LOG_DEBUG, msg);
}
void Log::info(String msg)
{
  doLog(LOG_INFO, msg);
}
void Log::warn(String msg)
{
  doLog(LOG_WARNING, msg);
}
void Log::error(String msg)
{
  doLog(LOG_ERR, msg);
}
