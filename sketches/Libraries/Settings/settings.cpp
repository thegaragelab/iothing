/*--------------------------------------------------------------------------*
* Implementation of the settings manager
*---------------------------------------------------------------------------*
* 28-Jan-2016 ShaneG
*
* Initial implementation.
*--------------------------------------------------------------------------*/
#include "Arduino.h"
#include <stdlib.h>
#include <string.h>
#include "Settings.h"

//---------------------------------------------------------------------------
// Helper functions
//---------------------------------------------------------------------------

/** Pack an integer value into the buffer
 *
 * @param pBuffer pointer to the buffer to pack into
 * @param index the starting index to write data
 * @param size the size of the buffer
 * @param value the value to store
 *
 * @return the new index after the data has been written or 0 if the
 *         buffer is full.
 */
static int packInteger(unsigned char *pBuffer, int index, int size, int value) {
  if((index + sizeof(int)) >= size)
    return 0;
  *((int *)&pBuffer[index]) = value;
  return index + sizeof(int);
  }

/** Pack an numeric (floating point) value into the buffer
 *
 * @param pBuffer pointer to the buffer to pack into
 * @param index the starting index to write data
 * @param size the size of the buffer
 * @param value the value to store
 *
 * @return the new index after the data has been written or 0 if the
 *         buffer is full.
 */
static int packDouble(unsigned char *pBuffer, int index, int size, double value) {
  if((index + sizeof(double)) >= size)
    return 0;
  *((double *)&pBuffer[index]) = value;
  return index + sizeof(double);
  }

/** Pack a boolean value into the buffer
 *
 * @param pBuffer pointer to the buffer to pack into
 * @param index the starting index to write data
 * @param size the size of the buffer
 * @param value the value to store
 *
 * @return the new index after the data has been written or 0 if the
 *         buffer is full.
 */
static int packBoolean(unsigned char *pBuffer, int index, int size, bool value) {
  if((index + sizeof(bool)) >= size)
    return 0;
  *((bool *)&pBuffer[index]) = value;
  return index + sizeof(bool);
  }

/** Pack a string value into the buffer
 *
 * @param pBuffer pointer to the buffer to pack into
 * @param index the starting index to write data
 * @param size the size of the buffer
 * @param value the value to store
 *
 * @return the new index after the data has been written or 0 if the
 *         buffer is full.
 */
static int packString(unsigned char *pBuffer, int index, int size, const char *value) {
  int len = strlen(value) + 2; // Character + NUL + length byte
  if ((len > 255) || ((index + len) >= size))
    return 0;
  pBuffer[index++] = (unsigned char)len;
  for(int i=0; i<len - 1; i++, index++)
    pBuffer[index] = value[i];
  return index;
  }

/** Pack an entire setting description into the buffer
 */
static int packSetting(unsigned char *pBuffer, int index, int size, SettingDescription &setting) {
  if (index>=size)
    return 0;
  // 1 byte for type
  pBuffer[index++] = setting.typeAndModifier;
  // The name
  if ((index=packString(pBuffer, index, size, setting.name))<=0)
    return index;
  // And the value itself
  switch(setting.typeAndModifier & SETTING_TYPE_MASK) {
    case StringSetting:
      index = packString(pBuffer, index, size, setting.value.string);
      break;
    case IntegerSetting:
      index = packInteger(pBuffer, index, size, setting.value.integer);
      break;
    case BooleanSetting:
      index = packBoolean(pBuffer, index, size, setting.value.boolean);
      break;
    case NumberSetting:
      index = packDouble(pBuffer, index, size, setting.value.number);
      break;
    default:
      index = 0;
    }
  return index;
  }

/** Unpack an integer from the buffer at the given index
 *
 * @param pBuffer the buffer to unpack from
 * @param index the index in the buffer to start at
 * @param size the size of the buffer in bytes
 * @param value the variable to hold the unpacked value
 *
 * @return the new index for the next piece of data or 0 if an error occurs.
 */
static int unpackInteger(unsigned char *pBuffer, int index, int size, int &value) {
  if((index + sizeof(int)) >= size)
    return 0;
  value = *((int *)&pBuffer[index]);
  return index + sizeof(int);
  }

/** Unpack a number from the buffer at the given index
 *
 * @param pBuffer the buffer to unpack from
 * @param index the index in the buffer to start at
 * @param size the size of the buffer in bytes
 * @param value the variable to hold the unpacked value
 *
 * @return the new index for the next piece of data or 0 if an error occurs.
 */
static int unpackDouble(unsigned char *pBuffer, int index, int size, double &value) {
  if((index + sizeof(double)) >= size)
    return 0;
  value = *((double *)&pBuffer[index]);
  return index + sizeof(double);
  }

/** Unpack a boolean from the buffer at the given index
 *
 * @param pBuffer the buffer to unpack from
 * @param index the index in the buffer to start at
 * @param size the size of the buffer in bytes
 * @param value the variable to hold the unpacked value
 *
 * @return the new index for the next piece of data or 0 if an error occurs.
 */
static int unpackBoolean(unsigned char *pBuffer, int index, int size, bool &value) {
  if((index + sizeof(bool)) >= size)
    return 0;
  value = *((bool *)&pBuffer[index]);
  return index + sizeof(bool);
  }

/** Unpack a string from the buffer at the given index
 *
 * @param pBuffer the buffer to unpack from
 * @param index the index in the buffer to start at
 * @param size the size of the buffer in bytes
 * @param ppValue pointer to the string pointer to store
 *
 * @return the new index for the next piece of data or 0 if an error occurs.
 */
static int unpackString(unsigned char *pBuffer, int index, int size, const char **ppValue) {
  if (index>=size)
    return 0;
  // Get the length (Characters + NUL + length byte)
  int len = pBuffer[index];
  if ((index + len) >= size)
    return 0;
  *ppValue = (const char *)&pBuffer[index + 1];
  return index + len;
  }

/** Unpack an entire setting description from the buffer
 *
 * @param pBuffer the buffer to unpack from
 * @param index the index in the buffer to start at
 * @param size the size of the buffer in bytes
 * @param setting the structure to hold the setting value
 *
 * @return the new index for the next piece of data or 0 if an error occurs.
 */
static int unpackSetting(unsigned char *pBuffer, int index, int size, SettingDescription &setting) {
  if ((index + 1)>=size) // Make sure we have the type modifier at least
    return 0;
  // 1 byte for type
  setting.typeAndModifier = pBuffer[index++];
  // The name
  if ((index=unpackString(pBuffer, index, size, &setting.name))<=0)
    return index;
  // And the value itself
  switch(setting.typeAndModifier & SETTING_TYPE_MASK) {
    case StringSetting:
      index = unpackString(pBuffer, index, size, &setting.value.string);
      break;
    case IntegerSetting:
      index = unpackInteger(pBuffer, index, size, setting.value.integer);
      break;
    case BooleanSetting:
      index = unpackBoolean(pBuffer, index, size, setting.value.boolean);
      break;
    case NumberSetting:
      index = unpackDouble(pBuffer, index, size, setting.value.number);
      break;
    default:
      index = 0;
    }
  return index;
  }

/** Find a setting by name
 *
 * @param pBuffer the buffer to search in
 * @param size the size of the buffer in bytes
 * @param cszName the name of the setting to locate
 * @param setting the SettingDescription structure to fill out
 *
 * @return the index into the buffer where the setting was found or -1 if no
 *         setting of that name is present.
 */
static int findSetting(unsigned char *pBuffer, int size, const char *cszName, SettingDescription &setting) {
  int prev = 0, index = 0;
  while ((index = unpackSetting(pBuffer, index, size, setting))>0) {
    if(strcmp(cszName, setting.name)==0)
      return prev;
    prev = index;
    }
  return -1;
  }

/** Clone the primary buffer into the secondary one with a new value
 *
 * @param pActive pointer to the active buffer
 * @param pBackup pointer to the secondary buffer
 * @param size size of the buffers
 * @param setting the new value for the named setting
 *
 * @return the size of the new buffer or 0 if an error occured
 */
static int cloneWithChange(unsigned char *pActive, unsigned char *pBackup, int size, SettingDescription &setting) {
  SettingDescription current;
  int src = 0, dst = 0;
  while ((src = unpackSetting(pActive, src, size, current))!=0) {
    if (strcmp(current.name, setting.name) == 0)
      dst = packSetting(pBackup, dst, size, setting);
    else
      dst = packSetting(pBackup, dst, size, current);
    // Did we have enough room ?
    if (dst==0)
      return 0;
    }
  return dst;
  }

/** Switch the active buffer
 */
static unsigned char *getInactive(unsigned char *p1, unsigned char *p2, unsigned char *pActive) {
  return (pActive == p1) ? p2 : p1;
  }

//---------------------------------------------------------------------------
// Implementation of Settings
//---------------------------------------------------------------------------

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
Settings::Settings(SettingDescription *defaults, void *pBuffer, int size) {
  // Set up state
  m_used = 0;
  m_size = size / 2;
  m_pBuffer1 = (unsigned char *)pBuffer;
  m_pBuffer2 = &m_pBuffer1[m_size];
  m_pActive = m_pBuffer1;
  m_pDefaults = defaults;
  // Start with default values
//  reset(); // Just use defaults
  }

/** Save the current settings to EEPROM
 *
 * @return true on success
 */
bool Settings::save() {
  return false;
  }

/** Load settings from EEPROM
 *
 * This will load the settings from EEPROM (replacing the currently active
 * settings).
 *
 * @return true on success
 */
bool Settings::load() {
  return false;
  }

/** Reset to default values
 *
 * Resets the current settings to default values
 */
bool Settings::reset() {
  int index = 0;
  m_pActive = getInactive(m_pBuffer1, m_pBuffer2, m_pActive);
  for(int i=0; m_pDefaults[i].typeAndModifier != EndOfSettings; i++) {
    index = packSetting(m_pActive, index, m_size, m_pDefaults[i]);
    if(index<=0) {
      m_pActive = getInactive(m_pBuffer1, m_pBuffer2, m_pActive);
      return false;
      }
    }
  m_used = index;
  return true;
  }

/** Get the value of a string setting
 *
 * @param cszName the name of the setting to retrieve
 * @param cszAlternate the alternative value to use on error or missing.
 *
 * @return the current value of the setting or the alternate value if the
 *         setting is not defined or not of the expected type.
 */
const char *Settings::getString(const char *cszName, const char *cszAlternate) {
  SettingDescription setting;
  if (findSetting(m_pActive, m_size, cszName, setting)<0)
    return cszAlternate;
  if ((setting.typeAndModifier & SETTING_TYPE_MASK) != StringSetting)
    return cszAlternate;
  return setting.value.string;
  }

/** Get the value of an integer setting
 *
 * @param cszName the name of the setting to retrieve
 * @param alternate the alternative value to use on error or missing.
 *
 * @return the current value of the setting or the alternate value if the
 *         setting is not defined or not of the expected type.
 */
int Settings::getInteger(const char *cszName, int alternate) {
  SettingDescription setting;
  if (findSetting(m_pActive, m_size, cszName, setting)<0)
    return alternate;
  if ((setting.typeAndModifier & SETTING_TYPE_MASK) != IntegerSetting)
    return alternate;
  return setting.value.integer;
  }

/** Get the value of a numeric (floating point) setting
 *
 * @param cszName the name of the setting to retrieve
 * @param alternate the alternative value to use on error or missing.
 *
 * @return the current value of the setting or the alternate value if the
 *         setting is not defined or not of the expected type.
 */
double Settings::getNumber(const char *cszName, double alternate) {
  SettingDescription setting;
  if (findSetting(m_pActive, m_size, cszName, setting)<0)
    return alternate;
  if ((setting.typeAndModifier & SETTING_TYPE_MASK) != NumberSetting)
    return alternate;
  return setting.value.number;
  }

/** Get the value of a boolean setting
 *
 * @param cszName the name of the setting to retrieve
 * @param alternate the alternative value to use on error or missing.
 *
 * @return the current value of the setting or the alternate value if the
 *         setting is not defined or not of the expected type.
 */
bool Settings::getBoolean(const char *cszName, bool alternate) {
  SettingDescription setting;
  if (findSetting(m_pActive, m_size, cszName, setting)<0)
    return alternate;
  if ((setting.typeAndModifier & SETTING_TYPE_MASK) != BooleanSetting)
    return alternate;
  return setting.value.boolean;
  }

/** Set the value of a string setting
 *
 * @param cszName the name of the setting to modify
 * @param cszValue the new value for the setting
 *
 * @return true on success, false if the setting is of the wrong type
 *         or the buffer is not large enough to contain the new value.
 */
bool Settings::setString(const char *cszName, const char *cszValue) {
  SettingDescription setting;
  if (findSetting(m_pActive, m_size, cszName, setting)<0)
    return false;
  if ((setting.typeAndModifier & SETTING_TYPE_MASK) != StringSetting)
    return false;
  setting.value.string = cszValue;
  // Now clone with the new value
  unsigned char *pBackup = getInactive(m_pBuffer1, m_pBuffer2, m_pActive);
  int result = cloneWithChange(m_pActive, pBackup, m_size, setting);
  if (result>0) {
    m_pActive = pBackup;
    m_used = result;
    return true;
    }
  return false;
  }

/** Set the value of an integer setting
 *
 * @param cszName the name of the setting to modify
 * @param value the new value for the setting
 *
 * @return true on success, false if the setting is of the wrong type
 *         or the buffer is not large enough to contain the new value.
 */
bool Settings::setInteger(const char *cszName, int value) {
  SettingDescription setting;
  if (findSetting(m_pActive, m_size, cszName, setting)<0)
    return false;
  if ((setting.typeAndModifier & SETTING_TYPE_MASK) != IntegerSetting)
    return false;
  setting.value.integer = value;
  // Now clone with the new value
  unsigned char *pBackup = getInactive(m_pBuffer1, m_pBuffer2, m_pActive);
  int result = cloneWithChange(m_pActive, pBackup, m_size, setting);
  if (result>0) {
    m_pActive = pBackup;
    m_used = result;
    return true;
    }
  return false;
  }

/** Set the value of a numeric (floating point) setting
 *
 * @param cszName the name of the setting to modify
 * @param value the new value for the setting
 *
 * @return true on success, false if the setting is of the wrong type
 *         or the buffer is not large enough to contain the new value.
 */
bool Settings::setNumber(const char *cszName, double value) {
  SettingDescription setting;
  if (findSetting(m_pActive, m_size, cszName, setting)<0)
    return false;
  if ((setting.typeAndModifier & SETTING_TYPE_MASK) != NumberSetting)
    return false;
  setting.value.number = value;
  // Now clone with the new value
  unsigned char *pBackup = getInactive(m_pBuffer1, m_pBuffer2, m_pActive);
  int result = cloneWithChange(m_pActive, pBackup, m_size, setting);
  if (result>0) {
    m_pActive = pBackup;
    m_used = result;
    return true;
    }
  return false;
  }

/** Set the value of a boolean setting
 *
 * @param cszName the name of the setting to modify
 * @param value the new value for the setting
 *
 * @return true on success, false if the setting is of the wrong type
 *         or the buffer is not large enough to contain the new value.
 */
bool Settings::setBoolean(const char *cszName, bool value) {
  SettingDescription setting;
  if (findSetting(m_pActive, m_size, cszName, setting)<0)
    return false;
  if ((setting.typeAndModifier & SETTING_TYPE_MASK) != BooleanSetting)
    return false;
  setting.value.boolean = value;
  // Now clone with the new value
  unsigned char *pBackup = getInactive(m_pBuffer1, m_pBuffer2, m_pActive);
  int result = cloneWithChange(m_pActive, pBackup, m_size, setting);
  if (result>0) {
    m_pActive = pBackup;
    m_used = result;
    return true;
    }
  return false;
  }
