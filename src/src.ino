#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <WiFiClient.h>
#include <DHTesp.h>

#include "config.h"

enum LogLevel {
    DEBUG,
    INFO,
    ERROR,
};

void setup_dht_sensor();
void setup_wifi();
void setup_http_server();
void handle_http_home_client();
void handle_http_metrics_client();
void read_sensors(boolean force=false);
bool read_sensor(float (*function)(), float *value);
void log(char const *message, LogLevel level=LogLevel::INFO);

DHTesp dht_sensor;
ESP8266WebServer http_server(HTTP_SERVER_PORT);

float humidity, temperature;
char str_humidity[10], str_temperature[10];
uint32_t previous_read_time = 0;

void setup(void) {
    Serial.begin(9600);
    setup_dht_sensor();
    setup_wifi();
    setup_http_server();
}

void setup_dht_sensor() {
    log("Setting up DHT sensor ...");
    dht_sensor.setup(DHT_PIN, DHTesp::DHT_TYPE);
    delay(dht_sensor.getMinimumSamplingPeriod());
    // Test read
    read_sensors(true);
    log("DHT sensor ready.");
}

void setup_wifi() {
    char message[100];
    snprintf(message, 100, "Using Wi-Fi SSID \"%s\".", WIFI_SSID);
    log(message);
    log("Wi-Fi connecting ...");
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    while (WiFi.status() != WL_CONNECTED) {
      log("Wi-Fi waiting ...", LogLevel::DEBUG);
      delay(500);
    }
    const IPAddress &ipaddr = WiFi.localIP();
    log("Wi-Fi connected.");
    snprintf(message, 100, "IPv4 address: %d.%d.%d.%d", ipaddr[0], ipaddr[1], ipaddr[2], ipaddr[3]);
    log(message);
}

void setup_http_server() {
    http_server.on("/", HTTPMethod::HTTP_GET, handle_http_home_client);
    http_server.on(HTTP_METRICS_ENDPOINT, HTTPMethod::HTTP_GET, handle_http_metrics_client);
    http_server.begin();
    log("HTTP server started.");
}

void loop(void) {
    http_server.handleClient();
}

void handle_http_home_client() {
    static char const *response =
        "Prometheus ESP8266 DHT Exporter by HON95.\n"
        "\n"
        "Project: https://github.com/HON95/prometheus-esp8266-dht-exporter\n"
        "\n"
        "Usage: " HTTP_METRICS_ENDPOINT "\n";
    http_server.send(200, "text/plain; charset=utf-8", response);
}

void handle_http_metrics_client() {
    read_sensors();
    char response[100];
    snprintf(response, 100, "Temperature: %s\nHumidity: %s", str_temperature, str_humidity);
    http_server.send(200, "text/plain; charset=utf-8", response);
}

void read_sensors(boolean force) {
    uint32_t min_interval = max(dht_sensor.getMinimumSamplingPeriod(), READ_INTERVAL);
    uint32_t current_time = millis();
    if (!force && current_time - previous_read_time < min_interval) {
        log("Sensors were recently read, will not read again yet.", LogLevel::DEBUG);
        return;
    }
    previous_read_time = current_time;
  
    read_humidity_sensor();
    read_temperature_sensor();
    // TODO float hic = dht.computeHeatIndex(temperature, humidity, false);
}

void read_humidity_sensor() {
    log("Reading humidity sensor ...", LogLevel::DEBUG);
    bool result = read_sensor([] {
          return dht_sensor.getHumidity();
      }, &humidity);
    if (result) {
        humidity += HUMIDITY_CORRECTION_OFFSET;
        snprintf(str_humidity, 10, "%.0f%%", humidity);
    } else {
        snprintf(str_humidity, 10, "ERROR");
        log("Failed to read humidity sensor.", LogLevel::ERROR);
    }
}

void read_temperature_sensor() {
    log("Reading temperature sensor ...", LogLevel::DEBUG);
    bool result = read_sensor([] {
        return dht_sensor.getTemperature();
    }, &temperature);
    if (result) {
        temperature += TEMPERATURE_CORRECTION_OFFSET;
        snprintf(str_temperature, 10, "%.0f\u00B0C", temperature);
    } else {
        snprintf(str_temperature, 10, "ERROR");
        log("Failed to read temperature sensor.", LogLevel::ERROR);
    }
}

bool read_sensor(float (*function)(), float *value) {
    bool success = false;
    for (int i = 0; i < READ_TRY_COUNT; i++) {
        *value = function();
        if (!isnan(*value)) {
            success = true;
            break;
        }
        log("Failed to read sensor.", LogLevel::DEBUG);
    }
    return success;
}

void log(char const *message, LogLevel level) {
    if (DEBUG_MODE == 0 && level == LogLevel::DEBUG) {
        return;
    }
    // Will overflow after a while
    float seconds = millis() / 1000.0;
    char str_level[10];
    switch (level) {
        case DEBUG:
            strcpy(str_level, "DEBUG");
            break;
        case INFO:
            strcpy(str_level, "INFO");
            break;
        case ERROR:
            strcpy(str_level, "ERROR");
            break;
        default:
            break;
    }
    char record[150];
    snprintf(record, 150, "[%10.3f] [%-5s] %s", seconds, str_level, message);
    Serial.println(record);
}
