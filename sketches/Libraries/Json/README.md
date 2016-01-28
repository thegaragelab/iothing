# JSON Manipulation

This library provides classes to parse and generate JSON formatted data. The libary is optimised for small JSON blocks sent over WebSockets or MQTT transports, generally less than 4K in size.

## Builder

The builder class allows you to build a JSON string in a memory buffer prior to sending it over the network.

## Parser

The parser is based on Jasmine (jsmn - http://zserge.com/jsmn.html) and converts JSON strings into an array of tokens that can then be processed using a state machine. The Jasmine example code [found here](http://alisdair.mcdiarmid.org/jsmn-example/) provides a template for how this works.


