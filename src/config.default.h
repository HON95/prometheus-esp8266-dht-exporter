#pragma once

// Debug mode is enabled if not zero
#define DEBUG_MODE 0
// Board name
#define BOARD_NAME "ESP8266"
// DHT sensor name (should be the same)
#define DHT_NAME "DHT11"
// DHT sensor type
#define DHT_TYPE DHT11
// DHT pin
#define DHT_PIN 14
// Temperature offset in degrees Celsius
#define TEMPERATURE_CORRECTION_OFFSET 0
// Humidity offset in percent
#define HUMIDITY_CORRECTION_OFFSET 0
// How long to cache the sensor results, in milliseconds
#define READ_INTERVAL 5000
// How many times to try to read the sensor before returning an error
#define READ_TRY_COUNT 5
// Wi-Fi SSID (required)
#define WIFI_SSID ""
// Wi-Fi password (required)
#define WIFI_PASSWORD ""
// Wi-Fi static IPv4 address enabled or disabled (disable for DHCPv4)
#define WIFI_IPV4_STATIC false
// Wi-Fi static IPv4 address
#define WIFI_IPV4_ADDRESS 192, 168, 1, 15
// Wi-Fi static IPv4 gateway address
#define WIFI_IPV4_GATEWAY 192, 168, 1, 1
// Wi-Fi static IPv4 subnet mask
#define WIFI_IPV4_SUBNET_MASK 255, 255, 255, 0
// Wi-Fi static IPv4 primary DNS server
#define WIFI_IPV4_DNS_1 1, 1, 1, 1
// Wi-Fi static IPv4 secondary DNS server
#define WIFI_IPV4_DNS_2 1, 0, 0, 1
// HTTP server port
#define HTTP_SERVER_PORT 80
// HTTP metrics endpoint
#define HTTP_METRICS_ENDPOINT "/metrics"
