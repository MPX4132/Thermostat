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
    return this->_mode;
}

void Thermostat::setMode(const Thermostat::Mode mode)
{
    _mode = mode;
    
    // To have the actuator immediately respond.
    this->_reflectUpdates();
}

Thermostat::Status Thermostat::status() const
{
    return this->_status;
}

Temperature<float> Thermostat::temperature()
{
    Temperature<float> temperature = 0;
    
    for (Thermometer *thermometer : this->thermometers)
    {
        temperature += thermometer->temperature();
    }
    
    if (this->thermometers.size()) temperature /= this->thermometers.size();
    
    return temperature;
}

Temperature<float> Thermostat::targetTemperature() const
{
    return this->_targetTemperature;
}

void Thermostat::setTargetTemperature(Temperature<float> const targetTemperature,
                                      float const targetTemperatureThreshold)
{
    this->_targetTemperatureThreshold = targetTemperatureThreshold;
    this->_targetTemperature = targetTemperature;
    
    // To have the actuator immediately respond.
    this->_reflectUpdates();
}

Thermostat::Measurement Thermostat::measurementType() const
{
    return this->_measurmentType;
}

void Thermostat::setMeasurementType(Thermostat::Measurement const measurementType)
{
    this->_measurmentType = measurementType;
    
    // To have the actuator immediately respond.
    this->_reflectUpdates();
}

Thermostat::Status Thermostat::_standby(Thermostat::Status const status)
{
    this->actuate({ // Toggle all pins to 0, or release all relays, immediately.
        {this->_pinout[0], {Pin::Mode::Output, 0}, 0},
        {this->_pinout[1], {Pin::Mode::Output, 0}, 0},
        {this->_pinout[2], {Pin::Mode::Output, 0}, 0}
    });
    
    return status;
}

Thermostat::Status Thermostat::_setCooler(bool const cool)
{
    this->actuate({ // Cool & Fan on if above target temp, off otherwise.
        {this->_pinout[0], {Pin::Mode::Output, cool}, 0},
        {this->_pinout[1], {Pin::Mode::Output, cool}, 0},
        {this->_pinout[2], {Pin::Mode::Output, 0}, 0}
    });
    return cool? Thermostat::Status::Cooling : Thermostat::Status::Standby;
}

Thermostat::Status Thermostat::_setHeater(bool const heat)
{
    this->actuate({ // Heat & Fan on if below target temp, off otherwise.
        {this->_pinout[0], {Pin::Mode::Output, heat}, 0},
        {this->_pinout[1], {Pin::Mode::Output, 0}, 0},
        {this->_pinout[2], {Pin::Mode::Output, heat}, 0}
    });
    return heat? Thermostat::Status::Heating : Thermostat::Status::Standby;
}

void Thermostat::_reflectUpdates()
{
    this->setExecuteTime(0); // Update ASAP.
}

int Thermostat::execute(Scheduler::Time const updateTime)
{
    // Verify minimum pin count to properly operate an HVAC with relays (min 3).
    // Why 3? Well: Fan call pin, Cool call pin & Heat call pin.
    if (this->_pins.size() < 3) return 1; // Not enough control pins (error code 1)
    
    // Check Actuator instance Pins are ready for operations.
    if (!this->ready()) return 2; // Pins not ready or unavailable (error code 2)

    // Read this only once every update, since the sensor may need to timeout for a bit.
    // In my case, the DHT22 needs to timeout for about two seconds after a read cycle.
    Temperature<float> const currentTemperature = this->temperature();
    
#if defined DEBUG && defined THERMOSTAT_LOGS
#ifdef HARDWARE_INDEPENDENT
    std::cout << "[Thermostat <" << std::hex << this << ">] Currently " << std::dec << currentTemperature.value(Temperature<float>::Scale::Fahrenheit) << "F at " << updateTime << ", temperature set at " << this->targetTemperature().value(Temperature<float>::Scale::Fahrenheit) << "F." << std::endl;
#else
    Serial.print("[Thermostat <");
    Serial.print((unsigned long) this, HEX);
    Serial.print(">] Currently ");
    Serial.print(currentTemperature.value(Temperature<float>::Scale::Fahrenheit));
    Serial.print("F at ");
    Serial.print(updateTime);
    Serial.print(", temperature set at ");
    Serial.print(this->targetTemperature().value(Temperature<float>::Scale::Fahrenheit));
    Serial.println("F.");
#endif
#endif
    
    switch (this->mode())
    {
        case Off: this->_status = this->_standby();
            break;
            
        case Cool: this->_status = this->_setCooler(currentTemperature > this->targetTemperature());
            break;
            
        case Heat: this->_status = this->_setHeater(currentTemperature < this->targetTemperature());
            break;
            
        case Auto: {
            if (currentTemperature > (this->targetTemperature() + this->_targetTemperatureThreshold)) {
                this->_status = this->_setCooler(true);
            } else
            if (currentTemperature < (this->targetTemperature() - this->_targetTemperatureThreshold)) {
                this->_status = this->_setHeater(true);
            } else {
                this->_status = this->_standby(Thermostat::Status::Stasis);
            }
        }   break;
            
        default: this->_status = this->_standby(); // Fuck you too.
            break;
    }
    
    // Since this is a Daemon, and Daemons repeat until finished,
    // calcualte next execution time and request scheduler priority update.
    this->_executeTimeUpdate(updateTime);
    return 0;
}


// =============================================================================
// Thermostat : Constructors & Destructor
// =============================================================================
Thermostat::Thermostat(Pin::Arrangement const &pins,
                       Thermometers const &thermometers,
                       Scheduler::Time const executeTimeInterval):
Actuator(pins),
Scheduler::Daemon(0, executeTimeInterval),
thermometers(thermometers),
_measurmentType(Thermostat::Measurement::TemperatureUnit),
_status(Thermostat::Status::Standby),
_mode(Thermostat::Mode::Off)
{
    // targetTemp, targetTempThresh & _scheduler are fine auto-initialized.
    this->_scheduler.enqueue(this);
}

Thermostat::~Thermostat()
{

}

