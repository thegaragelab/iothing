#!/usr/bin/env python
#----------------------------------------------------------------------------
# 05-Mar-2016 ShaneG
#
# Simple utility to convert a text file into a C character array.
#----------------------------------------------------------------------------
import sys
from os.path import splitext

if __name__ == "__main__":
  if len(sys.argv) <> 2:
    print "You must specify a single filename on the command line."
    exit(1)
  # Read the data file
  with open(sys.argv[1], "r") as input:
    data = input.read()
  # Generate the output
  output = open(splitext(sys.argv[1])[0] + ".cpp", "w")
  output.write("const char CONFIG_PAGE[] PROGMEM = {\n  ")
  count = 0
  for ch in data:
    output.write("0x%02x, " % ord(ch))
    count = count + 1
    if (count == 12):
      count = 0
      output.write("\n  ")
  output.write("0x00\n  };\n")
  output.close()
