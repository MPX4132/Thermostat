// This HVAC control system is designed to run on low resource systems.


#include "Development.hpp"

#ifndef HARDWARE_INDEPENDENT
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <ESP8266WebServer.h>
#include <FS.h>
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

String GetContentType(String const filename);
String GetStatusData();

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
    // Setting these via the web parameter interface.
    //thermostat->setTargetTemperature(Temperature<float>(72));
    //thermostat->setMode(Thermostat::Mode::Auto);
    
    
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
    /*server.on("/", []() {
        String response;
        response += "Currently ";
        response += thermometer->temperature().value();
        response += "F, Humidity @ ";
        response += thermometer->humidity();
        response += "%";
        server.send(200, "text/plain", response);
    });*/
    
    server.on("/status", []() {
        server.send(200, "application/json", GetStatusData());
    });
    
    server.on("/control", []() {
        if (!server.args())
        {
            server.send(200, "text/plain", "Thermostat's controller interface."); return;
        }
        
        String const modeArgument = server.arg("mode");
        if (modeArgument.length())
        {
            if (modeArgument.equals("auto") || modeArgument.equals("3"))
                thermostat->setMode(Thermostat::Mode::Auto);
            if (modeArgument.equals("cool") || modeArgument.equals("2"))
                thermostat->setMode(Thermostat::Mode::Cool);
            if (modeArgument.equals("heat") || modeArgument.equals("1"))
                thermostat->setMode(Thermostat::Mode::Heat);
            if (modeArgument.equals("off") || modeArgument.equals("0"))
                thermostat->setMode(Thermostat::Mode::Off);
        }
        
        String tempArgument = server.arg("temp");
        if (tempArgument.length())
        {
            // Get the unit from the passed in temperature.
            short scalePosition = tempArgument.indexOf("K");
            if (scalePosition < 0) scalePosition = tempArgument.indexOf("F");
            if (scalePosition < 0) scalePosition = tempArgument.indexOf("C");
            
            if (scalePosition > 0) // Check if it was found after 0, otherwise it's malformed.
            {
                char const scale = tempArgument[scalePosition];
                
                tempArgument.remove(scalePosition); // Remove scale to parse value.
                float const value = tempArgument.toFloat(); // Parse value, 0 COULD be failure.
                
                // Check if it's a proper value meaning it must not be zero, however,
                // if it is 0 we must check it indeed was set and isn't an issue with parsing.
                if (value || (tempArgument.length() == 1 && tempArgument[0] == '0'))
                {
                    thermostat->setTargetTemperature(Temperature<float>(value, static_cast<Temperature<float>::Scale>(scale)));
                }
            }
        }
        
        // Push changes by manually updating the instance.
        // This will reflect changes immediately which will be sent as JSON.
        thermostat->update(micros());
        
        server.send(200, "application/json", GetStatusData());
    });
    
    server.onNotFound([]() {
        String const path = server.uri().equals("/")? "/index.html" : server.uri();
        
        if (!SPIFFS.exists(path))
        {
            server.send(404, "text/plain", "Not Found!"); return;
        }
        
        File file = SPIFFS.open(path, "r");
        server.streamFile(file, GetContentType(path));
        file.close();
    });
    
    SPIFFS.begin();
    
    server.begin();
    
    MDNS.begin("thermostasis");
}

void loop()
{
    // Scheduler at microsecond resolution.
    Scheduler::Time const now = micros();
    Scheduler::UpdateInstances(now);
    
    if (WiFi.status() != WL_CONNECTED)
    {
        Serial.print("[WIFI] Radio is down, time is ");
        Serial.println(now); return; // Can't handle clients.
    }
    
    server.handleClient();
    
#ifdef CYCLE_LOGS
    Serial.print("[Cycle] Completed at: ");
    Serial.println(now);
#endif
}

String GetContentType(String const filename)
{
    if(server.hasArg("download")) return "application/octet-stream";
    else if(filename.endsWith(".htm")) return "text/html";
    else if(filename.endsWith(".html")) return "text/html";
    else if(filename.endsWith(".css")) return "text/css";
    else if(filename.endsWith(".js")) return "application/javascript";
    return "text/plain";
}

String GetStatusData()
{
    String statusData;
    statusData += "{\"temperature\": {\"current\":";
    statusData += thermostat->temperature().value();
    statusData += ",\"target\":";
    statusData += thermostat->targetTemperature().value();
    statusData += ",\"scale\":\"K\",\"delay\":";
    statusData += "\"?\"},\"humidity\":{\"current\":";
    statusData += thermometer->humidity();
    statusData += "},\"mode\":";
    statusData += thermostat->mode();
    statusData += ",\"status\":";
    statusData += thermostat->status();
    statusData += "}";
    return statusData;
}

#endif

