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
Thermometer::TemperatureUnit Thermometer::temperature()
{
    sense(); // Attempt to get new sensory data.
    return _temperature;
}

Thermometer::TemperatureUnit Thermometer::humiture()
{
    sense(); // Attempt to get new sensory data.
    
    // Returns the heat index, aka, the "feels like" temperature.
    // Heat Index is determined by Rothfusz Steadman's equation.
    Thermometer::TemperatureUnit::value_type const t = temperature().value(Thermometer::TemperatureUnit::Scale::Fahrenheit);
    Thermometer::TemperatureUnit::value_type const h = humidity() / 100;
    Thermometer::TemperatureUnit::value_type const tt = t * t;
    Thermometer::TemperatureUnit::value_type const hh = h * h;
    Thermometer::TemperatureUnit::value_type const th = t * h;
    
    // The formula below is used as an alternative to NOAA's
    // (obtained from Wikipedia), and integrates more temperatures.
    // NOTE: ±3 degrees between 70-115F, and humidity of 0-80%
    // NOTE: [Functional] range is for temperatures below 150F.
    if ((t < 70) || (t > 115) || (h < 0) || (h > 0.80))
        return Thermometer::TemperatureUnit(temperature());
    
    return Thermometer::TemperatureUnit(0.363445176f +
                                        0.988622465f * t +
                                        4.777114035f * h +
                                       -0.114037667f * th +
                                       -0.000850208f * tt +
                                       -0.020716198f * hh +
                                        0.000687678f * t * th +
                                        0.000274954f * th * h,
                                        Thermometer::TemperatureUnit::Scale::Fahrenheit);
}

Thermometer::TemperatureUnit::value_type Thermometer::humidity()
{
    sense(); // Attempt to get new sensory data.
    return _humidity;
}

Thermometer::Range Thermometer::range() const
{
    return _range;
}

bool Thermometer::_validTemperature(Thermometer::TemperatureUnit const &temperature)
{
    return ((temperature >= range().first) && (temperature <= range().second));
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

