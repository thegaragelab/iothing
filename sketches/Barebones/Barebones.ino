#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <DNSServer.h>
#include <EEPROM.h>
#include <TGL.h>
#include <Json.h>
#include <IotConfig.h>

bool inConfig = true;

void onStateChange(ConfigState previous, ConfigState current) {
  Serial.print("State change ");
  Serial.print(previous);
  Serial.print(" -> ");
  Serial.print(current);
  Serial.println();
  }

void onConfigUpdate() {
  Serial.println("Configuration updated.");
  }
  
void setup() {
  Serial.begin(115200);
  EEPROM.begin(IOTCONFIG_BLOCK_SIZE);
  IotConfig.setStateChangeCallback(onStateChange);
  IotConfig.setUpdateCallback(onConfigUpdate);
  inConfig = IotConfig.setup(false);
  }

void loop() {
  IotConfig.loop();
  if(inConfig)
    return; // Will restart when config finished
  }
