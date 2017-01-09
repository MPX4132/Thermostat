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
#include "Temperature.hpp"
#include "Actuator.cpp"
#include "Sensor.hpp"

// =============================================================================
// Thermometer : This class abstracts the functionality of temperature reading.
// The thermometer class is meant to be subclassed to implement the sense method
// of the Sensor class, which should return 
// =============================================================================
class Thermometer : protected Sensor {
public:
    // Range denotes the thermometer's range as a tuple, [Minimum, Maximum].
    typedef std::pair<Temperature<float>, Temperature<float>> Range;
    
    virtual Temperature<float> temperature() = 0;
    
    Range range() const;
    
    Thermometer(Actuator::Pins const &pins);
    virtual ~Thermometer();
    
protected:
    Temperature<float> const rangeMaximum;
    Temperature<float> const rangeMinimum;
};

#endif /* Thermometer_hpp */
