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
}

Thermostat::Status Thermostat::status() const
{
    return this->_status;
}

float Thermostat::humidity()
{
    float humidity = 0;
    
    for (Thermometer * const thermometer : this->thermometers)
    {
        humidity += thermometer->humidity();
    }
    
    if (this->thermometers.size() > 1) humidity /= this->thermometers.size();
    
    return humidity;
}

Thermometer::TemperatureUnit Thermostat::humiture()
{
    Thermometer::TemperatureUnit humiture = 0;
    
    for (Thermometer * const thermometer : this->thermometers)
    {
        humiture += thermometer->humiture();
    }
    
    if (this->thermometers.size() > 1) humiture /= this->thermometers.size();
    
    return humiture;
}


Thermometer::TemperatureUnit Thermostat::temperature()
{
    Thermometer::TemperatureUnit temperature = 0;
    
    for (Thermometer * const thermometer : this->thermometers)
    {
        temperature += thermometer->temperature();
    }
    
    if (this->thermometers.size() > 1) temperature /= this->thermometers.size();
    
    return temperature;
}

Thermometer::TemperatureUnit Thermostat::targetTemperature() const
{
    return this->_targetTemperature;
}

void Thermostat::setTargetTemperature(Thermometer::TemperatureUnit const targetTemperature,
                                      float const targetTemperatureThreshold)
{
    this->_targetTemperatureThreshold = targetTemperatureThreshold;
    this->_targetTemperature = targetTemperature;
}

Thermostat::PerceptionIndex Thermostat::perceptionIndex() const
{
    return this->_perceptionIndex;
}

void Thermostat::setPerceptionIndex(Thermostat::PerceptionIndex const perceptionIndex)
{
    this->_perceptionIndex = perceptionIndex;
}

int Thermostat::update(Scheduler::Time const time)
{
    // NOTE: This method will NOT change the previously scheduled update time,
    // meaning an update might occur back-to-back or fairly close to each other.
    return this->execute(time);
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

int Thermostat::execute(Scheduler::Time const updateTime)
{
    // Verify minimum pin count to properly operate an HVAC with relays (min 3).
    // Why 3? Well: Fan call pin, Cool call pin & Heat call pin.
    if (this->_pins.size() < 3) return 1; // Not enough control pins (error code 1)
    
    // Check Actuator instance Pins are ready for operations.
    if (!this->ready()) return 2; // Pins not ready or unavailable (error code 2)

    // Read this only once every update, since the sensor may need to timeout for a bit.
    // In my case, the DHT22 needs to timeout for about two seconds after a read cycle.
    Thermometer::TemperatureUnit const currentTemperature = this->perceptionIndex()?
        this->humiture() : this->temperature();
    
#if defined DEBUG && defined THERMOSTAT_LOGS
#ifdef HARDWARE_INDEPENDENT
    std::cout << "[Thermostat <" << std::hex << this
        << ">] Temperature:"
        << std::dec << this->temperature().value(Thermometer::TemperatureUnit::Scale::Fahrenheit)
        << "F, Humiture:"
        << this->humiture().value(Thermometer::TemperatureUnit::Scale::Fahrenheit)
        << "F, Humidity:"
        << this->humidity() << "%" << std::endl;
    std::cout << "[Thermostat <" << std::hex << this
        << ">] Target:"
        << std::dec << this->targetTemperature().value(Thermometer::TemperatureUnit::Scale::Fahrenheit)
        << "F, Perceivable Index:"
        << this->perceptionIndex()
        << ", Mode:" << this->mode()
        << ", Status:" << this->status()
        << " at " << updateTime << std::endl;
#else
    Serial.print("[Thermostat <");
    Serial.print((unsigned long) this, HEX);
    Serial.print(">] Temperature:");
    Serial.print(this->temperature().value(Thermometer::TemperatureUnit::Scale::Fahrenheit));
    Serial.print("F, Humiture:");
    Serial.print(this->humiture().value(Thermometer::TemperatureUnit::Scale::Fahrenheit));
    Serial.print("F, Humidity:");
    Serial.print(this->humidity());
    Serial.println("%");
    
    Serial.print("[Thermostat <");
    Serial.print((unsigned long) this, HEX);
    Serial.print(">] Target:");
    Serial.print(this->targetTemperature().value(Thermometer::TemperatureUnit::Scale::Fahrenheit));
    Serial.print("F, Measurement:");
    Serial.print(this->measurementType());
    Serial.print(", Mode:");
    Serial.print(this->mode());
    Serial.print(", Status:");
    Serial.print(this->status());
    Serial.print(" at ");
    Serial.println(updateTime);
#endif
#endif
    
    switch (this->mode())
    {
        case Off: this->_status = this->_standby();
            break;
            
        case Heat: this->_status = this->_setHeater(currentTemperature < this->targetTemperature());
            break;
            
        case Cool: this->_status = this->_setCooler(currentTemperature > this->targetTemperature());
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
            
        default: this->_status = this->_standby();
            break;
    }
    
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
_perceptionIndex(Thermostat::PerceptionIndex::TemperatureIndex),
_status(Thermostat::Status::Standby),
_mode(Thermostat::Mode::Off)
{
    // targetTemp, targetTempThresh & _scheduler are fine auto-initialized.
    this->_scheduler.enqueue(this);
}

Thermostat::~Thermostat()
{

}

