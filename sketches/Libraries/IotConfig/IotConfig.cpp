/*--------------------------------------------------------------------------*
* IoThing Configuration Library
*---------------------------------------------------------------------------*
* 06-Feb-2016 ShaneG
*
* Initial version
*--------------------------------------------------------------------------*/
#include "Arduino.h"
#include "ESP8266WiFi.h"
#include "ESP8266WebServer.h"
#include "DNSServer.h"
#include "IotConfig.h"
#include "TGL.h"

//--- SoftAP configuration
IPAddress apIP(192, 168, 4, 1);
IPAddress netMsk(255, 255, 255, 0);

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
    if(exists) {
      DMSG("AP '%s' is already in use, waiting.", szSSID);
      delay(5000); // Wait before scanning again
      }
    }
  // Set up the open AP with the given SSID
  DMSG("Setting up AP with name '%s'", szSSID);
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(apIP, apIP, netMsk);
  WiFi.softAP(szSSID);
  delay(100);
  }

//---------------------------------------------------------------------------
// Web server
//---------------------------------------------------------------------------

ESP8266WebServer httpServer(80);
DNSServer dnsServer;

void handleNotFound() {
  httpServer.send(404, "text/plain", "Resource not found.");
  }

void handleConfig() {
  int code = 200;
  String response = "";
  if(httpServer.method() == HTTP_GET) {
    // Read configuration
    code = 200;
    response += "Get current config";
    }
  else if(httpServer.method() == HTTP_POST) {
    // Write configuration
    code = 200;
    response += "Write new config";
    }
  httpServer.send(code, "text/plain", response);
  }

void handleDefault() {
  httpServer.send(200, "text/plain", "IoThing Configuration");
  }

void webServer(bool withForm) {
  httpServer.on("/config", handleConfig);
  if(withForm) {
    httpServer.onNotFound(handleDefault);
    /* Setup the DNS server redirecting all the domains to the apIP */
    dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
    dnsServer.start(53, "*", apIP);
    }
  else
    httpServer.onNotFound(handleNotFound);
  httpServer.begin();
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
 * @param force if true the library will go into wifi configuration mode
 *              regardless of the current settings.
 */
void IotConfigClass::setup(bool force) {
  // TODO: Load and verify the stored configuration
  force = true;
  // If we are not forcing config mode try and connect
  if(!force) {
    UPDATE_STATUS(m_pfnCallback, StateConnecting);
    force = !wifiConnect(
      Config.m_szSSID,
      Config.m_szPass
      );
    }
  // Do we need to enter system mode ?
  if(force) {
    UPDATE_STATUS(m_pfnCallback, StateSystemConfig);
    wifiAccessPoint();
    webServer(true);
    }
  else
    webServer(false);
  }

/** Main loop
 *
 * This method must be called from the applications main processing loop
 * to handle incoming configuration changes.
 */
void IotConfigClass::loop() {
  httpServer.handleClient();
  dnsServer.processNextRequest();
  }

// The singleton instance
IotConfigClass IotConfig = IotConfigClass();
