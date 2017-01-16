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
    this->sense();
    return this->_temperature;
}

float Thermometer::humidity()
{
    this->sense();
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
                         Thermometer::Range const &range):
Sensor(pins),
_range(range)
{
    
}

Thermometer::~Thermometer()
{
    
}

