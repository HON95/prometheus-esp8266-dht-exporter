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
- [DHT sensor library for ESPx](https://github.com/beegee-tokyo/DHTesp)
    - Install using the Arduino library manager.
    - The Adafruit one didn't work for the WEMOS D1.

## Building

This uses the Arduino IDE.

1. Copy `config.default.h` to `config.h` and fill inn the details.
1. Set the correct settings for the board.
    - WEMOS D1 Mini uses board "WeMoS D1 R2 & mini".
1. Build and upload using the Arduino IDE.

## Metrics

| Metric | Description | Unit |
| - | - | - |
| iot_humidity_percent | Air humidity. | `%` |
| iot_temperature_celsius | Air temperature. | `°C` |
| iot_heat_index_celsius | Apparent air temperature, based on temperature and humidity. | `°C` |
