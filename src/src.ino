#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <WiFiClient.h>
#include <DHT.h>

#include "config.h"

DHT dht_sensor(DHT_PIN, DHT_TYPE);
ESP8266WebServer web_server(80);

float humidity, temperature;
char str_humidity[10], str_temperature[10];
unsigned long previousReadTime = 0;

void setup_wifi();
void setup_dht();
void setup_web();
void read_sensors(boolean force=false);
void read_sensor(const char *name, float (*function)(), const char *str_format, char *str_result, size_t len_result, float &result);
void log(const char *message, boolean error=false);

void setup(void) {
  Serial.begin(9600);
  log("Starting setup ...");
  setup_wifi();
  setup_dht();
  setup_web();
  log("Started setup");
}

void setup_wifi() {
  char message[100];
  snprintf(message, 100, "SSID: %s", WIFI_SSID);
  log(message);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  log("WiFi connecting ...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
  const IPAddress &ipaddr = WiFi.localIP();
  snprintf(message, 100, "IP address: %d.%d.%d.%d", ipaddr[0], ipaddr[1], ipaddr[2], ipaddr[3]);
  log(message);
  log("Started WiFi");
}

void setup_dht() {
  dht_sensor.begin();
  // Initial read
  read_sensors(true);
  log("Started DHT sensor");
}

void setup_web() {
  web_server.on("/", [] {
    read_sensors();
    char response[100];
    snprintf(response, 100, "Temperature: %s\nHumidity: %s", str_temperature, str_humidity);
    web_server.send(200, "text/plain", response);
  });
  web_server.begin();
  log("Started web server");
}

void loop(void) {
  web_server.handleClient();
}

void read_sensors(boolean force) {
  unsigned long currentTime = millis();
  if (!force && currentTime - previousReadTime < READ_INTERVAL) {
    // Use cached values
    return;
  }
  previousReadTime = currentTime;

  // Read temperature as degrees Celsius and force read
  read_sensor("temperature", []() {
    return dht_sensor.readTemperature(false, true) + TEMPERATURE_CORRECTION_OFFSET;
  }, "%.0f\u00B0C", str_temperature, 10, temperature);

  // Read humidity and force read
  read_sensor("humidity", []() {
    return dht_sensor.readHumidity(true) + HUMIDITY_CORRECTION_OFFSET;
  }, "%.0f%%", str_humidity, 10, humidity);
}

void read_sensor(const char *name, float (*function)(), const char *str_format, char *str_result, size_t len_result, float &result) {
  char message[100];
  result = function();
  for (int i = 0; isnan(result) && i < READ_RETRY_COUNT; i++) {
#if DEBUG != 0
    snprintf(message, 100, "Re-reading failed %s ...", name);
    log(message, true);
#endif
    result = function();
  }
  if (!isnan(humidity)) {
    snprintf(str_result, len_result, str_format, result);
  } else {
    strcpy(str_result, "ERROR");
    snprintf(message, 100, "Failed to read %s!", name);
    log(message, true);
  }
}

void log(const char *message, boolean error) {
  float seconds = millis() / 1000.0;
  char level[10];
  if (!error) {
    strcpy(level, "INFO");
  } else {
    strcpy(level, "ERROR");
  }
  char record[150];
  snprintf(record, 150, "[%10.3f] [%-5s] %s", seconds, level, message);
  Serial.println(record);
}
