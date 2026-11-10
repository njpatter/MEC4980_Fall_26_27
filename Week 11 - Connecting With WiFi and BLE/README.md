# Wi-Fi weather and time example

1. In `src/wifi_example.cpp`, set `wifiSsid` and `wifiPassword` to your
   network credentials. Do not commit real passwords. The ESP32 needs a
   2.4 GHz Wi-Fi network with internet access; enterprise login and captive
   portals are not handled by this example.
2. Set `locationName`, `latitude`, and `longitude` for the weather location.
   The defaults are Chicago, Illinois. Temperature is reported in Fahrenheit.
3. Build and upload the `wifi_example` PlatformIO environment, then open the
   serial monitor at 115200 baud. After connecting, the ESP32 prints its IP
   address.
4. On a device connected to the same network, open the printed `http://`
   address. The page requests both values when opened; use the buttons to
   request an update to the temperature or Central time.
   The temperature includes its observation time; the clock is a snapshot
   updated when its button is pressed, not a continuously running display.

Open-Meteo supplies weather without an API key, using HTTP. The ESP32
synchronizes its clock using NTP and applies U.S. Central time rules,
including the switch between CST and CDT. If the clock reports that
synchronization is pending, wait and try again. Internet, NTP, or API
failures are reported on the page and serial monitor.

The weather request and local web page use unencrypted HTTP, and NTP is not
authenticated. Network traffic and responses can be read or altered; use this
example only on a trusted network. Some networks isolate wireless clients and
will prevent other devices from reaching the ESP32.
