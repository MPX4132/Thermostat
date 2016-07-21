// This HVAC control system is designed to run on low resource systems.


#include "ESP8266WiFi.h"
#include "Thermostat.hpp"


const char *WIFI_SSID = "SSIDHERE";
const char *WIFI_PASS = "PASSHERE";

Thermostat thermostat;

void setup() {
	Serial.begin(74880);

	WiFi.setOutputPower(0); // Temporary workaround for ESP-1 to ESP-6.
	WiFi.begin(WIFI_SSID, WIFI_PASS);
}

void loop() {
	while (WiFi.status() != WL_CONNECTED) {
		Serial.print("Attempting connection to \"");
		Serial.print(WIFI_SSID);
		Serial.print("\"... | Uptime ");
		Serial.println(millis());
		delay(5000); // Wait ~5 seconds.
	}

	Serial.print("Connected to \"");
	Serial.print(WIFI_SSID);
	Serial.print("\" with address ");
	Serial.println(WiFi.localIP());
	delay(5000);
}
