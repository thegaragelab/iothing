#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <DNSServer.h>
#include <EEPROM.h>
#include <TGL.h>
#include <Json.h>
#include <IotConfig.h>

bool inConfig = true;

void setup() {
  Serial.begin(115200);
  EEPROM.begin(IOTCONFIG_BLOCK_SIZE);
  inConfig = IotConfig.setup(false);
  }

void loop() {
  IotConfig.loop();
  if(inConfig)
    return; // Will restart when config finished
  }
