// This HVAC control system is designed to run on low resource systems.


//#include "ESP8266WiFi.h"
#include "Thermostat.hpp"
#include "Scheduler.hpp"
//#include <set>


//#warning Remember to remove your access point data before commiting!
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

/*class EventT : public Scheduler::Event
{
public:
    int execute(Scheduler::Time const time) {
    return 0;
    }

    EventT(Scheduler::Time const executeTime, Scheduler::Time const executeTimeInterval = 0):
    Event(executeTime, executeTimeInterval)
    {}

    ~EventT()
    {}
};*/

void setup() 
{
    Serial.begin(115200);
    Serial.println("Setting up...");
   
	//WiFi.setOutputPower(0); // Temporary workaround for ESP-1 to ESP-6.
	//WiFi.begin(WIFI_SSID, WIFI_PASS);
}

void loop() 
{
    Thermostat::Thermometers thermometers = {Thermometer({14})};
    Thermostat thermostat({2,12,13}, thermometers);

    /*std::set<Scheduler::Event *, Scheduler::Event::PtrCompare> test;
    EventT e1(2);
    EventT e2(3);
    EventT e3(1);

    Serial.print("Before: ");
    Serial.println(test.size());
    test.insert(&e1);
    Serial.print("After inserting e1: ");
    Serial.println(test.size());
    test.insert(&e2);
    Serial.print("After inserting e2: ");
    Serial.println(test.size());
    test.insert(&e3);
    Serial.print("After inserting e3: ");
    Serial.println(test.size());*/


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
        Scheduler::UpdateAll(now);
        
        Serial.print("[Cycle] Completed at: ");
        Serial.println(now);
        
        wdt_reset(); // Notify watchdog microcontroller hasn't crashed.
    }
}
