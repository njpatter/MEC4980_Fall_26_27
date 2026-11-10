#include <Arduino.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <WebServer.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <time.h>

// Enter your Wi-Fi credentials and the coordinates of the desired weather location.
constexpr char wifiSsid[] = "ssid here";
constexpr char wifiPassword[] = "pwd here";
constexpr char locationName[] = "SG-32, Science Building, MSOE, Milwaukee, WI";
constexpr char latitude[] = "43.04302803179632";
constexpr char longitude[] = "-87.90678040825065";

// U.S. Central time: daylight saving starts in March and ends in November.
constexpr char centralTimeZone[] = "CST6CDT,M3.2.0/2,M11.1.0/2";

constexpr char page[] PROGMEM = R"html(
<!doctype html>
<html lang="en">
<head>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>ESP32 Weather and Time</title>
</head>
<body>
  <h1>ESP32 Weather and Time</h1>
  <p>Weather location: <span id="location"></span></p>
  <p id="weather">Press the button to get the temperature.</p>
  <button id="weatherButton" onclick="update('/weather', 'weather', this)">Update weather</button>
  <p id="clock">Press the button to get Central time.</p>
  <button id="clockButton" onclick="update('/clock', 'clock', this)">Update Central time</button>
  <script>
    async function update(url, id, button) {
      button.disabled = true;
      document.getElementById(id).textContent = 'Updating...';
      try {
        const response = await fetch(url, {cache: 'no-store'});
        const text = await response.text();
        document.getElementById(id).textContent = response.ok ? text : 'Error: ' + text;
      } catch (error) {
        document.getElementById(id).textContent = 'Could not reach the ESP32: ' + error;
      } finally {
        button.disabled = false;
      }
    }
    fetch('/location').then(response => response.text())
      .then(text => document.getElementById('location').textContent = text)
      .catch(() => document.getElementById('location').textContent = 'Unavailable');
    update('/weather', 'weather', document.getElementById('weatherButton'));
    update('/clock', 'clock', document.getElementById('clockButton'));
  </script>
</body>
</html>
)html";

WebServer server(80);
bool wasConnected = false;
bool serverStarted = false;
uint32_t lastReconnectMs = 0;

bool getJson(const String& url, JsonDocument& document, String& error) {
    if (WiFi.status() != WL_CONNECTED) {
        error = "Wi-Fi is disconnected.";
        return false;
    }
    WiFiClient client;
    client.setTimeout(10);
    HTTPClient http;
    http.setConnectTimeout(10000);
    http.setTimeout(10000);
    if (!http.begin(client, url)) {
        error = "Could not initialize HTTP request.";
        return false;
    }

    const int status = http.GET();
    if (status != HTTP_CODE_OK) {
        error = "HTTP request failed: ";
        error += status > 0 ? String(status) : HTTPClient::errorToString(status);
        http.end();
        return false;
    }

    const DeserializationError result = deserializeJson(document, http.getString());
    http.end();
    if (result) {
        error = String("Invalid JSON response: ") + result.c_str();
        return false;
    }
    return true;
}

bool getWeather(String& result) {
    const String url = String("http://api.open-meteo.com/v1/forecast?latitude=") +
        latitude + "&longitude=" + longitude +
        "&current=temperature_2m&temperature_unit=fahrenheit&timezone=America%2FChicago";
    JsonDocument document;
    if (!getJson(url, document, result)) {
        return false;
    }
    if (!document["current"]["temperature_2m"].is<float>() ||
        !document["current"]["time"].is<const char*>()) {
        result = "Weather response is missing the temperature or observation time.";
        return false;
    }
    result = String(locationName) + ": " +
        String(document["current"]["temperature_2m"].as<float>(), 1) +
        " F (observation time: " + document["current"]["time"].as<String>() + " Central)";
    return true;
}

bool getCentralTime(String& result) {
    const time_t now = time(nullptr);
    if (now < 1700000000) {
        result = "Waiting for NTP time synchronization; try again shortly.";
        return false;
    }
    struct tm localTime;
    char formattedTime[40];
    if (localtime_r(&now, &localTime) == nullptr ||
        strftime(formattedTime, sizeof(formattedTime), "%Y-%m-%d %H:%M:%S %Z", &localTime) == 0) {
        result = "Could not format the Central time.";
        return false;
    }
    result = formattedTime;
    return true;
}

void sendUpdate(bool (*update)(String&)) {
    String result;
    const bool success = update(result);
    Serial.println(result);
    server.sendHeader("Cache-Control", "no-store");
    server.send(success ? 200 : 502, "text/plain; charset=utf-8", result);
}

void setup() {
    delay(3000);  // Wait for the serial monitor to open.
    Serial.begin(115200);
    WiFi.mode(WIFI_STA);
    WiFi.setAutoReconnect(true);
    WiFi.begin(wifiSsid, wifiPassword);
    Serial.print("Connecting to Wi-Fi: ");
    Serial.println(wifiSsid);

    server.on("/", HTTP_GET, []() { server.send_P(200, "text/html; charset=utf-8", page); });
    server.on("/location", HTTP_GET, []() { server.send(200, "text/plain; charset=utf-8", locationName); });
    server.on("/weather", HTTP_GET, []() { sendUpdate(getWeather); });
    server.on("/clock", HTTP_GET, []() { sendUpdate(getCentralTime); });
}

void loop() {
    const bool connected = WiFi.status() == WL_CONNECTED;
    if (connected && !wasConnected) {
        Serial.print("Connected. Open http://");
        Serial.println(WiFi.localIP());
        configTzTime(centralTimeZone, "pool.ntp.org", "time.nist.gov");
        if (!serverStarted) {
            server.begin();
            serverStarted = true;
        }
        Serial.println("Synchronizing the clock with NTP. Use the web page to request updates.");
    } else if (!connected && wasConnected) {
        Serial.println("Wi-Fi disconnected; reconnecting.");
    }
    wasConnected = connected;

    if (connected) {
        server.handleClient();
    } else if (millis() - lastReconnectMs >= 15000) {
        lastReconnectMs = millis();
        Serial.println("Still connecting to Wi-Fi...");
        WiFi.reconnect();
    }
    delay(5);
}