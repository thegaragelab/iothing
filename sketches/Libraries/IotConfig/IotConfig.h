/*--------------------------------------------------------------------------*
* IoThing Configuration Library
*---------------------------------------------------------------------------*
* Provides a means of configuring the library from a remote device. Requires
* the Settings and Json libraries.
*
* 06-Feb-2016 ShaneG
*
* Initial version
*--------------------------------------------------------------------------*/
#ifndef __IOTCONFIG_H
#define __IOTCONFIG_H

typedef struct {
  char     m_szSSID[32];
  char     m_szPass[32];
  uint16_t m_crc16;
  } WIFI_CONFIG;

typedef enum {
  StateConnecting,   // Connecting to a WiFi network
  StateSystemConfig, // WiFi network not configured or not available
  StateConnected,    // Connected to configured WiFi network
  } ConfigState;

// Callback function for state changes
typedef void (*PFN_STATE_CHANGED)(ConfigState state);

class IotConfigClass {
  private:
    PFN_STATE_CHANGED *m_pfnCallback;

  public:
    WIFI_CONFIG Config;

    /** Default constructor
     */
    IotConfigClass();

    /** Set the callback function for state changes
     *
     * @param pfnCallback pointer to the function to receive callback notifications.
     *                    This may be NULL to disable callbacks completely.
     */
    void setCallback(PFN_STATE_CHANGED *pfnCallback);

    /** Set up the library
     *
     * @param settings the system settings to use.
     * @param forceSystem if true the library will go into system configuration
     *                    mode regardless of the WiFi settings.
     */
    void setup(bool force);

    /** Main loop
     *
     * This method must be called from the applications main processing loop
     * to handle incoming configuration changes.
     */
    void loop();

  };

// Singleton instance
extern IotConfigClass IotConfig;

#endif /* __IOTCONFIG_H */
