//
//  DHT22.hpp
//  Thermostat
//
//  Created by Matias Barcenas on 1/15/17.
//  Copyright © 2017 Matias Barcenas. All rights reserved.
//

#ifndef DHT22_hpp
#define DHT22_hpp

#include <vector>
#include <utility>
#include <cstdint>
#include "Development.hpp"
#include "Pin.hpp"
#include "Sensor.hpp"
#include "Thermometer.hpp"
#include "Temperature.hpp"
#include "Scheduler.hpp"


#if defined(MJB_ARDUINO_LIB_API)
#include <Arduino.h>
#else
#include <iostream>
#endif

// We must wait about 2 seconds after sensing again with DHT22.
// NOTE: If called while in cooldown, cached values are returned.
// The value defined below represents the 2-second wait.
#define DHT22_TIMEOUT 2000 // In microseconds

class DHT22 : public Thermometer
{
public:

    enum Pinout
    {
        Data
    };
    
    // Sense is a blocking method since the sensor must retrive
    // information in microseconds and delays could corrupt the data.
    // NOTE: It takes approximately 5ms to retrieve the data,
    // but the specs say we must wait ~2 seconds before retrying.
    Sensor::Data sense();
    
    DHT22(Pin::Identifier const dataPin);
    virtual ~DHT22();
    
protected:
    
    bool _validData(Sensor::Data const &data);
    
};

#if ! defined(MJB_ARDUINO_LIB_API)
void delayMicroseconds(unsigned long time); // Fake test function.
#endif

#endif /* DHT22_hpp */
