# GreenhouseMonitor
Arduino code for an IoT device to measure soil moisture, temperature, and humidity in a greenhouse, and report those readings to a ThingSpeak channel.
Note that this code requires the following libraries:
<dht.h> - for the temperature & humidity sensor
<SPI.h> - for the WiFi shield
<WiFi.h> - for the WiFi shield
<U8glib.h> - for the OLED display
These libraries should all either be included in the Arduino IDE, or available through it (sketch > include library > manage libraries)
