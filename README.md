# prometheus-arduino-dht-exporter

A Prometheus exporter for IoT temperature and humidity measurements, using an Arduino with a Wi-Fi module and a DHT (temperature + humidity) sensor.

## Hardware

Arduino-compatible board: [WEMOS D1 Mini](https://wiki.wemos.cc/products:d1:d1_mini) (ESP8266)

DHT sensor: [Wemos DHT Shield](https://wiki.wemos.cc/products:retired:dht_shield_v1.0.0) (DHT11)

## Requirements

- [Arduino IDE](https://www.arduino.cc/en/Main/Software) (download)
- [esp8266 library for Arduino](https://github.com/esp8266/Arduino#installing-with-boards-manager) (see instructions)
- [Adafruit DHT sensor library](https://github.com/adafruit/DHT-sensor-library) (install using the library manager, including all dependencies)

## Building

1. Copy `config.default.h` to `config.h` and fill inn the details.
1. Set the correct board settings in Arduino IDE:
    - WEMOS D1 Mini uses board "WeMoS D1 R2 & mini", CPU frequency 160MHz and upload speed 821 600.
1. Build and upload in Arduino IDE.
