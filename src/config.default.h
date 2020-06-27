#pragma once

// Debug mode is enabled if not zero
#define DEBUG 0
// DHT sensor type: DHT11, DHT21 or DHT22
#define DHT_TYPE DHT11
// DHT pin
#define DHT_PIN D4
// Temperature offset in degrees Celsius
#define TEMPERATURE_CORRECTION_OFFSET 0
// Humidity offset in percent
#define HUMIDITY_CORRECTION_OFFSET 0
// How long to cache the sensor results
#define READ_INTERVAL 2000
// Retry up to this many times to read the snsor correctly before returning before returning error
#define READ_RETRY_COUNT 5

// WiFi SSID (required)
const char *WIFI_SSID = "";
// WiFi password (required)
const char *WIFI_PASSWORD = "";
