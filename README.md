# Prometheus ESP8266 DHT Exporter

[![GitHub release](https://img.shields.io/github/v/release/HON95/prometheus-esp8266-dht-exporter?label=Version)](https://github.com/HON95/prometheus-esp8266-dht-exporter/releases)

An IoT Prometheus exporter for measuring temperature and humidity, using an ESP8266 (Arduino-compatible) with a Wi-Fi module and a DHT (temperature + humidity) sensor.

## Metrics

| Metric | Description | Unit |
| - | - | - |
| `iot_info` | Metadata about the device. | |
| `iot_air_humidity_percent` | Air humidity. | `%` |
| `iot_air_temperature_celsius` | Air temperature. | `°C` |
| `iot_air_heat_index_celsius` | Apparent air temperature, based on temperature and humidity. | `°C` |

## Requirements

### Hardware

- ESP8266-based board (or some other appropriate Arduino-based board).
    - Tested with "Adafruit Feather HUZZAH ESP8266" and "WEMOS D1 Mini".
    - Tested with ESP8266 Mini from Jaycar ([XC3802](https://www.jaycar.com.au/wi-fi-mini-esp8266-main-board/p/XC3802))
- DHT sensor.
    - Tested with Jaycar DHT11 HAT ([XC3856](https://www.jaycar.com.au/duinotech-wi-fi-mini-dht11-temperature-and-humidity-sensor-shield/p/XC3856) w/ [XC3802](https://www.jaycar.com.au/wi-fi-mini-esp8266-main-board/p/XC3802), set `DHT_PIN` to `11`)
    - Tested with a cheap DHT11 from eBay and "Wemos DHT Shield" (using pin 2).
    - DHT11 supports a maximum of 1Hz polling while DHT22 supports a maximum of 2Hz polling.
    - Both DHT11 and DHT22 support both 3V and 5V at 2.5mA max current.

### Software

- [Arduino IDE](https://www.arduino.cc/en/Main/Software)
    - Download and install.
- [esp8266 library for Arduino](https://github.com/esp8266/Arduino#installing-with-boards-manager)
    - See the instructions on the page.
- [DHT sensor library for ESPx](https://github.com/beegee-tokyo/DHTesp)
    - Install using the Arduino library manager.
    - You can also try the Adafruit one, but that one didn't work for me.

## Building

### Hardware

- Adafruit Feather HUZZAH ESP8266
    - Wire the DHT sensor power to the 3.3V and any GND on the ESP and wire the data output to e.g. pin 14 (aka D5).

- Jaycar ESP8266 Mini
    - Set `DHT_PIN` to `11`
    - Set board to `LOLIN(WEMOS) D1 R2 & Mini` in Tools -> Board.

### Software

Using the Arduino IDE.

1. Copy `config.default.h` to `config.h` and fill inn the details.
1. Open `src/src.ino` in the Arduino IDE.
1. Set the correct settings for the board.
    - WEMOS D1 Mini uses board "WeMoS D1 R2 & mini".
    - Adafruit Feather HUZZAH ESP8266 uses "Adafruit Feather HUZZAH ESP8266".
    - Jarcar ESP8266 Mini uses `LOLIN(WEMOS) D1 R2 & Mini`
1. Build and upload using the Arduino IDE.

## Version

See `src/version.h`.

It's set manually since no build tools (or CI) other than the Arduino IDE is used.

## License

GNU General Public License version 3 (GPLv3).
