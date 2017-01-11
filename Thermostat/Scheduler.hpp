//
//  Scheduler.hpp
//  Thermostat
//
//  Created by Matias Barcenas on 11/12/16.
//  Copyright © 2016 Matias Barcenas. All rights reserved.
//

#ifndef Scheduler_hpp
#define Scheduler_hpp

#include <set>
#include <map>
#include <vector>
#include <algorithm>
#include <typeinfo>
#include <Arduino.h>

// =============================================================================
// Scheduler : This class is responsible for scheduling and executing jobs at
// specific times, given some type of clock.
// =============================================================================
class Scheduler
{
public:
    typedef unsigned long Time;
    
    // =========================================================================
    // Event: A schedualable class used to trigger one-time events.
    // =========================================================================
    class Event {
    public:
        
        struct PtrCompare {
            // Strict Weak Ordering: Returns ture if eventL < eventR
            bool operator()(Event const * const eventL, Event const * const eventR);
        };
        
        virtual int execute(Time const time) = 0;
        
        virtual bool operator==(Event const &event) const;
        virtual bool operator>(Event const &event) const;
        virtual bool operator<(Event const &event) const;
        
        // Realize the implications of this...
        // This means the subclass may change its time, requiring the scheduler
        // to implement a means to update its internal event calling sequence.
        
        virtual Time executeTime() const;
        virtual void setExecuteTime(Time const executeTime);
        
        Event(Time executeTime);
        virtual ~Event();
        
    protected:
        
        Time _executeTime;
    };
    
    // =========================================================================
    // Daemon: A schedulable class used to trigger repeating events.
    // =========================================================================
    class Daemon : public Event {
    public:
        
        virtual Time executeTimeInterval() const;
        virtual void setExecuteTimeInterval(Time const executeTimeInterval);
        
        virtual bool finished() const;
        
        Daemon(Time executeTime, Time executeTimeInterval);
        
    protected:
        Time _executeTimeInterval;
        
        Time _executeTimeUpdate(Time const updateTime);
    };
    
    // =========================================================================
    // Delegate: A common interface used to interface with other classes.
    // =========================================================================
    class Delegate {
    public:
        
        virtual void schedulerStartingEvent(Scheduler * const scheduler,
                                            Event * const event);
        virtual void schedulerCompletedEvent(Scheduler * const scheduler,
                                             Event * const event);
        
        virtual void schedulerEnqueuedEvent(Scheduler * const scheduler,
                                            Event * const event);
        virtual void schedulerDequeuedEvent(Scheduler * const scheduler,
                                            Event * const event);
        
        virtual ~Delegate();
    };
    
    Delegate * delegate;
    
    virtual bool enqueue(Event * const event);
    virtual bool dequeue(Event * const event);
    
    static void UpdateAll(Time const time);
    
    Scheduler();
    virtual ~Scheduler();
    
protected:
    
    std::set<Event *, Event::PtrCompare> _events;
    
    virtual void _update(Time const time);
    
    
    static std::set<Scheduler *> _Register;
    static std::map<Event *, Scheduler *> _EventSchedulerRegister;
    
    static void _RegisterEventOfScheduler(Event * event, Scheduler * scheduler = nullptr);
    static void _UnregisterEvent(Event * event);
    static void _RecalculateEventPriority(Event * event);
};

#endif /* Scheduler_hpp */

