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
#include "Development.h"

#ifdef HARDWARE_INDEPENDENT
#include <iostream>
#else
#include <Arduino.h>
#endif


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
            bool operator()(Event const * const eventL, Event const * const eventR) const;
        };
        
        // The following serve to sort the events by priority.
        virtual bool operator==(Event const &event) const;
        virtual bool operator>(Event const &event) const;
        virtual bool operator<(Event const &event) const;
        
        virtual int execute(Time const time) = 0;
        
        // Realize the implications of this...
        // This means the subclass may change its time, requiring the scheduler
        // to implement a means to update its internal event calling sequence.
        Time executeTime() const;
        void setExecuteTime(Time const executeTime);
        
        Event(Time const executeTime);
        virtual ~Event();
        
    protected:
        
        // Realize the implications of this...
        // This means that subclasses may wish to overwrite the virtual method,
        // making it possible the execution time changes without having notified
        // the scheduler with the event enqueued. At that point the scheduler
        // could potentially skip events down the queue with higher priority!
        virtual void _executeTimeReprioritize();
        
        Time _executeTime;
    };
    
    typedef std::set<Event *, Event::PtrCompare> Events;
    
    // Unfortunately I can't use the class below the way it was designed due to
    // the fact polymorphic objects can't be downcasted due to a lack of rtti.
    // Runtime Type Information does not fit on the memory of ESP8266-03, which
    // is the module(s) I've been using to test the code with.
    // =========================================================================
    // Daemon: A schedulable class used to trigger repeating events.
    // =========================================================================
    class Daemon : public Event {
    public:
        
        virtual Time executeTimeInterval() const;
        virtual void setExecuteTimeInterval(Time const executeTimeInterval);
        
        virtual bool finished() const;
        
        Daemon(Time const executeTime, Time const executeTimeInterval);
        virtual ~Daemon();
        
    protected:
        Time _executeTimeInterval;
        
        void _executeTimeReprioritize(); // Triggers Daemon priority update.
        
        Time _executeTimeUpdate(Time const updateTime);
    };
    
    typedef std::set<Daemon *, Event::PtrCompare> Daemons;
    
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
    
    // Due to a lack of runtime type information (RTTI) it's not possible to use
    // dynamic_cast/typeid, meaning we must overload the enqueue and dequeue
    // methods to support Daemon instances.
    virtual bool enqueue(Event * const event);
    virtual bool enqueue(Daemon * const daemon);
    
    virtual bool dequeue(Event * const event);
    virtual bool dequeue(Daemon * const daemon);
    
    static void UpdateInstances(Time const time);
    
    Scheduler();
    virtual ~Scheduler();
    
protected:
    
    typedef std::set<Scheduler *> Schedules;
    typedef std::map<Event *, Scheduler *> Clients;
    
    Events _events;
    Events _eventsEnqueued;
    Events _eventsDequeued;
    
    Daemons _daemons;
    Daemons _daemonsEnqueued;
    Daemons _daemonsDequeued;
    
    virtual void _update(Time const time);
    
    void _processPendingEnqueueEvents();
    void _processPendingDequeueEvents();
    
    
    static Schedules _Register;
    static Clients _EventSchedulerRegister;
    
    static void _RegisterEventOfScheduler(Event * const event, Scheduler * const scheduler = nullptr);
    static void _UnregisterEvent(Event * const event);
    
    static bool _RecalculateEventPriority(Event * const event);
    static bool _RecalculateDaemonPriority(Daemon * const event);
};

#endif /* Scheduler_hpp */

