# ESP-NOW RC

This is a library for connecting your two ESP32 devices via ESP-NOW in a manner similar to how standard rc transmitters work. It is designed to be easy to use and have auto-pairing capabilities (no need to know MAC addresses). 

## Capabilities and features

- 8 channel transmission
- ability to send 8 channel telemetry back to transmitter
- very simple syntax
- thanks to ESP-NOW protocol configured to Long Range mode range up to 1km LOS

## Basic operation and usage

Operation can be divided into three steps: configuration, pairing and transmission.

### Configuration

To configure this component first create a structure of type `erc_config_t`. It currently tells the component if the device is a transmitter or a receiver. You would typically configure a controller as a transmitter and the robot/drone/vehicle as a receiver. Then call function `erc_init` with the config structure as an argument. This will configure the component as well as start all the necessary hardware.

### Pairing

If the ESP32 is in transmitter mode, it will automatically wait for a message from a receiver that is ready to pair. To start sending that message call `erc_rx_start_pairing` on the receiver configured ESP32. This will periodically send a broadcast looking for a free transmitter. If a transmitter is found, they will bind. This will happen automatically and does not need any extra functions. Beware that bind does not persist after reboot.

### Transmission

To transmit and receive the data use functions `erc_send_data` and `erc_receive_data`. They can be used on both receiver and transmitter in order to send control signals to the receiver or back telemetry to the transmitter. Both use the `erc_dataframe_t` as the data carrier. After creating the dataframe and setting the ch0 to ch7 values (all are `int16_t`) send them using the function `erc_send_data`. Function `erc_receive_data` will write the most current received data to the dataframe given in the argument, which can be then used to control a device or display telemetry.