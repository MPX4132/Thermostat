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
#include "Development.hpp"
#include "Pin.hpp"
#include "Actuator.hpp"
#include "Sensor.hpp"
#include "Thermometer.hpp"
#include "Temperature.hpp"
#include "Scheduler.hpp"


#ifdef HARDWARE_INDEPENDENT
#include <iostream>
#else
#include <Arduino.h>
#endif

// We must wait about 2 seconds after sensing again with DHT22.
// NOTE: If called while in cooldown, cached values are returned.
// The value defined below represents the 2-second wait.
#define DHT22_TIMEOUT 2000000 // In microseconds

class DHT22 : public Thermometer
{
public:
    
    // Sense is a blocking method since the sensor must retrive
    // information in microseconds and delays could corrupt the data.
    // NOTE: It takes approximately 5ms to retrieve the data,
    // but the specs say we must wait ~2 seconds before retrying.
    Sensor::Data sense();
    
    DHT22(Pin::Identifier const pin);
    virtual ~DHT22();
    
protected:
    
    bool _validData(Sensor::Data const &data);
    
};

#ifdef HARDWARE_INDEPENDENT
void delayMicroseconds(unsigned long time); // Fake test function.
#endif

#endif /* DHT22_hpp */
