#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <DHTesp.h>

#include "config.h"
#include "version.h"

#define EXPLODE4(arr) (arr[0], arr[1], arr[2], arr[3])

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
    char message[128];
    Serial.begin(9600);
    setup_dht_sensor();
    setup_wifi();
    setup_http_server();
    snprintf(message, 128, "Prometheus namespace: %s", PROM_NAMESPACE);
    log(message);
    log("Setup done");
}

void setup_dht_sensor() {
    log("Setting up DHT sensor");
    dht_sensor.setup(DHT_PIN, DHTesp::DHT_TYPE);
    delay(dht_sensor.getMinimumSamplingPeriod());
    // Test read
    read_sensors(true);
    log("DHT sensor ready", LogLevel::DEBUG);
}

void setup_wifi() {
    char message[128];
    log("Setting up Wi-Fi");
    snprintf(message, 128, "Wi-Fi SSID: %s", WIFI_SSID);
    log(message, LogLevel::DEBUG);
    snprintf(message, 128, "MAC address: %s", WiFi.macAddress().c_str());
    log(message, LogLevel::DEBUG);
    snprintf(message, 128, "Initial hostname: %s", WiFi.hostname().c_str());
    log(message, LogLevel::DEBUG);

    WiFi.mode(WIFI_STA);

    #if WIFI_IPV4_STATIC == true
        log("Using static IPv4 adressing");
        IPAddress static_address(WIFI_IPV4_ADDRESS);
        IPAddress static_subnet(WIFI_IPV4_SUBNET_MASK);
        IPAddress static_gateway(WIFI_IPV4_GATEWAY);
        IPAddress static_dns1(WIFI_IPV4_DNS_1);
        IPAddress static_dns2(WIFI_IPV4_DNS_2);
        if (!WiFi.config(static_address, static_gateway, static_subnet, static_dns1, static_dns2)) {
            log("Failed to configure static addressing", LogLevel::ERROR);
        }
    #endif

    #ifdef WIFI_HOSTNAME
        log("Requesting hostname: " WIFI_HOSTNAME);
        if (WiFi.hostname(WIFI_HOSTNAME)) {
            log("Hostname changed");
        } else {
            log("Failed to change hostname (too long?)", LogLevel::ERROR);
        }
    #endif

    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    while (WiFi.status() != WL_CONNECTED) {
        log("Wi-Fi connection not ready, waiting", LogLevel::DEBUG);
        delay(500);
    }

    log("Wi-Fi connected.");
    snprintf(message, 128, "SSID: %s", WiFi.SSID().c_str());
    log(message);
    snprintf(message, 128, "BSSID: %s", WiFi.BSSIDstr().c_str());
    log(message);
    snprintf(message, 128, "Hostname: %s", WiFi.hostname().c_str());
    log(message);
    snprintf(message, 128, "MAC address: %s", WiFi.macAddress().c_str());
    log(message);
    snprintf(message, 128, "IPv4 address: %s", WiFi.localIP().toString().c_str());
    log(message);
    snprintf(message, 128, "IPv4 subnet mask: %s", WiFi.subnetMask().toString().c_str());
    log(message);
    snprintf(message, 128, "IPv4 gateway: %s", WiFi.gatewayIP().toString().c_str());
    log(message);
    snprintf(message, 128, "Primary DNS server: %s", WiFi.dnsIP(0).toString().c_str());
    log(message);
    snprintf(message, 128, "Secondary DNS server: %s", WiFi.dnsIP(1).toString().c_str());
    log(message);
}
void setup_http_server() {
    char message[128];
    log("Setting up HTTP server");
    http_server.on("/", HTTPMethod::HTTP_GET, handle_http_root);
    http_server.on(HTTP_METRICS_ENDPOINT, HTTPMethod::HTTP_GET, handle_http_metrics);
    http_server.onNotFound(handle_http_not_found);
    http_server.begin();
    log("HTTP server started", LogLevel::DEBUG);
    snprintf(message, 128, "Metrics endpoint: %s", HTTP_METRICS_ENDPOINT);
    log(message);
}

void loop(void) {
    http_server.handleClient();
}

void handle_http_root() {
    log_request();
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

void handle_http_metrics() {
    log_request();
    static size_t const BUFSIZE = 1024;
    static char const *response_template =
        "# HELP " PROM_NAMESPACE "_info Metadata about the device.\n"
        "# TYPE " PROM_NAMESPACE "_info gauge\n"
        "# UNIT " PROM_NAMESPACE "_info \n"
        PROM_NAMESPACE "_info{version=\"%s\",board=\"%s\",sensor=\"%s\"} 1\n"
        "# HELP " PROM_NAMESPACE "_air_humidity_percent Air humidity.\n"
        "# TYPE " PROM_NAMESPACE "_air_humidity_percent gauge\n"
        "# UNIT " PROM_NAMESPACE "_air_humidity_percent %%\n"
        PROM_NAMESPACE "_air_humidity_percent %f\n"
        "# HELP " PROM_NAMESPACE "_air_temperature_celsius Air temperature.\n"
        "# TYPE " PROM_NAMESPACE "_air_temperature_celsius gauge\n"
        "# UNIT " PROM_NAMESPACE "_air_temperature_celsius \u00B0C\n"
        PROM_NAMESPACE "_air_temperature_celsius %f\n"
        "# HELP " PROM_NAMESPACE "_air_heat_index_celsius Apparent air temperature, based on temperature and humidity.\n"
        "# TYPE " PROM_NAMESPACE "_air_heat_index_celsius gauge\n"
        "# UNIT " PROM_NAMESPACE "_air_heat_index_celsius \u00B0C\n"
        PROM_NAMESPACE "_air_heat_index_celsius %f\n";

    read_sensors();
    if (isnan(humidity) || isnan(temperature) || isnan(heat_index)) {
        http_server.send(500, "text/plain; charset=utf-8", "Sensor error.");
        return;
    }

    char response[BUFSIZE];
    snprintf(response, BUFSIZE, response_template, VERSION, BOARD_NAME, DHT_NAME, humidity, temperature, heat_index);
    http_server.send(200, "text/plain; charset=utf-8", response);
}

void handle_http_not_found() {
    log_request();
    http_server.send(404, "text/plain; charset=utf-8", "Not found.");
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

void log_request() {
    char message[128];
    char method_name[16];
    get_http_method_name(method_name, 16, http_server.method());
    snprintf(message, 128, "Request: client=%s:%u method=%s path=%s",
            http_server.client().remoteIP().toString().c_str(), http_server.client().remotePort(), method_name, http_server.uri().c_str());
    log(message, LogLevel::INFO);
}

void get_http_method_name(char *name, size_t name_length, HTTPMethod method) {
    switch (method) {
    case HTTP_GET:
        snprintf(name, name_length, "GET");
        break;
    case HTTP_HEAD:
        snprintf(name, name_length, "HEAD");
        break;
    case HTTP_POST:
        snprintf(name, name_length, "POST");
        break;
    case HTTP_PUT:
        snprintf(name, name_length, "PUT");
        break;
    case HTTP_PATCH:
        snprintf(name, name_length, "PATCH");
        break;
    case HTTP_DELETE:
        snprintf(name, name_length, "DELETE");
        break;
    case HTTP_OPTIONS:
        snprintf(name, name_length, "OPTIONS");
        break;
    default:
        snprintf(name, name_length, "UNKNOWN");
        break;
    }
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
