/*--------------------------------------------------------------------------*
* IoThing Configuration Library
*---------------------------------------------------------------------------*
* 06-Feb-2016 ShaneG
*
* Initial version
*--------------------------------------------------------------------------*/
#include "Arduino.h"
#include "ESP8266WiFi.h"
#include "Settings.h"
#include "IotConfig.h"

//--- Constants
#define WIFI_SSID     "ssid"
#define WIFI_PASSWORD "password"

#define UPDATE_STATUS(pfnCallback, status) \
  if(pfnCallback) { (*pfnCallback)(status); }

//---------------------------------------------------------------------------
// Helper functions
//---------------------------------------------------------------------------

/** Attempt to connect to the access point
 *
 * @param cszSSID the SSID of the AP to connect to
 * @param cszPassword the password to use for the connection
 *
 * @return true if the connection was successful
 */
static bool wifiConnect(const char *cszSSID, const char *cszPassword) {
  return false;
  }

/** Enter access point mode for system configuration
 */
static void wifiAccessPoint() {
  static char szSSID[12];
  sprintf(szSSID, "IoThing %02d", ESP.getChipId() % 100);
  Serial.print("Using AP name ");
  Serial.println(szSSID);
  // Set WiFi to station mode and disconnect from an AP if it was previously connected
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);
  // Make sure the requested SSID is not being used
  bool exists = true;
  while(exists) {
    int n = WiFi.scanNetworks();
    exists = false;
    for (int i=0; i<n; i++) {
      String ssid = WiFi.SSID(i);
      Serial.print("Found SSID ");
      Serial.println(ssid);
      if(strcmp(szSSID, ssid.c_str())==0)
        exists = true;
      }
    if(exists)
      delay(5000); // Wait before scanning again
    }
  // Set up the open AP with the given SSID
  Serial.println("Requested SSID is available, setting up AP");
  WiFi.mode(WIFI_AP);
  WiFi.softAP(szSSID);
  delay(100);
  }

//---------------------------------------------------------------------------
// Public API
//---------------------------------------------------------------------------

/** Default constructor
 */
IotConfigClass::IotConfigClass() {
  m_pfnCallback = NULL;
  }

/** Set the callback function for state changes
 *
 * @param pfnCallback pointer to the function to receive callback notifications.
 *                    This may be NULL to disable callbacks completely.
 */
void IotConfigClass::setCallback(PFN_STATE_CHANGED *pfnCallback) {
  m_pfnCallback = pfnCallback;
  }

/** Set up the library
 *
 * @param settings the system settings to use.
 * @param forceSystem if true the library will go into system configuration
 *                    mode regardless of the WiFi settings.
 */
void IotConfigClass::setup(Settings &settings, bool forceSystem) {
  m_pSettings = &settings;
  // If we are not forcing system config mode try and connect
  if(!forceSystem) {
    UPDATE_STATUS(m_pfnCallback, StateConnecting);
    forceSystem = !wifiConnect(
      m_pSettings->getString(WIFI_SSID, ""),
      m_pSettings->getString(WIFI_PASSWORD, "")
      );
    }
  // Do we need to enter system mode ?
  if(forceSystem) {
    UPDATE_STATUS(m_pfnCallback, StateSystemConfig);
    wifiAccessPoint();
    }
  }

/** Main loop
 *
 * This method must be called from the applications main processing loop
 * to handle incoming configuration changes.
 */
void IotConfigClass::loop() {
  }

// The singleton instance
IotConfigClass IotConfig = IotConfigClass();
