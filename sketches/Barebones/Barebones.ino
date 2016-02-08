#include <ESP8266WiFi.h>
#include <Settings.h>
#include <Json.h>
#include <IotConfig.h>

// Configuration settings
BEGIN_SETTINGS(IoThingSettings)
  SETTING_STRING("ssid", NoModifier, "")
  SETTING_STRING("password", HiddenSetting, "")
  SETTING_STRING("mqtt", NoModifier, "")
  SETTING_STRING("topic", NoModifier, "iothing")
  SETTING_INTEGER("updates", NoModifier, 30)
END_SETTINGS

// Settings buffer
#define SETTINGS_BUFFER 2048
static char pSettings[SETTINGS_BUFFER * 2];
Settings settings = Settings(IoThingSettings, pSettings, SETTINGS_BUFFER * 2);

void setup() {
  Serial.begin(115200);
  IotConfig.setup(settings, true);
  }

void loop() {
  IotConfig.loop();
  }
