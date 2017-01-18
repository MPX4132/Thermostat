//
//  DHT22.cpp
//  Thermostat
//
//  Created by Matias Barcenas on 1/15/17.
//  Copyright © 2017 Matias Barcenas. All rights reserved.
//

#include "DHT22.hpp"

#ifdef HARDWARE_INDEPENDENT
void delayMicroseconds(unsigned long time)
{
    // Fake test function.
}

unsigned long micros();
#endif

// ================================================================
// DHT22 Implementation
// ================================================================
bool DHT22::ready() const
{
#ifndef HARDWARE_INDEPENDENT
    return (this->_senseTime + DHT22_TIMEOUT) <= millis();
#else
    return true;
#endif
}

Sensor::Data DHT22::sense() {
    
    // Assure all pins are ready to use (data line).
    if (!this->ready()) return Sensor::Data();
    
#ifndef HARDWARE_INDEPENDENT
    // Log sensor's sense time to throttle usage while on cooldown.
    // NOTE: This is relevant for physical device, not simulation.
    this->_senseTime = millis();
#endif
    
    Sensor::Data data(5); // Buffer for data (40-bit)
    
    // Prepare data pin for operation.
    Pin &dataPin = this->_pins[this->_pinout[0]];
    dataPin.setMode(Pin::Mode::Output);
    
    // ============================================================
    // Pull down for 1000us, then up for 20us to wake DHT22
    // Lower values cause failure sporadically, so these are fine.
    // ============================================================
    dataPin.setState(0);
    delayMicroseconds(1000);
    
    dataPin.setState(1);
    delayMicroseconds(20);
    
    // ============================================================
    // Await response from the DHT22, 80us down followed by 80us up
    // ============================================================
    dataPin.setMode(Pin::Mode::Input);
    delayMicroseconds(40); // Wait a bit for the sensor to reply.
    
    
    // NOTE: POLING CAN BEGIN HERE WITH BLOCKING WAITS.
    // Wait till output high signal is pulled down by MCU.
    // This needs to be protected for potential hangs.
    // while (dataPin.state()) continue;
    
    
    // Check for low, if up return nothing.
    if (dataPin.state()) {
#if defined DEBUG && defined DHT22_LOGS
#ifndef HARDWARE_INDEPENDENT
        Serial.print("[DHT22 <");
        Serial.print((unsigned long) this);
        Serial.println(">] ERROR: No reply from sensor!");
#endif
#endif
        return Sensor::Data();
    }
    delayMicroseconds(80);
    
    // Wait for the MCU to repeat the down part of down-up signal.
    // This needs to be protected for potential hangs.
    // while (!dataPin.state()) continue;
    
    
    // Check for high, if low, return nothing.
    if (!dataPin.state()) {
#if defined DEBUG && defined DHT22_LOGS
#ifndef HARDWARE_INDEPENDENT
        Serial.print("[DHT22 <");
        Serial.print((unsigned long) this);
        Serial.println(">] ERROR: Invalid reply from sensor!");
#endif
#endif
        return Sensor::Data();
    }
    delayMicroseconds(40);
    
    // Wait for the MCU to repeat the up part of down-up signal.
    // This needs to be protected for potential hangs.
    //while (dataPin.state()) continue;
    
    // Get 40 bits of data from the sensor.
    for (Sensor::Byte &byte : data)
    {
        // Each byte has 8 bits, loop 8 times for each byte.
        for (Sensor::Byte bit = 0; bit < 8; bit++)
        {
            // Wait for high signal signifying next bit started.
            // WARNING: This needs to be protected for potential hangs.
            while (!dataPin.state()) continue; // NOTE: BLOCKING WAIT
            unsigned short const timeA = micros();

            // Wait for low signal, signifying start of next bit.
            // WARNING: This needs to be protected for potential hangs.
            while (dataPin.state()) continue; // NOTE: BLOCKING WAIT
            unsigned short const timeB = micros();
            
            // If high signal lasted > ~30us, it's a 1, 0 otherwise.
            byte = (byte << 1) | (timeB - timeA > 50);
        }
    }
    
    // NOTE: Temperature is in Celcius and is x10 scaled, including humidity.
    float const humidity = static_cast<float>(((static_cast<unsigned short>(data[0]) << 8) | data[1])) / 10;
    float const temperature = static_cast<float>(((static_cast<unsigned short>(data[2]) << 8) | data[3])) / 10;
    
#if defined DEBUG && defined DHT22_LOGS
#ifndef HARDWARE_INDEPENDENT
    
    Serial.print("[DHT22 <");
    Serial.print((unsigned long) this);
    Serial.print(">] All Data: ");
    for (Sensor::Byte &byte : data)
    {
        Serial.print(byte, BIN);
        Serial.print(" ");
    }
    
    Serial.println("");
    
    Serial.print("[DHT22 <");
    Serial.print((unsigned long) this);
    Serial.print(">] RHu Data: ");
    Serial.print(data[0], BIN);
    Serial.print(" ");
    Serial.print(data[1], BIN);
    Serial.print(" = ");
    
    Serial.println(humidity);
    
    Serial.print("[DHT22 <");
    Serial.print((unsigned long) this);
    Serial.print(">] Tmp Data: ");
    Serial.print(data[2], BIN);
    Serial.print(" ");
    Serial.print(data[3], BIN);
    Serial.print(" = ");
    
    Serial.println(temperature);
    
    Serial.print("[DHT22 <");
    Serial.print((unsigned long) this);
    Serial.print(">] Chk Data: ");
    Serial.print(data[4], BIN);
    Serial.print(" = ");
    Serial.println((unsigned short) data[4]);
#endif
#endif
    
    if (this->_validData(data)) // Update cached values only if valid data received.
    {
#if defined DEBUG && defined DHT22_LOGS
#ifndef HARDWARE_INDEPENDENT
        Serial.print("[DHT22 <");
        Serial.print((unsigned long) this);
        Serial.println(">] Data checks out.");
        Serial.print("[DHT22 <");
        Serial.print((unsigned long) this);
        Serial.print(">] Cached values updating to T: ");
        Serial.print(temperature);
        Serial.print(" H: ");
        Serial.println(humidity);
#endif
#endif
        this->_temperature = Temperature<float>(temperature, Temperature<float>::Scale::Celsius);
        this->_humidity = humidity;
    }
#if defined DEBUG && defined DHT22_LOGS
#ifndef HARDWARE_INDEPENDENT
    else
    {
        Serial.print("[DHT22 <");
        Serial.print((unsigned long) this);
        Serial.println(">] ERROR: Data is corrupted!");
    }
#endif
#endif
    
    return data;
}

bool DHT22::DHT22::_validData(Sensor::Data const &data)
{
    return (data.size() == 5) && (data[4] == static_cast<Sensor::Byte>(data[0] + data[1] + data[2] + data[3]));
}

DHT22::DHT22(Pin::Identifier const pin):
Thermometer({pin},
std::make_pair(Temperature<float>(-40, Temperature<float>::Scale::Celsius),
               Temperature<float>(80, Temperature<float>::Scale::Celsius))),
_senseTime(0)
{
    
}

DHT22::~DHT22()
{
    
}
