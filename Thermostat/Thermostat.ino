// This HVAC control system is designed to run on low resource systems.


//#include "ESP8266WiFi.h"
#include "Thermostat.hpp"
#include "Scheduler.hpp"
#include "Development.hpp"


#warning Remember to remove your access point data before commiting!
const char *WIFI_SSID = "SSIDHERE";
const char *WIFI_PASS = "PASSHERE";

// NOTE: If objects are declared here from other files, and they've got 
// static members such as a static std::map member (like Pin), initialization 
// will not occur properly and will likely crash the ESP8266 module.
// The reason for that is because an object declared here is considered
// static, meaning it will be initialized with the static code in this
// translational unit (file), and this could run before the other
// translational unit's static code is initialized, such as the map member.
// This is fixed by having the instance in a function, which runs after
// the static code has been initialized, such as Thermostat in loop.

void setup() 
{
    Serial.begin(74880);
    Serial.println("Setting up...");
   
	//WiFi.setOutputPower(0); // Temporary workaround for ESP-1 to ESP-6.
	//WiFi.begin(WIFI_SSID, WIFI_PASS);
}

void loop() 
{
    Thermostat::Thermometers thermometers = {Thermometer({14})};
    Thermostat thermostat({14,12,13}, thermometers);

    thermostat.setTargetTemperature(Temperature<float>(70));
    thermostat.setMode(Thermostat::Mode::Cooling);

    /* while (WiFi.status() != WL_CONNECTED) {
		Serial.print("Attempting connection to \"");
		Serial.print(WIFI_SSID);
		Serial.print("\"... | Uptime ");
		Serial.println(millis());
		delay(5000); // Wait ~5 seconds.
	}

	Serial.print("Connected to \"");
	Serial.print(WIFI_SSID);
	Serial.print("\" with address ");
	Serial.println(WiFi.localIP()); */

    for (;;)
    {        
        // Scheduler at microsecond resolution.
        Scheduler::Time const now = micros();
        Scheduler::UpdateInstances(now);

#ifdef CYCLE_LOGS
        Serial.print("[Cycle] Completed at: ");
        Serial.println(now);
#endif
        
        wdt_reset(); // Notify watchdog microcontroller hasn't crashed.
    }
}
