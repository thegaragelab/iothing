#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <DNSServer.h>
#include <TGL.h>
#include <Json.h>
#include <IotConfig.h>

void setup() {
  Serial.begin(115200);
  IotConfig.setup(false);
  }

void loop() {
  IotConfig.loop();
  }
