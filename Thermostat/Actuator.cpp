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

bool Actuator::ready() const
{
    // The loop below was used for copy pins, when copies would transfer ownership to other pins.
    /*for (Pin::Set::const_iterator iPin = this->_pins.begin(); iPin != this->_pins.end(); iPin++)
    {
        if (!iPin->second.ready()) return false;
    }*/
    
    for (std::pair<Pin::Identifier, Pin *> const &pair : this->_pins)
    {
        if (!pair.second->ready()) return false;
    }
    return true;
}

void Actuator::actuate(Actuator::Actions const &actions)
{
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

Actuator::Actuator(Pin::Arrangement const &pins):
_pins(Pin::AllocateSet(pins)),
_pinout(pins)
{
    this->_scheduler.delegate = static_cast<Scheduler::Delegate *>(this);
}

Actuator::Actuator(Actuator const &actuator):
_pins(actuator._pins),
_pinout(actuator._pinout)
{
    
}

Actuator::~Actuator()
{
    Pin::DeallocateSet(this->_pins);
}

