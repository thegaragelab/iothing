/*--------------------------------------------------------------------------*
* Persistent settings management for microcontrollers.
*---------------------------------------------------------------------------*
* 28-Jan-2016 ShaneG
*
* Initial implementation.
*--------------------------------------------------------------------------*/
#ifndef __SETTINGS_H
#define __SETTINGS_H

/** Data types available for settings
 */
typedef enum {
  EndOfSettings  = 0,
  StringSetting  = 1,
  IntegerSetting = 2,
  BooleanSetting = 3,
  NumberSetting  = 4
  } SettingType;

// Mask to extract setting type
#define SETTING_TYPE_MASK 0x0f

/** Access modifiers for settings
 */
typedef enum {
  NoModifier      = 0,    // Use for no modifications
  HiddenSetting   = 0x80, // Setting is not visible in description
  ReadOnlySetting = 0x40, // Setting cannot be written to
  } SettingModifier;

// Mask to extract setting modifiers
#define SETTING_MODIFIER_MASK 0x80

// Maximum length of a setting name
#define MAX_SETTING_NAME_LENGTH 32

/** Represents the value for a setting
 */
typedef union {
  const char *string;
  int         integer;
  bool        boolean;
  double      number;
  } SettingValue;

/** Description of a single setting
 */
typedef struct {
  const char   *name;             // The name of this setting
  unsigned char typeAndModifier;  // Type and modifier byte
  SettingValue  value;            // The value of this setting
  } SettingDescription;

//---------------------------------------------------------------------------
// Macros to build setting description table
//---------------------------------------------------------------------------

#define BEGIN_SETTINGS(name) \
  SettingDescription name[] = {

#define SETTING_STRING(name, flags, initval) \
  { name, StringSetting | flags, { string: initval } },

#define SETTING_INTEGER(name, flags, initval) \
  { name, IntegerSetting | flags, { integer: initval } },

#define SETTING_NUMBER(name, flags, initval) \
  { name, NumberSetting | flags, { number: initval } },

#define SETTING_BOOLEAN(name, flags, initval) \
  { name, BooleanSetting | flags, { boolean: initval } },

#define END_SETTINGS \
  { "", EndOfSettings } \
  };

//---------------------------------------------------------------------------
// Settings management classes
//---------------------------------------------------------------------------

/** Manage a settings table
 */
class Settings {
  private:
    SettingDescription *m_pDefaults; // The default setting values
    unsigned char      *m_pBuffer1; // The first half of the buffer
    unsigned char      *m_pBuffer2; // The second half of the buffer
    unsigned char      *m_pActive;  // Pointer to the active buffer
    int                 m_size;     // Size of each of the buffer halves
    int                 m_used;     // Number of bytes used

  public:
    /** Initialise the settings.
     *
     * This method prepares the internal settings buffers, reads any valid
     * settings from EEPROM and populates missing values with the provided
     * defaults.
     *
     * @param defaults the settings description table containing default
     *                 values.
     * @param pBuffer a pointer to the buffer to use to contain the active
     *                settings.
     * @param size the size (in bytes) of the buffer. Only half the buffer is
     *             used to store the active settings, the other half is used
     *             for applying modifications. The amount of data stored in
     *             the EEPROM will be size / 2 bytes.
     */
    Settings(SettingDescription *defaults, void *pBuffer, int size);

    /** Save the current settings to EEPROM
     *
     * @return true on success
     */
    bool save();

    /** Load settings from EEPROM
     *
     * This will load the settings from EEPROM (replacing the currently active
     * settings).
     *
     * @return true on success
     */
    bool load();

    /** Reset to default values
     *
     * Resets the current settings to default values
     */
    bool reset();

    /** Get the value of a string setting
     *
     * @param cszName the name of the setting to retrieve
     * @param cszAlternate the alternative value to use on error or missing.
     *
     * @return the current value of the setting or the alternate value if the
     *         setting is not defined or not of the expected type.
     */
    const char *getString(const char *cszName, const char *cszAlternate);

    /** Get the value of an integer setting
     *
     * @param cszName the name of the setting to retrieve
     * @param alternate the alternative value to use on error or missing.
     *
     * @return the current value of the setting or the alternate value if the
     *         setting is not defined or not of the expected type.
     */
    int getInteger(const char *cszName, int alternate);

    /** Get the value of a numeric (floating point) setting
     *
     * @param cszName the name of the setting to retrieve
     * @param alternate the alternative value to use on error or missing.
     *
     * @return the current value of the setting or the alternate value if the
     *         setting is not defined or not of the expected type.
     */
    double getNumber(const char *cszName, double alternate);

    /** Get the value of a boolean setting
     *
     * @param cszName the name of the setting to retrieve
     * @param alternate the alternative value to use on error or missing.
     *
     * @return the current value of the setting or the alternate value if the
     *         setting is not defined or not of the expected type.
     */
    bool getBoolean(const char *cszName, bool alternate);

    /** Set the value of a string setting
     *
     * @param cszName the name of the setting to modify
     * @param cszValue the new value for the setting
     *
     * @return true on success, false if the setting is of the wrong type
     *         or the buffer is not large enough to contain the new value.
     */
    bool setString(const char *cszName, const char *cszValue);

    /** Set the value of an integer setting
     *
     * @param cszName the name of the setting to modify
     * @param value the new value for the setting
     *
     * @return true on success, false if the setting is of the wrong type
     *         or the buffer is not large enough to contain the new value.
     */
    bool setInteger(const char *cszName, int value);

    /** Set the value of a numeric (floating point) setting
     *
     * @param cszName the name of the setting to modify
     * @param value the new value for the setting
     *
     * @return true on success, false if the setting is of the wrong type
     *         or the buffer is not large enough to contain the new value.
     */
    bool setNumber(const char *cszName, double value);

    /** Set the value of a boolean setting
     *
     * @param cszName the name of the setting to modify
     * @param value the new value for the setting
     *
     * @return true on success, false if the setting is of the wrong type
     *         or the buffer is not large enough to contain the new value.
     */
    bool setBoolean(const char *cszName, bool value);
  };

#endif /* __SETTINGS_H */
