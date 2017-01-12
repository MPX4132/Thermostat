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
    for (std::pair<Pin::Identifier, Pin *> const &pair : this->_pins) {
        if (!pair.second->ready()) return false;
    }
    return true;
}

void Actuator::actuate(Actuator::Actions const &actions)
{
    for (Actuator::Action const &action : actions) {
        // Note: Count is optimal here due to the fact _pins is a map log(n).
        if (!_pins.count(action.pin)) continue; // If not ours, skip the pin.
        Actuator::Event * actuatorEvent = new Actuator::Event(_pins[action.pin],
                                                              action.configuration,
                                                              action.time);
        this->_scheduler.enqueue(actuatorEvent);
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

void Actuator::schedulerCompletedEvent(Scheduler * const scheduler,
                                       Scheduler::Event * const event)
{
    delete event; // Delete the dynamically created actuator event object
}

Actuator::Actuator(Actuator::Pins const &pins):
_pinout(pins)
{
    this->_scheduler.delegate = static_cast<Scheduler::Delegate *>(this);
    
    for (Pin::Identifier const &pin : pins) {
        this->_pins[pin] = new Pin(pin);
    }
}

Actuator::~Actuator()
{
    for (std::pair<Pin::Identifier, Pin *> const &pair : this->_pins) {
        if (pair.second) delete pair.second; // Delete the pin object
    }
}

