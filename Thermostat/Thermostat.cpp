//
//  Thermostat.cpp
//  Thermostat
//
//  Created by Matias Barcenas on 11/12/16.
//  Copyright © 2016 Matias Barcenas. All rights reserved.
//

#include "Thermostat.hpp"

// =============================================================================
// Thermostat : Implementation
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

void Thermostat::setTargetTemperature(Temperature<float> const targetTemperature, Temperature<float> const targetTemperatureThreshold)
{
    _targetTemperatureThreshold = targetTemperatureThreshold;
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

void Thermostat::_standby()
{
    this->actuate({ // Toggle all pins to 0, or release all relays, immediately.
        {this->_pinout[0], {Pin::Mode::Output, 0}, 0},
        {this->_pinout[1], {Pin::Mode::Output, 0}, 0},
        {this->_pinout[2], {Pin::Mode::Output, 0}, 0}
    });
}

void Thermostat::_setCooler(bool const cool)
{
    this->actuate({ // Cool & Fan on if above target temp, off otherwise.
        {this->_pinout[0], {Pin::Mode::Output, cool}, 0},
        {this->_pinout[1], {Pin::Mode::Output, cool}, 0},
        {this->_pinout[2], {Pin::Mode::Output, 0}, 0}
    });
}

void Thermostat::_setHeater(bool const heat)
{
    this->actuate({ // Heat & Fan on if below target temp, off otherwise.
        {this->_pinout[0], {Pin::Mode::Output, heat}, 0},
        {this->_pinout[1], {Pin::Mode::Output, 0}, 0},
        {this->_pinout[2], {Pin::Mode::Output, heat}, 0}
    });
}

int Thermostat::execute(Scheduler::Time const updateTime)
{
    // Verify minimum pin count to properly operate an HVAC with relays (min 3).
    if (this->_pins.size() < 3) return 1; // Not enough control pins.
    
    // Check Actuator instance Pins are ready for operations.
    if (!this->ready()) return 2; // Pins not ready or unavailable.

    switch (this->mode())
    {
        case Off: this->_standby();
            break;
            
        case Cooling: this->_setCooler(this->temperature() > this->targetTemperature());
            break;
            
        case Heating: this->_setHeater(this->temperature() < this->targetTemperature());
            break;
            
        case Auto: {
            if (this->temperature() > (this->targetTemperature() + this->_targetTemperatureThreshold)) {
                this->_setCooler(true);
            } else
            if (this->temperature() < (this->targetTemperature() - this->_targetTemperatureThreshold)) {
                this->_setHeater(true);
            } else {
                this->_standby();
            }
        }   break;
            
        default: this->_standby(); // Fuck you too.
            break;
    }
    this->_executeTimeUpdate(updateTime);
    return 0;
}


// =============================================================================
// Thermostat : Constructors & Destructor
// =============================================================================
Thermostat::Thermostat(Actuator::Pins const &pins,
                       Thermometers &thermometers,
                       Scheduler::Time const executeTimeInterval):
Actuator(pins),
//Scheduler::Daemon(0, executeTimeInterval),
Scheduler::Event(0, executeTimeInterval),
thermometers(thermometers),
_measurmentType(Thermostat::Measurement::TemperatureUnit),
_mode(Thermostat::Mode::Off)
{
    // targetTemp, targetTempThresh & _scheduler are fine auto-initialized.
    this->_scheduler.enqueue(static_cast<Scheduler::Event * const>(this));
}

Thermostat::~Thermostat()
{

}

