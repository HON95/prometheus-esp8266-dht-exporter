#pragma once

// Debug mode is enabled if not zero
#define DEBUG_MODE 0
// DHT sensor type: DHT11, DHT21 or DHT22
#define DHT_TYPE DHT11
// DHT pin
#define DHT_PIN D4
// HTTP server port
#define HTTP_SERVER_PORT 80
// HTTP metrics endpoint
#define HTTP_METRICS_ENDPOINT "/metrics"
// Temperature offset in degrees Celsius
#define TEMPERATURE_CORRECTION_OFFSET 0
// Humidity offset in percent
#define HUMIDITY_CORRECTION_OFFSET 0
// How long to cache the sensor results
#define READ_INTERVAL 2000
// How many times to try to read the sensor before returning an error
#define READ_TRY_COUNT 5
// WiFi SSID (required)
#define WIFI_SSID ""
// WiFi password (required)
#define WIFI_PASSWORD ""
