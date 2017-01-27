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
    this->sense(); // Attempt to get new sensory data.
    return this->_temperature;
}

Temperature<float> Thermometer::humiture()
{
    this->sense(); // Attempt to get new sensory data.
    
    // Returns the heat index, aka, the "feels like" temperature
    // Heat Index is determined by Rothfusz Steadman's equation
    float const t = this->temperature().value(Temperature<float>::Scale::Fahrenheit);
    float const h = this->humidity();
    float const tt = t * t;
    float const hh = h * h;
    float const th = t * h;
    
    // The formula below is used as an alternative to the NOAA
    // (obtained from Wikipedia), and integrates more temperatures.
    // NOTE: ±3 degrees between 70-115F, and humidity of 0-80%
    // NOTE: [Functional] range is for temperatures below 150F.
    return Temperature<float>(0.363445176f +
                              0.988622465f * t +
                              4.777114035f * h +
                              -0.114037667f * th +
                              -0.000850208f * tt +
                              -0.020716198f * hh +
                              0.000687678f * t * th +
                              0.000274954f * th * h, Temperature<float>::Scale::Fahrenheit);
}

float Thermometer::humidity()
{
    this->sense(); // Attempt to get new sensory data.
    return this->_humidity;
}

Thermometer::Range Thermometer::range() const
{
    return this->_range;
}

bool Thermometer::_validTemperature(Temperature<float> const &temperature)
{
    return temperature >= this->range().first && temperature <= this->range().second;
}

Thermometer::Thermometer(Pin::Arrangement const &pins,
                         Scheduler::Time const senseTimeout,
                         Thermometer::Range const &range):
Sensor(pins, senseTimeout),
_range(range)
{
    
}

Thermometer::~Thermometer()
{
    
}

