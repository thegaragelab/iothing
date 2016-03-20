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

// Maximum length of SSIDs (32 characters + terminator)
#define MAX_SSID_LENGTH 34

// Maximum length of passwords (64 characters + terminator)
#define MAX_PASSWORD_LENGTH 66

// Maximum length of NodeID (36 character UUID + terminator)
#define MAX_NODEID_LENGTH 40

// Maximum length of MQTT server name
#define MAX_SERVER_NAME_LENGTH 64

// Maximum length of MQTT topic name
#define MAX_TOPIC_NAME_LENGTH 128

#define IOTCONFIG_BLOCK_SIZE 512

#if (MAX_SSID_LENGTH + MAX_PASSWORD_LENGTH + MAX_NODEID_LENGTH + MAX_SERVER_NAME_LENGTH + MAX_TOPIC_NAME_LENGTH + 2) > IOTCONFIG_BLOCK_SIZE
# error "IOTCONFIG_BLOCK_SIZE is too small, adjust upwards"
#endif

/** The block containing our current configuration
 */
typedef struct {
  char     m_szSSID[MAX_SSID_LENGTH];
  char     m_szPass[MAX_PASSWORD_LENGTH];
  char     m_szNode[MAX_NODEID_LENGTH];
  char     m_szMqtt[MAX_SERVER_NAME_LENGTH];
  char     m_szTopic[MAX_TOPIC_NAME_LENGTH];
  uint16_t m_crc16;
  } WIFI_CONFIG;

extern WIFI_CONFIG Config;

typedef enum {
  StateIdle,         // Idle, no connection
  StateConnecting,   // Connecting to a WiFi network
  StateSystemConfig, // WiFi network not configured or not available
  StateConnected,    // Connected to configured WiFi network
  } ConfigState;

// Callback function for state changes
typedef void (*PFN_STATE_CHANGED)(ConfigState previous, ConfigState current);

// Callback function for updates
typedef void (*PFN_UPDATE_APPLIED)();

class IotConfigClass {
  private:
    PFN_STATE_CHANGED  m_pfnCallback;
    PFN_UPDATE_APPLIED m_pfnUpdate;
    ConfigState        m_state;
    int                m_eepromOffset;

  public:
    /** Default constructor
     */
    IotConfigClass();

    /** Set the callback function for state changes
     *
     * @param pfnCallback pointer to the function to receive callback notifications.
     *                    This may be NULL to disable callbacks completely.
     */
    inline void setStateChangeCallback(PFN_STATE_CHANGED pfnCallback) {
      m_pfnCallback = pfnCallback;
      }

    /** Set the callback function for updates
     */
    inline void setUpdateCallback(PFN_UPDATE_APPLIED pfnUpdate) {
      m_pfnUpdate = pfnUpdate;
      }

    /** Trigger the state update callback
     */
    inline void onStateChange(ConfigState state) {
      ConfigState previous = m_state;
      m_state = state;
      if(m_pfnCallback!=NULL)
        (*m_pfnCallback)(previous, state);
      }

    /** Trigger the config changed callback
     */
    inline void onConfigChange() {
      if(m_pfnUpdate!=NULL)
        (*m_pfnUpdate)();
      }

    /** Get the current state
     */
    inline ConfigState state() {
      return m_state;
      }

    /** Set up the library
     *
     * @param settings the system settings to use.
     * @param forceSystem if true the library will go into system configuration
     *                    mode regardless of the WiFi settings.
     */
    bool setup(bool force, int eepromOffset = 0);

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
