//
//  Thermometer.hpp
//  Thermostat
//
//  Created by Matias Barcenas on 1/8/17.
//  Copyright © 2017 Matias Barcenas. All rights reserved.
//

#ifndef Thermometer_hpp
#define Thermometer_hpp

#include <utility>
#include "Development.hpp"
#include "Temperature.hpp"
#include "Actuator.hpp"
#include "Sensor.hpp"

// =============================================================================
// Thermometer : This class abstracts the functionality of temperature reading.
// The thermometer class is meant to be subclassed to implement the sense method
// of the Sensor class, which generate/get temperature info and return that.
// =============================================================================
class Thermometer : protected Sensor {
public:
    
    // Range denotes the thermometer's range as a tuple, [Minimum, Maximum].
    typedef std::pair<Temperature<float>, Temperature<float>> Range;
    
    // NOTE: The methods below call Sensor's sense() method to update values,
    // which are then returned by default. However, these can be overwritten.
    virtual Temperature<float> temperature();
    virtual Temperature<float> humiture(); // AKA, Heat Index.
    virtual float humidity();
    
    virtual Range range() const;
    
    Thermometer(Pin::Arrangement const &pins,
                Scheduler::Time const senseTimeout = 0,
                Range const &range = std::make_pair(Temperature<float>(),
                                                    Temperature<float>()));
    
    virtual ~Thermometer();
    
    
protected:
    
    // The following serve for two purposes:
    // 1. To have a common method which will update the values below and not be
    //    forced to have to implement the temperature() and humidity() methods.
    // 2. To cache for values to retrival if the sensor needs to timeout.
    Temperature<float> _temperature;
    Range const _range;
    float _humidity;

    bool _validTemperature(Temperature<float> const &temperature);
    
};

#endif /* Thermometer_hpp */

