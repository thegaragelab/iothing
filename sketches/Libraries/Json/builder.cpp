/*--------------------------------------------------------------------------*
* Implementation of the JSON builder
*---------------------------------------------------------------------------*
* 27-Jan-2016 ShaneG
*
* Initial implementation.
*--------------------------------------------------------------------------*/
//#include "Arduino.h"
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

/** Add a character to the given buffer
 */
static bool addChar(char *szBuffer, int &index, int size, char ch) {
  if (index>=size)
    return false;
  szBuffer[index++] = ch;
  return true;
  }

/** Add text to the given buffer
 *
 * @return false if the buffer is full
 */
static bool addText(char *szBuffer, int &index, int size, const char *cszText) {
  bool result = true;
  for (int i=0; cszText[i] && result; i++)
    result &= addChar(szBuffer, index, size, cszText[i]);
  return result;
  }

/** Add a quoted string to the output
 */
static bool addString(char *szBuffer, int &index, int size, const char *cszString) {
  bool result = addChar(szBuffer, index, size, QUOTE);
  result &= addText(szBuffer, index, size, cszString);
  result &= addChar(szBuffer, index, size, QUOTE);
  return result;
  }

/** Add an integer value to the output
 *
 * use itoa()
 *
 */
static bool addInteger(char *szBuffer, int &index, int size, int value) {
  static char intBuff[12];
  int count = snprintf(intBuff, sizeof(intBuff), "%d", value);
  if((count<0)||(count>=sizeof(intBuff)))
    return false;
  return addText(szBuffer, index, size, intBuff);
  }

/** Add a floating point value to the output
 *
 * Use dtostrf()
 *
 * http://www.nongnu.org/avr-libc/user-manual/group__avr__stdlib.html#ga6c140bdd3b9bd740a1490137317caa44
 */
static bool addDouble(char *szBuffer, int &index, int size, double value) {
  static char floatBuff[12];
  int count = snprintf(floatBuff, sizeof(floatBuff), "%g", value);
  if((count<0)||(count>=sizeof(floatBuff)))
    return false;
  return addText(szBuffer, index, size, floatBuff);
  }

//---------------------------------------------------------------------------
// Implementation of JsonBuilder
//---------------------------------------------------------------------------

/** Initialise the state
 */
void JsonBuilder::init() {
  m_szBuffer = NULL;
  m_index = 0;
  m_size = 0;
  m_depth = -1;
  }

/** Default constructor
 */
JsonBuilder::JsonBuilder() {
  init();
  }

/** Initialise the builder
 *
 * The builder always creates an 'object' and adds a '{' character to the
 * start of the buffer. Additional values are then added with the
 * appropriate 'addXXX()' methods.
 *
 * @param szBuffer pointer to a buffer to hold the generated JSON
 * @param size the number of characters the buffer can hold.
 */
void JsonBuilder::begin(char *szBuffer, int size) {
  init();
  m_szBuffer = szBuffer;
  m_size = size;
  m_state[0] = BuildBase;
  m_depth = 0;
  // Add the opening brace
  addChar(m_szBuffer, m_index, m_size, BEGIN_OBJECT);
  }

/** Add a string value to the current object
 *
 * @param cszName the name of the new value
 * @param cszValue the value to associated with the name.
 *
 * @return true if the value was added, false if the buffer is full.
 */
bool JsonBuilder::add(const char *cszName, const char *cszValue) {
  if((m_depth<0)||(m_state[m_depth]==BuildArray))
    return false; // Invalid state
  bool result = addString(m_szBuffer, m_index, m_size, cszName);
  result &= addChar(m_szBuffer, m_index, m_size, END_NAME);
  result &= addString(m_szBuffer, m_index, m_size, cszValue);
  result &= addChar(m_szBuffer, m_index, m_size, SEPARATOR);
  return result;
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
  bool result = addString(m_szBuffer, m_index, m_size, cszName);
  result &= addChar(m_szBuffer, m_index, m_size, END_NAME);
  result &= addText(m_szBuffer, m_index, m_size, value ? "true" : "false");
  result &= addChar(m_szBuffer, m_index, m_size, SEPARATOR);
  return result;
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
  bool result = addString(m_szBuffer, m_index, m_size, cszName);
  result &= addChar(m_szBuffer, m_index, m_size, END_NAME);
  result &= addInteger(m_szBuffer, m_index, m_size, value);
  result &= addChar(m_szBuffer, m_index, m_size, SEPARATOR);
  return result;
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
  bool result = addString(m_szBuffer, m_index, m_size, cszName);
  result &= addChar(m_szBuffer, m_index, m_size, END_NAME);
  result &= addDouble(m_szBuffer, m_index, m_size, value);
  result &= addChar(m_szBuffer, m_index, m_size, SEPARATOR);
  return result;
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
  bool ok = true;
  ok &= addString(m_szBuffer, m_index, m_size, cszName);
  ok &= addChar(m_szBuffer, m_index, m_size, END_NAME);
  return ok && addChar(m_szBuffer, m_index, m_size, BEGIN_OBJECT);
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
  return addChar(m_szBuffer, m_index, m_size, BEGIN_OBJECT);
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
  // Remove trailing comma
  if (m_szBuffer[m_index - 1] == SEPARATOR)
    m_index--;
  // Add the closing brace and comma
  return addChar(m_szBuffer, m_index, m_size, END_OBJECT) && addChar(m_szBuffer, m_index, m_size, SEPARATOR);
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
  bool ok = true;
  ok &= addString(m_szBuffer, m_index, m_size, cszName);
  ok &= addChar(m_szBuffer, m_index, m_size, END_NAME);
  return ok && addChar(m_szBuffer, m_index, m_size, BEGIN_ARRAY);
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
  return addChar(m_szBuffer, m_index, m_size, BEGIN_ARRAY);
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
  // Remove trailing comma
  if (m_szBuffer[m_index - 1] == SEPARATOR)
    m_index--;
  // Add the closing bracket and comma
  return addChar(m_szBuffer, m_index, m_size, END_ARRAY) && addChar(m_szBuffer, m_index, m_size, SEPARATOR);
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
  return addString(m_szBuffer, m_index, m_size, cszValue) && addChar(m_szBuffer, m_index, m_size, SEPARATOR);
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
  return addText(m_szBuffer, m_index, m_size, value ? "true" : "false") && addChar(m_szBuffer, m_index, m_size, SEPARATOR);
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
  return addInteger(m_szBuffer, m_index, m_size, value) && addChar(m_szBuffer, m_index, m_size, SEPARATOR);
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
  return addDouble(m_szBuffer, m_index, m_size, value) && addChar(m_szBuffer, m_index, m_size, SEPARATOR);
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
  bool ok = true;
  // Make sure we have been initialised
  if (m_depth < 0)
    return false;
  // Close out any pending blocks
  while(m_depth) {
    if (m_szBuffer[m_index - 1] == SEPARATOR)
      m_index--; // Remove trailing comma
    switch(m_state[m_depth]) {
      case BuildObject:
        ok = addChar(m_szBuffer, m_index, m_size, END_OBJECT);
        break;
      case BuildArray:
        ok = addChar(m_szBuffer, m_index, m_size, END_ARRAY);
        break;
      }
    m_depth--;
    }
  if (m_szBuffer[m_index - 1] == SEPARATOR)
    m_index--; // Remove trailing comma
  // Add the terminating brace
  ok = addChar(m_szBuffer, m_index, m_size, END_OBJECT);
  if(ok)
    m_szBuffer[m_index] = '\0';
  return ok ? m_index : 0;
  }
