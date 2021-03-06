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

Thermostat::Status Thermostat::status() const
{
    return _status;
}

Thermometer::TemperatureUnit::value_type Thermostat::humidity()
{
    Thermometer::TemperatureUnit::value_type humidity = 0;
    
    for (std::shared_ptr<Thermometer> const &thermometer : thermometers)
    {
        humidity += thermometer->humidity();
    }

    // NOTE: The following accounts for no thermometers; division by 0.
    if (thermometers.size() > 1) humidity /= thermometers.size();
    
    return humidity;
}

Thermometer::TemperatureUnit Thermostat::humiture()
{
    Thermometer::TemperatureUnit humiture = 0;
    
    for (std::shared_ptr<Thermometer> const &thermometer : thermometers)
    {
        humiture += thermometer->humiture();
    }

    // NOTE: The following accounts for no thermometers; division by 0.
    if (thermometers.size() > 1) humiture /= thermometers.size();
    
    return humiture;
}


Thermometer::TemperatureUnit Thermostat::temperature()
{
    Thermometer::TemperatureUnit temperature = 0;
    
    for (std::shared_ptr<Thermometer> const &thermometer : thermometers)
    {
        temperature += thermometer->temperature();
    }

    // NOTE: The following accounts for no thermometers; division by 0.
    if (thermometers.size() > 1) temperature /= thermometers.size();
    
    return temperature;
}

Thermometer::TemperatureUnit Thermostat::targetTemperature() const
{
    return _targetTemperature;
}

void Thermostat::setTargetTemperature(Thermometer::TemperatureUnit const &targetTemperature)
{
    _targetTemperature = targetTemperature;
}

Thermostat::TemperatureThreshold Thermostat::targetTemperatureThreshold() const
{
    return _targetTemperatureThreshold;
}

void Thermostat::setTargetTemperatureThreshold(Thermostat::TemperatureThreshold const & targetTemperatureThreshold)
{
    _targetTemperatureThreshold = targetTemperatureThreshold;
}

Thermostat::PerceptionIndex Thermostat::perceptionIndex() const
{
    return _perceptionIndex;
}

void Thermostat::setPerceptionIndex(Thermostat::PerceptionIndex const perceptionIndex)
{
    _perceptionIndex = perceptionIndex;
}

int Thermostat::update(Scheduler::Time const time)
{
    // NOTE: This method will NOT change the previously scheduled update time,
    // meaning an update might occur back-to-back or fairly close to each other.
    return execute(time);
}

Thermostat::Status Thermostat::_standby(Thermostat::Status const status)
{
    _controller.actuate({ // Toggle all pins to 0, or release all relays, immediately.
        {_controller.pinout[Thermostat::SignalLine::FanCall],  {Pin::Mode::Output, 0}, 0},
        {_controller.pinout[Thermostat::SignalLine::CoolCall], {Pin::Mode::Output, 0}, 0},
        {_controller.pinout[Thermostat::SignalLine::HeatCall], {Pin::Mode::Output, 0}, 0}
    });
    
    return status;
}

Thermostat::Status Thermostat::_setCooler(bool const cool)
{
    _controller.actuate({ // Cool & Fan on if above target temp, off otherwise.
        {_controller.pinout[Thermostat::SignalLine::FanCall],  {Pin::Mode::Output, cool}, 0},
        {_controller.pinout[Thermostat::SignalLine::CoolCall], {Pin::Mode::Output, cool}, 0},
        {_controller.pinout[Thermostat::SignalLine::HeatCall], {Pin::Mode::Output,    0}, 0}
    });
    return cool? Thermostat::Status::Cooling : Thermostat::Status::Standby;
}

Thermostat::Status Thermostat::_setHeater(bool const heat)
{
    _controller.actuate({ // Heat & Fan on if below target temp, off otherwise.
        {_controller.pinout[Thermostat::SignalLine::FanCall],  {Pin::Mode::Output, heat}, 0},
        {_controller.pinout[Thermostat::SignalLine::CoolCall], {Pin::Mode::Output,    0}, 0},
        {_controller.pinout[Thermostat::SignalLine::HeatCall], {Pin::Mode::Output, heat}, 0}
    });
    return heat? Thermostat::Status::Heating : Thermostat::Status::Standby;
}

int Thermostat::execute(Scheduler::Time const updateTime)
{
    // Verify minimum pin count to properly operate an HVAC with relays (min 3).
    // Why 3? Well: Fan call pin, Cool call pin, & Heat call pin.
    if (_controller.pinout.size() < 3) {
#if defined(MJB_DEBUG_LOGGING_THERMOSTAT)
        MJB_DEBUG_LOG("[Thermostat <");
        MJB_DEBUG_LOG_FORMAT((unsigned long) this, MJB_DEBUG_LOG_HEX);
        MJB_DEBUG_LOG_LINE(">] ERROR: Signal lines missing!");
#endif
        return Thermostat::ExecutionCode::SignalLinesMissing; // Not enough control pins!
    }
    
    // Check Actuator instance Pins are ready for operations.
    if (_controller.status() != Actuator::Status::Ready) {
#if defined(MJB_DEBUG_LOGGING_THERMOSTAT)
        MJB_DEBUG_LOG("[Thermostat <");
        MJB_DEBUG_LOG_FORMAT((unsigned long) this, MJB_DEBUG_LOG_HEX);
        MJB_DEBUG_LOG_LINE(">] WARNING: Signal lines not ready!");
#endif
        return Thermostat::ExecutionCode::SignalLinesNotReady; // Pins not ready or unavailable!
    }

    // Read this only once every update, since the sensor may need to timeout for a bit.
    // In my case, the DHT22 needs to timeout for about two seconds after a read cycle.
    Thermometer::TemperatureUnit const currentTemperature = perceptionIndex()? humiture() : temperature();

    const Thermometer::TemperatureUnit::value_type temperatureThreshold = _targetTemperatureThreshold.first;
    const Thermometer::TemperatureUnit::Scale temperatureThresholdScale = _targetTemperatureThreshold.second;

    switch (mode())
    {
        case Off: _status = _standby();
            break;
            
        case Heat: _status = _setHeater(currentTemperature.value(temperatureThresholdScale) <
                                        (targetTemperature().value(temperatureThresholdScale) + temperatureThreshold));
            break;
            
        case Cool: _status = _setCooler(currentTemperature.value(temperatureThresholdScale) >
                                        (targetTemperature().value(temperatureThresholdScale) - temperatureThreshold));
            break;
            
        case Auto: {
            if (currentTemperature.value(temperatureThresholdScale) <
                (targetTemperature().value(temperatureThresholdScale) - temperatureThreshold)) {
                _status = _setHeater(true);
            } else
            if (currentTemperature.value(temperatureThresholdScale) >
                (targetTemperature().value(temperatureThresholdScale) + temperatureThreshold)) {
                _status = _setCooler(true);
            } else {
                _status = _standby(Thermostat::Status::Stasis);
            }
        }   break;
            
        default: _status = _standby();
            break;
    }

#if defined(MJB_DEBUG_LOGGING_THERMOSTAT)
    MJB_DEBUG_LOG("[Thermostat <");
    MJB_DEBUG_LOG_FORMAT((unsigned long) this, MJB_DEBUG_LOG_HEX);
    MJB_DEBUG_LOG(">] Temperature:");
    MJB_DEBUG_LOG_FORMAT(temperature().value(Thermometer::TemperatureUnit::Scale::Fahrenheit), MJB_DEBUG_LOG_DEC);
    MJB_DEBUG_LOG("F, Humiture:");
    MJB_DEBUG_LOG(humiture().value(Thermometer::TemperatureUnit::Scale::Fahrenheit));
    MJB_DEBUG_LOG("F, Humidity:");
    MJB_DEBUG_LOG(humidity());
    MJB_DEBUG_LOG("% Target: ");
    MJB_DEBUG_LOG_FORMAT(targetTemperature().value(Thermometer::TemperatureUnit::Scale::Fahrenheit), MJB_DEBUG_LOG_DEC);
    MJB_DEBUG_LOG_LINE("F");

    MJB_DEBUG_LOG("[Thermostat <");
    MJB_DEBUG_LOG_FORMAT((unsigned long) this, MJB_DEBUG_LOG_HEX);
    MJB_DEBUG_LOG(">] Threshold:");
    MJB_DEBUG_LOG_FORMAT(targetTemperatureThreshold().first, MJB_DEBUG_LOG_DEC);
    MJB_DEBUG_LOG(((targetTemperatureThreshold().second == Thermometer::TemperatureUnit::Scale::Fahrenheit)? "F" :
                   ((targetTemperatureThreshold().second == Thermometer::TemperatureUnit::Scale::Celsius)?  "C" :  "K")));
    MJB_DEBUG_LOG(", Perceivable Index:");
    MJB_DEBUG_LOG(perceptionIndex());
    MJB_DEBUG_LOG(", Mode:");
    MJB_DEBUG_LOG(mode());
    MJB_DEBUG_LOG(", Status:");
    MJB_DEBUG_LOG(status());
    MJB_DEBUG_LOG(" at ");
    MJB_DEBUG_LOG_LINE(updateTime);
#endif
    
    return Thermostat::ExecutionCode::Success;
}


// =============================================================================
// Thermostat : Constructors & Destructor
// =============================================================================
Thermostat::Thermostat(Pin::Arrangement const &pins,
                       Thermometers const &thermometers,
                       Scheduler::Time const executeTimeInterval):
Scheduler::Daemon(0, executeTimeInterval),
thermometers(thermometers),
_targetTemperatureThreshold(std::make_pair(1, Thermometer::TemperatureUnit::Scale::Fahrenheit)),
_perceptionIndex(Thermostat::PerceptionIndex::TemperatureIndex),
_status(Thermostat::Status::Standby),
_mode(Thermostat::Mode::Off),
_controller(pins)
{
    // targetTemp, targetTempThresh & _scheduler are fine auto-initialized.
    _scheduler.enqueue(std::static_pointer_cast<Scheduler::Event>(Scheduler::Event::self()));
}

Thermostat::~Thermostat()
{

}

