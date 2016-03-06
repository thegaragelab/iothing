/*--------------------------------------------------------------------------*
* Implementation of the JSON parser
*---------------------------------------------------------------------------*
* 27-Jan-2016 ShaneG
*
* Initial implementation. Based on simple Json parser code found at
* http://zserge.com/jsmn.html
*--------------------------------------------------------------------------*/
#include "Arduino.h"
#include <stdlib.h>
#include "Json.h"

//---------------------------------------------------------------------------
// Implementation of JsonParser
//---------------------------------------------------------------------------

/** Helper to fill out values of a token structure
 *
 */
static void FillToken(JsonToken *pToken, JsonType type, int start, int end) {
  pToken->type = type;
  pToken->start = start;
  pToken->end = end;
  pToken->size = 0;
  }

/** Allocate a new token from the pool
 */
JsonToken *JsonParser::AllocToken() {
  // Make sure we have one available
  if (m_toknext >= m_tokens)
    return NULL;
  // Set up the new token
  JsonToken *pToken = &m_pTokens[m_toknext++];
  pToken->start = pToken->end = -1;
  pToken->size = 0;
  return pToken;
  }

/** Parse a primitive from the current position in the source
 *
 * Looks for a primitive (boolean, null and numbers) and adds the appropriate
 * token.
 *
 * @return 0 on success, error code on failure
 */
int JsonParser::ParsePrimitive() {
  JsonToken *pToken;
  int start = m_pos;
  for(;m_cszSource[m_pos]!='\0'; m_pos++) {
    switch(m_cszSource[m_pos]) {
      case '\t' : case '\r' : case '\n' : case ' ' :
      case ','  : case ']'  : case '}' :
        goto found;
      }
    if(m_cszSource[m_pos] < 32 || m_cszSource[m_pos] >= 127) {
      m_pos = start;
      return JsonErrorInvalidChar;
      }
    }
  /* In strict mode primitive must be followed by a comma/object/array */
  m_pos = start;
  return JsonErrorPartial;
found:
  pToken = AllocToken();
  if(pToken==NULL) {
    m_pos = start;
    return JsonErrorNoMemory;
    }
  FillToken(pToken, JsonPrimitive, start, m_pos);
  m_pos--;
  return 0;
  }

/** Parse a quoted string from the current position in the source
 *
 * Looks for a quoted string and adds the appropriate token.
 *
 * @return 0 on success, error code on failure
 */
int JsonParser::ParseString() {
  JsonToken *pToken;
  int start = m_pos++;
  /* Skip starting quote */
  for(;m_cszSource[m_pos] != '\0'; m_pos++) {
    char c = m_cszSource[m_pos];
    /* Quote: end of string */
    if(c == '\"') {
      pToken = AllocToken();
      if (pToken==NULL) {
        m_pos = start;
        return JsonErrorNoMemory;
        }
      FillToken(pToken, JsonString, start + 1, m_pos);
      return 0;
      }
      /* Backslash: Quoted symbol expected */
      if(c == '\\' && m_cszSource[m_pos + 1]) {
        int i;
        m_pos++;
        switch(m_cszSource[m_pos]) {
          /* Allowed escaped symbols */
          case '\"': case '/' : case '\\' : case 'b' :
          case 'f' : case 'r' : case 'n'  : case 't' :
            break;
          /* Allows escaped symbol \uXXXX */
          case 'u':
            m_pos++;
            for(i=0; i < 4 && m_cszSource[m_pos]; i++) {
              /* If it isn't a hex character we have an error */
              char h = m_cszSource[m_pos];
              if(!((h >= '0' && h <= '9') || (h >= 'A' && h <= 'F') || (h >= 'a' && h <= 'f'))) {
                m_pos = start;
                return JsonErrorInvalidChar;
                }
              m_pos++;
              }
            m_pos--;
            break;
          /* Unexpected symbol */
          default:
            m_pos = start;
            return JsonErrorInvalidChar;
          }
      }
    }
  m_pos = start;
  return JsonErrorPartial;
  }

/** Initialise the parser with space to keep tokens
 *
 * @param pTokens pointer to an array of JsonToken structures
 * @param tokens the maximum number of tokens that can be stored.
 */
JsonParser::JsonParser(JsonToken *pTokens, int tokens) {
  m_pTokens = pTokens;
  m_tokens = tokens;
  }

/** Parse JSON from a string buffer in memory
 *
 * @param cszJson pointer to a NUL terminated string containing the JSON
 *                to be parsed.
 *
 * @return the number of tokens discovered or a negative value if an error
 *         occurs.
 */
int JsonParser::parse(const char *cszJson) {
  // Set up state
  m_pos = 0;
  m_toknext = 0;
  m_toksuper = -1;
  m_cszSource = cszJson;
  // Do the parsing
  int r, i;
  JsonToken *pToken;
  int count = m_toknext;
  for(; m_cszSource[m_pos] != '\0'; m_pos++) {
    JsonType type;
    char c = m_cszSource[m_pos];
    switch (c) {
      case '{': case '[':
        count++;
        if((pToken = AllocToken())==NULL)
          return JsonErrorNoMemory;
        if(m_toksuper != -1)
          m_pTokens[m_toksuper].size++;
        pToken->type = (c == '{' ? JsonObject : JsonArray);
        pToken->start = m_pos;
        m_toksuper = m_toknext - 1;
        break;
      case '}': case ']':
        type = (c == '}' ? JsonObject : JsonArray);
        for (i = m_toknext - 1; i >= 0; i--) {
          pToken = &m_pTokens[i];
          if (pToken->start != -1 && pToken->end == -1) {
            if (pToken->type != type)
              return JsonErrorInvalidChar;
            m_toksuper = -1;
            pToken->end = m_pos + 1;
            break;
            }
          }
        /* Error if unmatched closing bracket */
        if(i == -1)
          return JsonErrorInvalidChar;
        for (; i >= 0; i--) {
          pToken = &m_pTokens[i];
          if (pToken->start != -1 && pToken->end == -1) {
            m_toksuper = i;
            break;
            }
          }
        break;
      case '\"':
        if((r = ParseString()) < 0)
          return r;
        count++;
        if(m_toksuper != -1)
          m_pTokens[m_toksuper].size++;
        break;
      case '\t' : case '\r' : case '\n' : case ' ':
        break;
      case ':':
        m_toksuper = m_toknext - 1;
        break;
      case ',':
        if(m_toksuper != -1 && m_pTokens[m_toksuper].type != JsonArray && m_pTokens[m_toksuper].type != JsonObject) {
          for (i = m_toknext - 1; i >= 0; i--) {
            if (m_pTokens[i].type == JsonArray || m_pTokens[i].type == JsonObject) {
              if (m_pTokens[i].start != -1 && m_pTokens[i].end == -1) {
                m_toksuper = i;
                break;
                }
              }
            }
          }
        break;
      /* In strict mode primitives are: numbers and booleans */
      case '-': case '0': case '1' : case '2': case '3' : case '4':
      case '5': case '6': case '7' : case '8': case '9':
      case 't': case 'f': case 'n' :
        /* And they must not be keys of the object */
        if (m_toksuper != -1) {
          JsonToken *t = &m_pTokens[m_toksuper];
          if (t->type == JsonObject || (t->type == JsonString && t->size != 0))
            return JsonErrorInvalidChar;
          }
        if((r = ParsePrimitive()) < 0)
          return r;
        count++;
        if (m_toksuper != -1)
          m_pTokens[m_toksuper].size++;
        break;
      /* Unexpected char in strict mode */
      default:
          return JsonErrorInvalidChar;
      }
    }
  for (i = m_toknext - 1; i >= 0; i--) {
    /* Unmatched opened object or array */
    if (m_pTokens[i].start != -1 && m_pTokens[i].end == -1)
      return JsonErrorPartial;
    }
  return count;
  }

/** Find the token representing the content of a named field
 *
 * @param object the token index of the object containing the field.
 * @param cszName the name of the field to find.
 *
 * @return the index of the token representing the data for the field
 *         or -1 if the field does not exist.
 */
int JsonParser::find(int object, const char *cszName) {
  // Make sure we are starting with a valid object token
  if ((object < 0) || (object >= m_tokens) || (m_pTokens[object].type != JsonObject))
    return -1;
  int index = 1, fields = 1;
  int nameLen = strlen(cszName);
  while(fields <= m_pTokens[object].size) {
    if (m_pTokens[object + index].type != JsonString)
      return -1; // Field name must be a string
    int fieldLen = m_pTokens[object + index].end - m_pTokens[object + index].start;
    if((fieldLen == nameLen) && (strncmp(cszName, &m_cszSource[m_pTokens[object + index].start], nameLen) == 0))
      return object + index + 1; // Found it
    // Move to the next field
    fields++;
    index = index + m_pTokens[object + index].size + 1;
    }
  // Not found
  return -1;
  }

/** Get a pointer to the string represented by the token
 */
const char *JsonParser::str(int token) {
  // Make it can be represented as a string
  if ((token < 0) || (token >= m_tokens) || ((m_pTokens[token].type != JsonPrimitive) && (m_pTokens[token].type != JsonString)))
    return NULL;
  // Return the pointer to the start of the string
  return &m_cszSource[m_pTokens[token].start];
  }

/** Get the length to the string represented by the token
 */
int JsonParser::len(int token) {
  // Make it can be represented as a string
  if ((token < 0) || (token >= m_tokens) || ((m_pTokens[token].type != JsonPrimitive) && (m_pTokens[token].type != JsonString)))
    return 0;
  return m_pTokens[token].end - m_pTokens[token].start;
  }
