//
//  Thermometer.cpp
//  Thermostat
//
//  Created by Matias Barcenas on 1/8/17.
//  Copyright © 2017 Matias Barcenas. All rights reserved.
//

#include "Thermometer.hpp"

// =============================================================================
// Thermometer : Implementation
// =============================================================================
Temperature<float> Thermometer::temperature()
{
    // Change temperature class to store values as kelvin (neutral/scientific).
    return Temperature<float>();
}


Thermometer::Range Thermometer::range() const
{
    return std::make_pair(rangeMinimum, rangeMaximum);
}

Thermometer::Thermometer(Actuator::Pins const &pins):
Sensor(pins)
{
    
}

Thermometer::~Thermometer()
{
    
}

