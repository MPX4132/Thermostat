//
//  Actuator.cpp
//  Thermostat
//
//  Created by Matias Barcenas on 11/12/16.
//  Copyright © 2016 Matias Barcenas. All rights reserved.
//

#include "Actuator.hpp"

Actuator::Action::Action(Pin::Identifier const pin,
                         Pin::Configuration const &configuration,
                         Scheduler::Time const time):
time(time),
pin(pin),
configuration(configuration)
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
        if (!_pins.count(action.pin)) continue; // If not ours, skip the pin.
        Actuator::Event * actuatorEvent = new Actuator::Event(_pins[action.pin],
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
_pin(pin),
_configuration(configuration),
Scheduler::Event(time)
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

Actuator::Actuator(Actuator::Pins const &pins)
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
