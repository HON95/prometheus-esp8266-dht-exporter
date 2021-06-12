#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <WiFiClient.h>
#include <DHTesp.h>

#include "config.h"
#include "version.h"

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

float humidity, temperature, heat_index;
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
    char message[128];
    snprintf(message, 128, "Wi-Fi SSID: %s", WIFI_SSID);
    log(message);

    if (WIFI_IPV4_STATIC) {
        IPAddress address(WIFI_IPV4_ADDRESS);
        IPAddress subnet(WIFI_IPV4_SUBNET_MASK);
        IPAddress gateway(WIFI_IPV4_GATEWAY);
        IPAddress dns1(WIFI_IPV4_DNS_1);
        IPAddress dns2(WIFI_IPV4_DNS_2);
        snprintf(message, 128, "Static IPv4 address: %d.%d.%d.%d", address[0], address[1], address[2], address[3]);
        log(message);
        snprintf(message, 128, "Static IPv4 subnet mask: %d.%d.%d.%d", subnet[0], subnet[1], subnet[2], subnet[3]);
        log(message);
        snprintf(message, 128, "Static IPv4 gateway: %d.%d.%d.%d", gateway[0], gateway[1], gateway[2], gateway[3]);
        log(message);
        snprintf(message, 128, "Static IPv4 primary DNS server: %d.%d.%d.%d", dns1[0], dns1[1], dns1[2], dns1[3]);
        log(message);
        snprintf(message, 128, "Static IPv4 secondary DNS server: %d.%d.%d.%d", dns2[0], dns2[1], dns2[2], dns2[3]);
        log(message);
        if (!WiFi.config(address, gateway, subnet, dns1, dns2)) {
            log("Failed to configure Wi-Fi.", LogLevel::ERROR);
        }
    }

    log("Wi-Fi connecting ...");
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    while (WiFi.status() != WL_CONNECTED) {
        log("Wi-Fi waiting ...", LogLevel::DEBUG);
        delay(500);
    }
    const IPAddress &address = WiFi.localIP();
    log("Wi-Fi connected.");
    snprintf(message, 128, "IPv4 address: %d.%d.%d.%d", address[0], address[1], address[2], address[3]);
    log(message);
}

void setup_http_server() {
    http_server.on("/", HTTPMethod::HTTP_GET, handle_http_home_client);
    http_server.on(HTTP_METRICS_ENDPOINT, HTTPMethod::HTTP_GET, handle_http_metrics_client);
    http_server.begin();
    log("HTTP server started.");
    char message[128];
    snprintf(message, 128, "Metrics endpoint: %s", HTTP_METRICS_ENDPOINT);
    log(message);
}

void loop(void) {
    http_server.handleClient();
}

void handle_http_home_client() {
    static size_t const BUFSIZE = 256;
    static char const *response_template =
        "Prometheus ESP8266 DHT Exporter by HON95.\n"
        "\n"
        "Project: https://github.com/HON95/prometheus-esp8266-dht-exporter\n"
        "\n"
        "Usage: %s\n";
    char response[BUFSIZE];
    snprintf(response, BUFSIZE, response_template, HTTP_METRICS_ENDPOINT);
    http_server.send(200, "text/plain; charset=utf-8", response);
}

void handle_http_metrics_client() {
    static size_t const BUFSIZE = 1024;
    static char const *response_template =
        "# HELP iot_info Metadata about the device.\n"
        "# TYPE iot_info gauge\n"
        "# UNIT iot_info \n"
        "iot_info{version=\"%s\",board=\"%s\",sensor=\"%s\"} 1\n"
        "# HELP iot_air_humidity_percent Air humidity.\n"
        "# TYPE iot_air_humidity_percent gauge\n"
        "# UNIT iot_air_humidity_percent %%\n"
        "iot_air_humidity_percent %f\n"
        "# HELP iot_air_temperature_celsius Air temperature.\n"
        "# TYPE iot_air_temperature_celsius gauge\n"
        "# UNIT iot_air_temperature_celsius \u00B0C\n"
        "iot_air_temperature_celsius %f\n"
        "# HELP iot_air_heat_index_celsius Apparent air temperature, based on temperature and humidity.\n"
        "# TYPE iot_air_heat_index_celsius gauge\n"
        "# UNIT iot_air_heat_index_celsius \u00B0C\n"
        "iot_air_heat_index_celsius %f\n";

    read_sensors();
    if (isnan(humidity) || isnan(temperature) || isnan(heat_index)) {
        http_server.send(500, "text/plain; charset=utf-8", "Sensor error.");
        return;
    }

    char response[BUFSIZE];
    snprintf(response, BUFSIZE, response_template, VERSION, BOARD_NAME, DHT_NAME, humidity, temperature, heat_index);
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
    read_heat_index();
}

void read_humidity_sensor() {
    log("Reading humidity sensor ...", LogLevel::DEBUG);
    bool result = read_sensor([] {
          return dht_sensor.getHumidity();
      }, &humidity);
    if (result) {
        humidity += HUMIDITY_CORRECTION_OFFSET;
    } else {
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
    } else {
        log("Failed to read temperature sensor.", LogLevel::ERROR);
    }
}

void read_heat_index() {
    if (!isnan(humidity) && !isnan(temperature)) {
        heat_index = dht_sensor.computeHeatIndex(temperature, humidity, false);
    } else {
        heat_index = NAN;
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
