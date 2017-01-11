//
//  Actuator.hpp
//  Thermostat
//
//  Created by Matias Barcenas on 11/12/16.
//  Copyright © 2016 Matias Barcenas. All rights reserved.
//

#ifndef Actuator_hpp
#define Actuator_hpp

#include <map>
#include <vector>
#include <utility>
#include <Arduino.h>
#include "Pin.hpp"
#include "Scheduler.hpp"

// =============================================================================
// Actuator : This class abstracts the functionality of Actuators, only being
// able to send output signals via the I/O rail.
// =============================================================================
class Actuator : public Scheduler::Delegate
{
public:
    
    // This is the public interface for a Pin events, designed like this due to:
    // * [Security] Pins are encapsulated in the instance of an Actuator so that
    // other actuators cannot use foreign pins, or pins from other Actuators.
    // * [Usability] It's a struct, meaning struct literal usage is possible.
    struct Action
    {
        Pin::Identifier const pin;
        Pin::Configuration const configuration;
        Scheduler::Time const time;
        
        Action(Pin::Identifier const pin,
               Pin::Configuration const &configuration,
               Scheduler::Time const time);
    };
    
    typedef std::vector<Action> Actions;
    typedef std::vector<Pin::Identifier> Pins;
    
    virtual bool ready() const;
    virtual void actuate(Actions const &actions);
    
    Actuator(Pins const &pins);
    virtual ~Actuator();
    
protected:
    
    class Event : public Scheduler::Event {
    public:
        
        int execute(Scheduler::Time const time);
        
        Event(Pin * const pin,
              Pin::Configuration const &configuration,
              Scheduler::Time const time);
        
        ~Event();
        
    protected:
        Pin * const _pin;
        Pin::Configuration const _configuration;
    };
    
    std::map<Pin::Identifier, Pin *> _pins;
    Scheduler _scheduler;
    Pins const _pinout;
    
    virtual void schedulerCompletedEvent(Scheduler * const scheduler,
                                 Scheduler::Event * const event);
};

#endif /* Actuator_hpp */

