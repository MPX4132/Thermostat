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

Temperature<int> Thermostat::temperature(int const sensorID) const
{
    if (sensorID >= 0 && sensorID < _sensor.size()) return _sensor[sensorID];
    
    // Otherwise, we're taking the average
    Temperature<int> averageTemperature = 0;
#warning Incomplete code here!
    return averageTemperature;
}

Temperature<int> Thermostat::targetTemperature() const
{
    return this->_targetTemperature;
}

void Thermostat::setTargetTemperature(Temperature<int> const targetTemperature)
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
Thermostat::Thermostat(Scheduler::Time const executeTimeInterval):
Scheduler::Daemon(0, executeTimeInterval)
{

}

Thermostat::~Thermostat()
{

}
