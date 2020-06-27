# prometheus-esp8266-dht-exporter

A Prometheus exporter for IoT temperature and humidity measurements, using an ESP8266 (Arduino-compatible) with a Wi-Fi module and a DHT (temperature + humidity) sensor.

## Hardware

ESP8266 board: [WEMOS D1 Mini](https://wiki.wemos.cc/products:d1:d1_mini) (ESP8266)

DHT sensor: [Wemos DHT Shield](https://wiki.wemos.cc/products:retired:dht_shield_v1.0.0) (DHT11)

## Requirements

- [Arduino IDE](https://www.arduino.cc/en/Main/Software)
    - Download and install.
- [esp8266 library for Arduino](https://github.com/esp8266/Arduino#installing-with-boards-manager)
    - See the instructions on the page.
- [Adafruit DHT sensor library](https://github.com/adafruit/DHT-sensor-library)
    - Install using the library manager, including all dependencies.

## Building

This uses the Arduino IDE.

1. Copy `config.default.h` to `config.h` and fill inn the details.
1. Set the correct settings for the board.
    - WEMOS D1 Mini uses board "WeMoS D1 R2 & mini".
1. Build and upload using the Arduino IDE.
