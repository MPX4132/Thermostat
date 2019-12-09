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
Actuator::Status Actuator::status() const
{
    for (Pin::Set::value_type const &pair : _pins)
    {
        // Check if pin isn't yet ready to actuate.
        if (!pair.second->ready())
        {
#if defined(MJB_DEBUG_LOGGING_ACTUATOR)
            MJB_DEBUG_LOG("[Actuator <");
            MJB_DEBUG_LOG_FORMAT((unsigned long) this, MJB_DEBUG_LOG_HEX);
            MJB_DEBUG_LOG(">] NOTICE: Waiting on pin ");
            MJB_DEBUG_LOG_LINE_FORMAT(pair.second->identity(), MJB_DEBUG_LOG_DEC);
#endif
            return Actuator::Status::WaitingOnPins;
        }
    }
    
    Scheduler::Time const currentTime = micros();
    
    // Check instance's timeout has elapsed and is now capable of actuating,
    // but we must consider a potential integer overflow from the MCU clock.
    Scheduler::Time const elapsedTime = (currentTime > _actuateTime)?
        (currentTime - _actuateTime) :
        ((std::numeric_limits<Scheduler::Time>::max() - _actuateTime) + currentTime);

    // If the actuator time-out/cool-down time has passed, it's ready to actuate.
    if (elapsedTime <= _actuateTimeout)
    {
#if defined(MJB_DEBUG_LOGGING_ACTUATOR)
        MJB_DEBUG_LOG("[Actuator <");
        MJB_DEBUG_LOG_FORMAT((unsigned long) this, MJB_DEBUG_LOG_HEX);
        MJB_DEBUG_LOG(">] NOTICE: Waiting on timeout; remaining: ");
        MJB_DEBUG_LOG_LINE_FORMAT((_actuateTime - elapsedTime), MJB_DEBUG_LOG_DEC);
#endif
        return Actuator::Status::WaitingOnTimeout;
    }

    return Actuator::Status::Ready;
}
#ifdef RESTORE_max_P1
#pragma pop_macro("max")
#endif

void Actuator::actuate(Actuator::Actions const &actions)
{
    _actuateTime = micros(); // Update actuation time to now.
    
    for (Actuator::Action const &action : actions) {
        // Note: Count is optimal here due to the fact _pins is a map log(n).
        // TODO: Consider throwing exception below if triggering foreign pin.
        if (!_pins.count(action.pin)) continue; // If not ours, skip the pin.
        std::shared_ptr<Actuator::Event> actuatorEvent = std::make_shared<Actuator::Event>(_pins[action.pin], action.configuration, action.time);
        _scheduler.enqueue(std::static_pointer_cast<Scheduler::Event>(actuatorEvent));
    }
}

int Actuator::Event::execute(Scheduler::Time const time)
{
    // The following done to suppress unused variable warnings.
    (void) time;
    _pin->setConfiguration(_configuration);
    return 0;
}

Actuator::Event::Event(std::shared_ptr<Pin> const &pin,
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


#ifdef max // To the one who made a lowercase macro: Fuck you.
#pragma push_macro("max")
#undef max
#define RESTORE_max_P2
#endif
Actuator::Actuator(Pin::Arrangement const &pins, Scheduler::Time const actuateTimeout):
pinout(pins),
_pins(Pin::MakeSet(pins)),
_actuateTimeout(actuateTimeout),
// The following will force the instance to be ready as soon as it's initialized.
_actuateTime(std::numeric_limits<Scheduler::Time>::max() - (actuateTimeout - 1))
{
    _scheduler.addDelegate(std::static_pointer_cast<SchedulerDelegate>(std::static_pointer_cast<Actuator>(self())));
}
#ifdef RESTORE_max_P2
#pragma pop_macro("max")
#endif

Actuator::Actuator(Actuator const &actuator):
pinout(actuator.pinout),
_pins(actuator._pins),
_actuateTimeout(actuator._actuateTimeout),
_actuateTime(actuator._actuateTime)
{
    
}

Actuator::~Actuator()
{
    
}
