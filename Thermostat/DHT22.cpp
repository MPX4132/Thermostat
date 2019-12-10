//
//  DHT22.cpp
//  Thermostat
//
//  Created by Matias Barcenas on 1/15/17.
//  Copyright © 2017 Matias Barcenas. All rights reserved.
//

#include "DHT22.hpp"

#if ! defined(MJB_ARDUINO_LIB_API)
void delayMicroseconds(unsigned long time)
{
    // The following done to suppress unused variable warnings.
    (void) time;
    // Fake test function.
}
#endif

// ================================================================
// DHT22 Implementation
// ================================================================
Sensor::Data DHT22::sense() {
    
    // Assure all pins are ready to use (data line).
    // If the sensor isn't ready, return empty data.
    if (status() != Actuator::Status::Ready) return Sensor::Data();

#if defined(MJB_DEBUG_LOGGING_DHT22)
    MJB_DEBUG_LOG("[DHT22 <");
    MJB_DEBUG_LOG_FORMAT((unsigned long) this, MJB_DEBUG_LOG_HEX);
    MJB_DEBUG_LOG(">] NOTICE: Starting sense operation with status ");
    MJB_DEBUG_LOG_LINE(status());
#endif
    
    Sensor::Data data(5); // Buffer for data (40-bit)
    
    // Prepare data pin for operation.
    Pin &dataPin = *(_pins[pinout[DHT22::Pinout::Data]]);
    dataPin.setMode(Pin::Mode::Output);

    Sensor::sense(); // Prevent interrptions by causing timeout.
    
    // ============================================================
    // Pull down for 1000us, then up for 20us to wake DHT22
    // Lower values cause failure sporadically, so these are fine.
    // ============================================================
    dataPin.setValue(0);
    delayMicroseconds(1000);
    
    dataPin.setValue(1);
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
    if (dataPin.value()) {
#if defined(MJB_DEBUG_LOGGING_DHT22)
        MJB_DEBUG_LOG("[DHT22 <");
        MJB_DEBUG_LOG_FORMAT((unsigned long) this, MJB_DEBUG_LOG_HEX);
        MJB_DEBUG_LOG_LINE(">] WARNING: No reply from sensor!");
#endif
        return Sensor::Data();
    }
    delayMicroseconds(80);
    
    // Wait for the MCU to repeat the down part of down-up signal.
    // This needs to be protected for potential hangs.
    // while (!dataPin.state()) continue;
    
    
    // Check for high, if low, return nothing.
    if (!dataPin.value()) {
#if defined(MJB_DEBUG_LOGGING_DHT22)
        MJB_DEBUG_LOG("[DHT22 <");
        MJB_DEBUG_LOG_FORMAT((unsigned long) this, MJB_DEBUG_LOG_HEX);
        MJB_DEBUG_LOG_LINE(">] ERROR: Invalid reply from sensor!");
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
            while (!dataPin.value()) continue; // NOTE: BLOCKING WAIT
            unsigned short const timeA = micros();

            // Wait for low signal, signifying start of next bit.
            // WARNING: This needs to be protected for potential hangs.
            while (dataPin.value()) continue; // NOTE: BLOCKING WAIT
            unsigned short const timeB = micros();
            
            // If high signal lasted > ~30us, it's a 1, 0 otherwise.
            byte = (byte << 1) | (timeB - timeA > 50);
        }
    }

    uint16_t const hRaw = ((static_cast<uint16_t>(data[0]) << 8) | data[1]);

    // Humidity (percentage) is in unsigned scalar format, x10 scaled.
    float const humidity = static_cast<float>(hRaw) / 10;

    // Temperature (Celcius) is in signed-magnitude format, x10 scaled.
    uint16_t const tRaw = ((static_cast<uint16_t>(data[2]) << 8) | data[3]);
    
    // Temperature (Celcius) must be converted from signed-magnitude to two's complement.
    float const temperature = static_cast<float>((tRaw & 0x7FFF) * ((tRaw & 0x8000)? -1 : 1)) / 10;

#if defined(MJB_DEBUG_LOGGING_DHT22)
    MJB_DEBUG_LOG("[DHT22 <");
    MJB_DEBUG_LOG_FORMAT((unsigned long) this, MJB_DEBUG_LOG_HEX);
    MJB_DEBUG_LOG(">] All Data: ");

    for (Sensor::Byte &byte : data)
    {
        MJB_DEBUG_LOG_FORMAT(byte, MJB_DEBUG_LOG_BIN);
        MJB_DEBUG_LOG(" ");
    }
    
    MJB_DEBUG_LOG_LINE("");
    
    MJB_DEBUG_LOG("[DHT22 <");
    MJB_DEBUG_LOG_FORMAT((unsigned long) this, MJB_DEBUG_LOG_HEX);
    MJB_DEBUG_LOG(">] RHu Data: ");
    MJB_DEBUG_LOG_FORMAT(data[0], MJB_DEBUG_LOG_BIN);
    MJB_DEBUG_LOG(" ");
    MJB_DEBUG_LOG_FORMAT(data[1], MJB_DEBUG_LOG_BIN);
    MJB_DEBUG_LOG(" = ");
    MJB_DEBUG_LOG(hRaw);
    MJB_DEBUG_LOG(" | normalized = ");
    MJB_DEBUG_LOG_LINE_FORMAT(humidity, MJB_DEBUG_LOG_DEC);
    
    MJB_DEBUG_LOG("[DHT22 <");
    MJB_DEBUG_LOG_FORMAT((unsigned long) this, MJB_DEBUG_LOG_HEX);
    MJB_DEBUG_LOG(">] Tmp Data: ");
    MJB_DEBUG_LOG_FORMAT(data[2], MJB_DEBUG_LOG_BIN);
    MJB_DEBUG_LOG(" ");
    MJB_DEBUG_LOG_FORMAT(data[3], MJB_DEBUG_LOG_BIN);
    MJB_DEBUG_LOG(" = ");
    
    MJB_DEBUG_LOG(tRaw);
    MJB_DEBUG_LOG(" | normalized = ");
    MJB_DEBUG_LOG_LINE_FORMAT(temperature, MJB_DEBUG_LOG_DEC);
    
    MJB_DEBUG_LOG("[DHT22 <");
    MJB_DEBUG_LOG_FORMAT((unsigned long) this, MJB_DEBUG_LOG_HEX);
    MJB_DEBUG_LOG(">] Chk Data: ");
    MJB_DEBUG_LOG_FORMAT(data[4], MJB_DEBUG_LOG_BIN);
    MJB_DEBUG_LOG(" = ");
    MJB_DEBUG_LOG_LINE((unsigned short) data[4]);
#endif
    
    if (_validData(data)) // Update cached values only if valid data received.
    {
#if defined(MJB_DEBUG_LOGGING_DHT22)
        MJB_DEBUG_LOG("[DHT22 <");
        MJB_DEBUG_LOG_FORMAT((unsigned long) this, MJB_DEBUG_LOG_HEX);
        MJB_DEBUG_LOG_LINE(">] Data checks out.");
        MJB_DEBUG_LOG("[DHT22 <");
        MJB_DEBUG_LOG_FORMAT((unsigned long) this, MJB_DEBUG_LOG_HEX);
        MJB_DEBUG_LOG(">] Cached values updating to T: ");
        MJB_DEBUG_LOG(temperature);
        MJB_DEBUG_LOG(" H: ");
        MJB_DEBUG_LOG_LINE(humidity);
#endif
        _temperature = Thermometer::TemperatureUnit(temperature, Thermometer::TemperatureUnit::Scale::Celsius);
        _humidity = humidity;
    }
#if defined(MJB_DEBUG_LOGGING_DHT22)
    else
    {
        MJB_DEBUG_LOG("[DHT22 <");
        MJB_DEBUG_LOG_FORMAT((unsigned long) this, MJB_DEBUG_LOG_HEX);
        MJB_DEBUG_LOG_LINE(">] ERROR: Data is corrupted!");
    }
#endif
    
    return data;
}

bool DHT22::DHT22::_validData(Sensor::Data const &data)
{
    if (data.size() != 5) return false; // We require exactly 5 bytes of data.

    // The 5th (checksum) byte must equal to the sum of the first four bytes.
    return data[4] == static_cast<Sensor::Byte>(data[0] + data[1] + data[2] + data[3]);
}

DHT22::DHT22(Pin::Identifier const dataPin):
Thermometer({dataPin},
            DHT22_TIMEOUT,
            std::make_pair(Thermometer::TemperatureUnit(-40, Thermometer::TemperatureUnit::Scale::Celsius),
                           Thermometer::TemperatureUnit(80, Thermometer::TemperatureUnit::Scale::Celsius)))
{
    
}

DHT22::~DHT22()
{
    
}
