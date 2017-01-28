//
//  Actuator.cpp
//  Thermostat
//
//  Created by Matias Barcenas on 11/12/16.
//  Copyright © 2016 Matias Barcenas. All rights reserved.
//

#include "Actuator.hpp"

// =============================================================================
// Actuator : Implementation
// =============================================================================
Actuator::Action::Action(Pin::Identifier const pin,
                         Pin::Configuration const &configuration,
                         Scheduler::Time const time):
pin(pin),
configuration(configuration),
time(time)
{
    
}

#ifdef max // To the one who made a lowercase macro: Fuck you.
#pragma push_macro("max")
#undef max
#define RESTORE_max_P1
#endif
bool Actuator::ready() const
{
    for (Pin::Set::value_type const &pair : this->_pins)
    {
        if (!pair.second->ready()) return false;
    }
    
    Scheduler::Time const currentTime = micros();
    
    // Check instance's timeout has elapsed and is now capable of actuating,
    // but we must consider a potential integer overflow from the MCU clock.
    Scheduler::Time const elapsedTime = (currentTime > this->_actuateTime)?
    (currentTime - this->_actuateTime) :
    (std::numeric_limits<Scheduler::Time>::max() - this->_actuateTime) + currentTime;
    
    return elapsedTime > this->_actuateTimeout;
}
#ifdef RESTORE_max_P1
#pragma pop_macro("max")
#endif

void Actuator::actuate(Actuator::Actions const &actions)
{
    this->_actuateTime = micros(); // Update actuation time to now.
    
    for (Actuator::Action const &action : actions) {
        // Note: Count is optimal here due to the fact _pins is a map log(n).
        if (!_pins.count(action.pin)) continue; // If not ours, skip the pin.
        Actuator::Event * actuatorEvent = new Actuator::Event(_pins[action.pin], //&_pins[action.pin],
                                                              action.configuration,
                                                              action.time);
        this->_scheduler.enqueue(static_cast<Scheduler::Event *>(actuatorEvent));
    }
}

int Actuator::Event::execute(Scheduler::Time const time)
{
    this->_pin->setConfiguration(this->_configuration);
    return 0;
}

Actuator::Event::Event(Pin * const pin,
                       Pin::Configuration const &configuration,
                       Scheduler::Time const time):
Scheduler::Event(time),
_pin(pin),
_configuration(configuration)
{
    
}

Actuator::Event::~Event()
{
    
}

void Actuator::schedulerDequeuedEvent(Scheduler * const scheduler,
                                      Scheduler::Event * const event)
{
    delete event; // Delete the dynamically created actuator event object
}

#ifdef max // To the one who made a lowercase macro: Fuck you.
#pragma push_macro("max")
#undef max
#define RESTORE_max_P2
#endif
Actuator::Actuator(Pin::Arrangement const &pins, Scheduler::Time const actuateTimeout):
_pins(Pin::AllocateSet(pins)),
_pinout(pins),
_actuateTimeout(actuateTimeout),
// The following will force the instance to be ready as soon as it's initialized.
_actuateTime(std::numeric_limits<Scheduler::Time>::max() - (actuateTimeout - 1))
{
    this->_scheduler.delegate = static_cast<Scheduler::Delegate *>(this);
}
#ifdef RESTORE_max_P2
#pragma pop_macro("max")
#endif

Actuator::Actuator(Actuator const &actuator):
_pins(actuator._pins),
_pinout(actuator._pinout),
_actuateTimeout(actuator._actuateTimeout),
_actuateTime(actuator._actuateTime)
{
    
}

Actuator::~Actuator()
{
    Pin::DeallocateSet(this->_pins);
}
