/*--------------------------------------------------------------------------*
* Implementation of the JSON builder
*---------------------------------------------------------------------------*
* 27-Jan-2016 ShaneG
*
* Initial implementation.
*--------------------------------------------------------------------------*/
#include "Arduino.h"
#include <stdlib.h>
#include <stdio.h>
#include "Json.h"

#define BEGIN_OBJECT '{'
#define END_OBJECT '}'
#define BEGIN_ARRAY '['
#define END_ARRAY ']'
#define SEPARATOR ','
#define QUOTE '"'
#define END_NAME ':'

//---------------------------------------------------------------------------
// Helpers
//---------------------------------------------------------------------------

/** Add an integer value to the output
 *
 * use itoa()
 *
 */
static const char *getInteger(int value) {
  static char intBuff[12];
  int count = snprintf(intBuff, sizeof(intBuff), "%d", value);
  if((count<0)||(count>=sizeof(intBuff)))
    return "0"; // Should be something better
  return intBuff;
  }

/** Add a floating point value to the output
 *
 * Use dtostrf()
 *
 * http://www.nongnu.org/avr-libc/user-manual/group__avr__stdlib.html#ga6c140bdd3b9bd740a1490137317caa44
 */
static const char *getDouble(double value) {
  static char floatBuff[12];
  int count = snprintf(floatBuff, sizeof(floatBuff), "%g", value);
  if((count<0)||(count>=sizeof(floatBuff)))
    return "0"; // Should be something better
  return floatBuff;
  }

/** Trim a separator character from the end of the string
 */
static void trimSeparator(String &buffer) {
  int last = buffer.length() - 1;
  if((last>0)&&(buffer.charAt(last) == SEPARATOR))
    buffer.remove(last);
  }

//---------------------------------------------------------------------------
// Implementation of JsonBuilder
//---------------------------------------------------------------------------

void JsonBuilder::addString(const char *cszString) {
  m_buffer += '"';
  m_buffer += cszString;
  m_buffer += '"';
  }

/** Default constructor
 */
JsonBuilder::JsonBuilder() {
  m_state[0] = BuildBase;
  m_depth = 0;
  // Add the opening brace
  m_buffer += BEGIN_OBJECT;
  }

/** Add a string value to the current object
 *
 * @param cszName the name of the new value
 * @param cszValue the value to associated with the name.
 */
bool JsonBuilder::add(const char *cszName, const char *cszValue) {
  if((m_depth<0)||(m_state[m_depth]==BuildArray))
    return false; // Invalid state
  addString(cszName);
  m_buffer += END_NAME;
  addString(cszValue);
  m_buffer += SEPARATOR;
  return true;
  }

/** Add a boolean value to the current object
 *
 * @param cszName the name of the new value
 * @param value the value to associated with the name.
 *
 * @return true if the value was added, false if the buffer is full.
 */
bool JsonBuilder::add(const char *cszName, bool value) {
  if((m_depth<0)||(m_state[m_depth]==BuildArray))
    return false; // Invalid state
  addString(cszName);
  m_buffer += END_NAME;
  m_buffer += (value ? "true" : "false");
  m_buffer += SEPARATOR;
  return true;
  }

/** Add a integer value to the current object
 *
 * @param cszName the name of the new value
 * @param value the value to associated with the name.
 *
 * @return true if the value was added, false if the buffer is full.
 */
bool JsonBuilder::add(const char *cszName, int value) {
  if((m_depth<0)||(m_state[m_depth]==BuildArray))
    return false; // Invalid state
  addString(cszName);
  m_buffer += END_NAME;
  m_buffer += getInteger(value);
  m_buffer += SEPARATOR;
  return true;
  }

/** Add a floating point value to the current object
 *
 * @param cszName the name of the new value
 * @param value the value to associated with the name.
 *
 * @return true if the value was added, false if the buffer is full.
 */
bool JsonBuilder::add(const char *cszName, double value) {
  if((m_depth<0)||(m_state[m_depth]==BuildArray))
    return false; // Invalid state
  addString(cszName);
  m_buffer += END_NAME;
  m_buffer += getDouble(value);
  m_buffer += SEPARATOR;
  return true;
  }

/** Add a new child object to the current object
 *
 * Adds a new field to the current object that will contain a child
 * object. Future calls to addXXX(name, value) will add fields to the
 * child object.
 *
 * @param cszName the name of the new field.
 *
 * @return true on success, false if the buffer is full or the operation
 *         is not available in the current state.
 */
bool JsonBuilder::beginObject(const char *cszName) {
  if((m_depth<0)||(m_depth>=MAX_DEPTH)||(m_state[m_depth]==BuildArray))
    return false; // Invalid state
  // Start the new object
  m_depth++;
  m_state[m_depth] = BuildObject;
  // Add the name and opening brace
  addString(cszName);
  m_buffer += END_NAME;
  m_buffer += BEGIN_OBJECT;
  return true;
  }

/** Add a new child object to the current list
 *
 * Adds a new field to the current list that will contain a child
 * object. Future calls to addXXX(name, value) will add fields to the
 * child object.
 *
 * @return true on success, false if the buffer is full or the operation
 *         is not available in the current state.
 */
bool JsonBuilder::beginObject() {
  if((m_depth<0)||(m_depth>=MAX_DEPTH)||(m_state[m_depth]!=BuildArray))
    return false; // Invalid state
  // Start the new object
  m_depth++;
  m_state[m_depth] = BuildObject;
  // Add the opening brace
  m_buffer += BEGIN_OBJECT;
  return true;
  }

/** End the current child object
 *
 * @return true on success, false if the buffer is full or the operation
 *         is not available in the current state.
 */
bool JsonBuilder::endObject() {
  if((m_depth<1)||(m_depth>=MAX_DEPTH)||(m_state[m_depth]!=BuildObject))
    return false;
  // Move back to the previous state
  m_depth--;
  // Remove trailing comma if present
  trimSeparator(m_buffer);
  // Add the closing brace and comma
  m_buffer += END_OBJECT;
  m_buffer += SEPARATOR;
  return true;
  }

/** Add a new child array to the current object
 *
 * @param cszName the name of the new field.
 *
 * @return true on success, false if the buffer is full or the operation
 *         is not available in the current state.
 */
bool JsonBuilder::beginArray(const char *cszName) {
  if((m_depth<0)||(m_depth>=MAX_DEPTH)||(m_state[m_depth]==BuildArray))
    return false; // Invalid state
  // Start the new array
  m_depth++;
  m_state[m_depth] = BuildArray;
  // Add the name and opening bracket
  addString(cszName);
  m_buffer += END_NAME;
  m_buffer += BEGIN_ARRAY;
  return true;
  }

/** Add a new child array to the current array
 *
 * @return true on success, false if the buffer is full or the operation
 *         is not available in the current state.
 */
bool JsonBuilder::beginArray() {
  if((m_depth<0)||(m_depth>=MAX_DEPTH)||(m_state[m_depth]!=BuildArray))
    return false; // Invalid state
  // Start the new object
  m_depth++;
  m_state[m_depth] = BuildArray;
  // Add the opening bracket
  m_buffer += BEGIN_ARRAY;
  return true;
  }

/** End the current child array
 *
 * @return true on success, false if the buffer is full or the operation
 *         is not available in the current state.
 */
bool JsonBuilder::endArray() {
  if((m_depth<1)||(m_depth>=MAX_DEPTH)||(m_state[m_depth]!=BuildArray))
    return false;
  // Move back to the previous state
  m_depth--;
  // Remove trailing comma if present
  trimSeparator(m_buffer);
  // Add the closing bracket and comma
  m_buffer += END_ARRAY;
  m_buffer += SEPARATOR;
  return true;
  }

/** Add a new string to the current array
 *
 * @param cszValue the value to add to the array
 *
 * @return true on success, false if the buffer is full or the builder is
 *         not currently building an array.
 */
bool JsonBuilder::add(const char *cszValue) {
  if((m_depth<0)||(m_state[m_depth]!=BuildArray))
    return false; // Invalid state
  addString(cszValue);
  m_buffer += SEPARATOR;
  return true;
  }

/** Add a new boolean value to the current array
 *
 * @param value the value to add to the array
 *
 * @return true on success, false if the buffer is full or the builder is
 *         not currently building an array.
 */
bool JsonBuilder::add(bool value) {
  if((m_depth<0)||(m_state[m_depth]!=BuildArray))
    return false; // Invalid state
  m_buffer += (value ? "true" : "false");
  m_buffer += SEPARATOR;
  return true;
  }

/** Add a new integer value to the current array
 *
 * @param value the value to add to the array
 *
 * @return true on success, false if the buffer is full or the builder is
 *         not currently building an array.
 */
bool JsonBuilder::add(int value) {
  if((m_depth<0)||(m_state[m_depth]!=BuildArray))
    return false; // Invalid state
  m_buffer += getInteger(value);
  m_buffer += SEPARATOR;
  return true;
  }

/** Add a new floating point value to the current array
 *
 * @param value the value to add to the array
 *
 * @return true on success, false if the buffer is full or the builder is
 *         not currently building an array.
 */
bool JsonBuilder::add(double value) {
  if((m_depth<0)||(m_state[m_depth]!=BuildArray))
    return false; // Invalid state
  m_buffer += getDouble(value);
  m_buffer += SEPARATOR;
  return true;
  }

/** Finish building.
 *
 * This closes all current open objects and arrays and terminates the
 * string in the output buffer.
 *
 * @return the number of characters (excluding the NUL terminator) in the
 *         buffer or 0 if the buffer is full.
 */
int JsonBuilder::end() {
  // Make sure we have been initialised
  if (m_depth < 0)
    return -1;
  // Close out any pending blocks
  while(m_depth) {
    Serial.print("Inside: ");
    Serial.println(m_buffer);
    // Remove trailing comma if present
    trimSeparator(m_buffer);
    // Handle each object type
    switch(m_state[m_depth]) {
      case BuildObject:
        m_buffer += END_OBJECT;
        break;
      case BuildArray:
        m_buffer += END_ARRAY;
        break;
      }
    m_depth--;
    }
  // Remove trailing comma if present
  trimSeparator(m_buffer);
  // Add the terminating brace
  m_buffer += END_OBJECT;
  return m_buffer.length();
  }
