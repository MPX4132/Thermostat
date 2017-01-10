// This HVAC control system is designed to run on low resource systems.


#include "ESP8266WiFi.h"
#include "Pin.hpp"
//#include <map>
//#include "Thermostat.hpp"


#warning Remember to remove your access point data before commiting!
const char *WIFI_SSID = "SSIDHERE";
const char *WIFI_PASS = "PASSHERE";

//Thermostat::Thermometers thermometers = {Thermometer({3,4})};
//Thermostat thermostat({0,1,2}, thermometers);

/*class Test {
public:
    int data() const 
    {
        return this->_data;
    }

    Test(int x): _data(x) 
    {
        Test::_Reserve(this);
    }
    ~Test() {};
protected:
    int _data;

    static std::map<int, Test const *>thing;

    static bool _Reserve(Test const * const t) {
        Test::thing[t->data()] = t;
    }
};*/

//std::map<int, Test const *> Test::thing;

Pin pin(2);
//Test test(2);

//std::map<Pin::Identifier, Pin const *> test;
void setup() {
	Serial.begin(115200);
    if (pin.setMode(Pin::Mode::Output)) Serial.println("Mode was set successfully!");
    else Serial.println("Unable to set the proper mode");
	//WiFi.setOutputPower(0); // Temporary workaround for ESP-1 to ESP-6.
	//WiFi.begin(WIFI_SSID, WIFI_PASS);
}

void loop() {
    /*
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
    */
    Serial.print("Cycled: ");
    Serial.println(millis());
    Serial.print("Value is ");
    Serial.println(pin.state());
    Serial.print("Trying to change state: ");
    if (pin.setState(!pin.state())) Serial.println("Success!");
    else Serial.println("Failure!");
	delay(1000);
}
