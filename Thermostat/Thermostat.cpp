//
//  Thermostat.cpp
//  Thermostat
//
//  Created by Matias Barcenas on 11/12/16.
//  Copyright © 2016 Matias Barcenas. All rights reserved.
//

#include "Thermostat.hpp"

// =============================================================================
#pragma mark - Thermostat : Implementation
// =============================================================================
Thermostat::Mode Thermostat::mode() const
{
    return _mode;
}

void Thermostat::setMode(const Thermostat::Mode mode)
{
    _mode = mode;
}

Temperature<float> Thermostat::temperature()
{
    Temperature<float> temperature = 0;
    
    for (Thermometer &thermometer : this->thermometers)
    {
        temperature += thermometer.temperature();
    }
    
    if (this->thermometers.size()) temperature /= this->thermometers.size();
    return temperature;
}

Temperature<float> Thermostat::targetTemperature() const
{
    return this->_targetTemperature;
}

void Thermostat::setTargetTemperature(Temperature<float> const targetTemperature)
{
    _targetTemperature = targetTemperature;
}

Thermostat::Measurement Thermostat::measurementType() const
{
    return this->_measurmentType;
}

void Thermostat::setMeasurementType(Thermostat::Measurement const measurementType)
{
    this->_measurmentType = measurementType;
}

int Thermostat::execute(Scheduler::Time const updateTime,
                        Scheduler::Time const updateDelay)
{
    switch (this->mode())
    {
        case Off:
            break;
        case Cooling:
            break;
        case Heating:
            break;
        case Auto:
            break;
        default:
            break;
    }
    
    return 0;
}


// =============================================================================
#pragma mark - Thermostat : Constructors & Destructor
// =============================================================================
Thermostat::Thermostat(Thermometers &thermometers,
                       Scheduler::Time const executeTimeInterval):
thermometers(thermometers),
_mode(Thermostat::Mode::Off),
_measurmentType(Thermostat::Measurement::TemperatureUnit),
Scheduler::Daemon(0, executeTimeInterval)
{
    // targetTemp & _scheduler are fine auto-initialized.
}

Thermostat::~Thermostat()
{

}
