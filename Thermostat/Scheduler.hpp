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
#include "Development.hpp"
#include "Identifiable.hpp"

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
    class Event
    {
    public:
        
        // The following method is meant to be implemented, meaning this class
        // must be subclassed and custom code to execute at certain time placed
        // in the execute method. Once enqueued in a scheduler the current time
        // at the time of exectuion will be passed to the event.
        virtual int execute(Time const time) = 0;
        
        Time executeTime() const;
        
        // NOTE: The following method(s) have serious implications, read below:
        // This means the subclass may change its time, requiring the scheduler
        // to implement a means to update its internal event calling sequence.
        void setExecuteTime(Time const executeTime);
        
        Event(Time const executeTime = 0); // By default, immediate event.
        virtual ~Event();
        
    protected:
        
        Time _executeTime;
        
        // NOTE: The following method(s) have serious implications, read below:
        // This means that subclasses may wish to overwrite the virtual method,
        // making it possible the execution time changes without having notified
        // the scheduler with the event enqueued. At that point the scheduler
        // could potentially skip events down the queue with higher priority!
        virtual void _executeTimeReprioritize(Time const lastPriority);
    };
    
    // Unfortunately I can't use the class below the way it was designed due to
    // the fact polymorphic objects can't be downcasted due to a lack of rtti.
    // Runtime Type Information does not fit on the memory of ESP8266-03, which
    // is the module(s) I've been using to test the code with.
    // Working around it by using a super-bootlegged template I designed, fugly.
    // =========================================================================
    // Daemon: A schedulable class used to trigger repeating events.
    // =========================================================================
    class Daemon : public Event
    {
    public:
        
        virtual Time executeTimeInterval() const;
        virtual void setExecuteTimeInterval(Time const executeTimeInterval);
        
        virtual bool finished() const;
        
        Daemon(Time const executeTime = 0, Time const executeTimeInterval = 0);
        virtual ~Daemon();
        
    protected:
        
        Time _executeTimeInterval;
        
    };
    
    
    // =========================================================================
    // Delegate: A common interface used to interface with other classes.
    // =========================================================================
    class Delegate
    {
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
    
    bool enqueue(Event * const event);
    bool dequeue(Event * const event);
    
    static void UpdateInstances(Time const time);
    
    Scheduler();
    ~Scheduler();
    
protected:
    
    typedef std::set<Scheduler *> Schedules;
    typedef std::set<Event *> Events;
    
    // =========================================================================
    // Task: A wrapper for events to avoid potentially deleted memory, also
    // used to keep equal-priority elements together in a set.
    // =========================================================================
    struct Task
    {
        bool operator<(Task const &other) const;
        bool operator>(Task const &other) const;
        bool operator==(Task const &other) const;
        
        // Using [implicit] struct copy operator.
        
        // The following memeber is marked mutable, signaling the compiler the
        // member should be modifiable in const cases. In this case, it's maked
        // mutable because I've got a set of Task, and sets only return
        // const_iterator (or implicitly const iterator), and the events member
        // needs to be mutable (note, it doesn't affect Task's key/order in set).
        mutable Events events;
        
        // For the assignment operator, the value below must be modifiable.
        Time priority; // Non-const becuase it's casted as const anyway by set.
        
        Task(Task const &task);
        Task(Event * const event);
        Task(Time const priority);
    };

    // I'm using a Task set because if I were to use a map I wouldn't know which
    // priority I should stop at, and it's relevant because events must be
    // executed following their priority, so iterating over the map is impossible.
    // [Explanation: Because iterating over the map may result in mixed results.]
    typedef std::set<Task> Tasks;
    typedef std::map<Event *, Scheduler *> Associations;
    
    Tasks _tasks;
    Tasks _tasksOverflowed; // Holds Tasks to insert when time wraps around.
    
    Time _lastTime; // Last update cycle time.
    
    void _processEventsForTime(Time const priority);
    
    static bool _EnqueueTasksEvent(Tasks &tasks, Event * const event);
    static bool _DequeueTasksEvent(Tasks &tasks, Event * const event);
    
    static bool _DequeueTasksEventWithPriority(Tasks &tasks, Event * const event, Time const priority);
    
    
    // The following static member holds all instances created of Scheduler,
    // which the class uses to update by calling the static UpdateInstances
    // method once an update cycle is being executed.
    static Schedules _InstanceRegister;
    
    // The following static member associates instances of Event to instances of
    // Scheduler in constant time to update event priority in constant time.
    static Associations _AssociationRegister;
    
    static void _AssociateEventToScheduler(Event * const event, Scheduler * const scheduler = nullptr);
    static void _DissasociateEvent(Event * const event);
    
    static bool _ReprioritizeEvent(Event * const event, Time const lastPriority);
    
};

#endif /* Scheduler_hpp */

