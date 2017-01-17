// This HVAC control system is designed to run on low resource systems.


#include "Development.hpp"

#ifndef HARDWARE_INDEPENDENT
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include "Thermostat.hpp"
#include "Scheduler.hpp"
#include "Sensor.hpp"
#include "DHT22.hpp"

#warning Remember to remove your access point data before commiting!
char const * const WIFI_SSID = "SSIDHERE";
char const * const WIFI_PASS = "PASSHERE";

// NOTE: If objects are declared here from other files, and they've got 
// static members such as a static std::map member (like Pin), initialization 
// will not occur properly and will likely crash the ESP8266 module.
// The reason for that is because an object declared here is considered
// static, meaning it will be initialized with the static code in this
// translational unit (file), and this could run before the other
// translational unit's static code is initialized, such as the map member.
// This is fixed by having the instance in a function, which runs after
// the static code has been initialized, such as Thermostat in loop.

DHT22 * thermometer; // Will point to the thermometer (DHT22) instance.
Thermostat * thermostat; // Will point to the thermostat instance.

// Prepare the web server object with typical port 80 for HTTP requests.
ESP8266WebServer server(80);

void setup() 
{
    Serial.begin(115200);
    Serial.println("Thermostat starting up...");

    // We'll start all thermostat related objects first to have them ready.
    thermometer = new DHT22(2); // Should stabilize while waiting for WIFI to connect.
    thermostat = new Thermostat({14,12,13}, {thermometer});

    // Set some test Thermostat temerature parameters to demo the objects.
    // NOTE: Thermostat objects rely on the Scheduler class's updates,
    // meaning nothing will execute until the Scheduler class begins to update
    // all its instances, meaning it's also safe to set these values here.
    thermostat->setTargetTemperature(Temperature<float>(72));
    thermostat->setMode(Thermostat::Mode::Auto);


    // Begin WiFi configuration and do not continue until we've connected successfully.
    Serial.println("[WIFI] Setting radio configuration, please wait...");
	WiFi.begin(WIFI_SSID, WIFI_PASS); // Configure and activate WIFI for connection.

    // Check WIFI connection state, and wait if it's not yet ready.
    Serial.println("[WIFI] Checking connection state...");
    while (WiFi.status() != WL_CONNECTED) {
        Serial.print("[WIFI] Waiting for connection to \"");
        Serial.print(WIFI_SSID);
        Serial.print("\" @ ");
        Serial.print(millis());
        // Give it a bit more time to attempt connecting.
        for (byte i = 0; i < 3; i++) {delay(1000); Serial.print(".");}
        Serial.println("");
    }

    Serial.print("[WIFI] Connected to \"");
    Serial.print(WIFI_SSID);
    Serial.print("\" with address ");
    Serial.println(WiFi.localIP());


    // Begin web server for serving thermostat related data & operations.
    server.on("/", []() {
        String response;
        response += "[{\"T\":";
        response += thermometer->temperature().value();
        response += ",\"H\":";
        response += thermometer->humidity();
        response += "}]";
        server.send(200, "application/json", response);
    });

    server.begin();
}

void loop()
{
    // Scheduler at microsecond resolution.
    Scheduler::Time const now = micros();
    Scheduler::UpdateInstances(now);

    if (WiFi.status() != WL_CONNECTED)
    {
        Serial.println("[WIFI] Radio is disconnected..."); return;
    }

    server.handleClient();

#ifdef CYCLE_LOGS
    Serial.print("[Cycle] Completed at: ");
    Serial.println(now);
#endif
}
#endif

