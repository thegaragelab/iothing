/*--------------------------------------------------------------------------*
* Simple utilities to extend the Arduino library for the ESP8266
*---------------------------------------------------------------------------*
* 09-Feb-2016 ShaneG
*
* Initial implementation.
*--------------------------------------------------------------------------*/
#ifndef __TGL_H
#define __TGL_H

// Define this to enable debugging features
#define DEBUG

#ifdef DEBUG
#  include <stdio.h>
#  define DMSG(fmt, ...) \
     printf("DEBUG: "), printf(fmt, ## __VA_ARGS__), printf("\n");
#else
#  define DMSG(fmt, ...)
#endif

#define /* __TGL_H */
