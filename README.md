# ABOUT ESP-NOW RC

This is a library for connecting your two ESP32 devices via ESP-NOW in a manner similar to how standard rc transmitters work. It is designed to be easy to use and have auto-pairing capabilities (no need to know MAC addresses). 

## Capabilities and features

- 8 channel transmission
- ability to send 8 channel telemetry back to transmitter
- very simple syntax
- thanks to ESP-NOW protocol configured to Long Range mode range up to 1km LOS

## Basic operation and usage

Operation can be divided into three steps, configuration, pairing and transmission.

### Configuration

To configure this component first setup the structure 
