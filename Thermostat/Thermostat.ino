//
//  Thermostat.ino
//  Thermostat
//
//  Created by Matias Barcenas on 1/11/17.
//  Copyright © 2017 Matias Barcenas. All rights reserved.
//

// Description: This HVAC control system is designed to run on low resource systems.

#ifndef Thermostat_ino
#define Thermostat_ino

#include "Development.hpp"

#if defined(MJB_ARDUINO_LIB_API)
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <ESP8266WebServer.h>
#include <FS.h>
#endif

#include <memory>

#include "Thermostat.hpp"
#include "Scheduler.hpp"
#include "Sensor.hpp"
#include "DHT22.hpp"
#include "Pin.hpp"


#warning Remember to remove your access point data before commiting!
#if defined(MJB_ARDUINO_LIB_API)
char const * const WIFI_SSID = "SSIDHERE";
char const * const WIFI_PASS = "PASSHERE";
#endif

Pin::Identifier const temperatureSensorDataPin = 2;
Pin::Identifier const thermostatBlowerPin = 14;
Pin::Identifier const thermostatCoolingPin = 12;
Pin::Identifier const thermostatHeatingPin = 13;

std::shared_ptr<DHT22> thermometer(std::make_shared<DHT22>(temperatureSensorDataPin));

Thermostat thermostat(Pin::Arrangement({
    thermostatBlowerPin,
    thermostatCoolingPin,
    thermostatHeatingPin
}), Thermostat::Thermometers({
    // DHT22 should stabilize while waiting for WIFI to connect.
    thermometer
}), 5000000);

#if defined(MJB_ARDUINO_LIB_API)
// Prepare the web server object with typical port 80 for HTTP requests.
ESP8266WebServer server(80);

String GetContentType(String const filename);
String GetStatusData();
#endif

void setup()
{
#if defined(MJB_ARDUINO_LIB_API)
    Serial.begin(115200);
    MJB_DEBUG_LOG_LINE("Thermostat starting up...");
#endif

    thermostat.setTargetTemperature(Thermometer::TemperatureUnit(72, Thermometer::TemperatureUnit::Scale::Fahrenheit));
    thermostat.setMode(Thermostat::Mode::Auto);

#if defined(MJB_ARDUINO_LIB_API)
    // Begin WiFi configuration and do not continue until we've connected successfully.
    MJB_DEBUG_LOG_LINE("[WIFI] Setting radio configuration, please wait...");
    WiFi.mode(WIFI_STA); // Configure as a WIFI station, rather than an access point. 
    WiFi.begin(WIFI_SSID, WIFI_PASS); // Configure and activate WIFI for connection.

    // Check WIFI connection state, and wait if it's not yet ready.
    MJB_DEBUG_LOG_LINE("[WIFI] Checking connection state...");
    while (WiFi.status() != WL_CONNECTED) {
        MJB_DEBUG_LOG("[WIFI] Waiting for connection to \"");
        MJB_DEBUG_LOG(WIFI_SSID);
        MJB_DEBUG_LOG("\" @ ");
        MJB_DEBUG_LOG(millis());
        // Give it a bit more time to attempt connecting.
        for (byte i = 0; i < 3; i++) {delay(1000); MJB_DEBUG_LOG(".");}
        MJB_DEBUG_LOG_LINE("");
    }
    
    MJB_DEBUG_LOG("[WIFI] Connected to \"");
    MJB_DEBUG_LOG(WIFI_SSID);
    MJB_DEBUG_LOG("\" with address ");
    MJB_DEBUG_LOG_LINE(WiFi.localIP());
    
    
    // Begin web server for serving thermostat related data & operations.
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
                thermostat.setMode(Thermostat::Mode::Auto);
            else
            if (modeArgument.equals("cool") || modeArgument.equals("2"))
                thermostat.setMode(Thermostat::Mode::Cool);
            else
            if (modeArgument.equals("heat") || modeArgument.equals("1"))
                thermostat.setMode(Thermostat::Mode::Heat);
            else
            if (modeArgument.equals("off") || modeArgument.equals("0"))
                thermostat.setMode(Thermostat::Mode::Off);
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
                    thermostat.setTargetTemperature(Temperature<float>(value, static_cast<Temperature<float>::Scale>(scale)));
                }
            }
        }
        
        String const tempTypeArgument = server.arg("type");
        if (tempTypeArgument.length())
        {
            if (tempTypeArgument.equals("HI") || tempTypeArgument.equals("1"))
                thermostat.setPerceptionIndex(Thermostat::PerceptionIndex::HeatIndex);
            else
            if (tempTypeArgument.equals("T") || tempTypeArgument.equals("0"))
                thermostat.setPerceptionIndex(Thermostat::PerceptionIndex::TemperatureIndex);
        }
        
        // Push changes by manually updating the instance.
        // This will reflect changes immediately which will be sent as JSON.
        thermostat.update(micros());
        
        server.send(200, "application/json", GetStatusData());
    });
    
    SPIFFS.begin();
    
    server.begin();
    
    MDNS.begin("thermostasis");
#endif
}

void loop()
{
    // Scheduler at microsecond resolution.
    Scheduler::Time const now = micros();
    Scheduler::UpdateInstances(now);

#if defined(MJB_ARDUINO_LIB_API)
    if (WiFi.status() != WL_CONNECTED)
    {
        MJB_DEBUG_LOG("[WIFI] Radio is down, time is ");
        MJB_DEBUG_LOG_LINE(now); return; // Can't handle clients.
    }
    
    server.handleClient();
#endif
    
#ifdef MJB_DEBUG_LOGGING_CYCLE
    MJB_DEBUG_LOG("[Cycle] Completed at: ");
    MJB_DEBUG_LOG_LINE(now);
#endif
}

#if defined(MJB_ARDUINO_LIB_API)
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
    statusData += thermostat.temperature().value();
    statusData += ",\"target\":";
    statusData += thermostat.targetTemperature().value();
    statusData += ",\"scale\":\"K\",\"delay\":\"?\"}";
    statusData += ",\"humiture\":";
    statusData += thermostat.humiture().value();
    statusData += ",\"measurement\":";
    statusData += thermostat.perceptionIndex();
    statusData += ",\"humidity\":{\"current\":";
    statusData += thermometer->humidity();
    statusData += "},\"mode\":";
    statusData += thermostat.mode();
    statusData += ",\"status\":";
    statusData += thermostat.status();
    statusData += "}";
    return statusData;
}
#endif

#endif
