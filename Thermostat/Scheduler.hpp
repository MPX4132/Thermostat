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
#include <unordered_map>
#include <memory>
#include <cstdint>
#include <mutex>
#include "Development.hpp"
#include "Identifiable.hpp"
#include "Accessible.hpp"
#include "Delegable.hpp"

#if defined(MJB_ARDUINO_LIB_API)
#include <Arduino.h>
#else
#include <iostream>
#endif

class SchedulerDelegate;

// =============================================================================
// Scheduler : This class is responsible for scheduling and executing jobs at
// specific times, given some type of clock.
// =============================================================================
class Scheduler : public Accessible, public Delegable<SchedulerDelegate>
{
public:
//    typedef unsigned long Time;
    typedef uint32_t Time;

    // =========================================================================
    // Event: A schedualable class used to trigger one-time events.
    // =========================================================================
    class Event : public Accessible
    {
    public:

        // The method below must be implemented by the deriving class, defining
        // the code to be triggered at the event's run-time given by the method
        // Event::executeTime(), which is set using Event::setExeuteTime(...).
        virtual int execute(Time const time) = 0;
        
        Time executeTime() const;
        bool setExecuteTime(Time const executeTime);

        bool scheduled() const;
        std::weak_ptr<Scheduler> const &scheduler() const;
        bool setScheduler(std::weak_ptr<Scheduler> const &scheduler);

        bool schedule(std::weak_ptr<Scheduler> const &scheduler);
        bool unschedule();

        Event(Time const executeTime = 0); // Trigger event instantly by default.
        virtual ~Event();
        
    protected:

        virtual void _executeTimeDidChange(Time const executeTimeDelta);

    private:

        Time _executeTime;

        std::weak_ptr<Scheduler> _scheduler;
    };


    // Unfortunately I can't use the class below the way it was designed due to
    // the fact polymorphic objects can't be downcasted due to a lack of rtti.
    // Runtime Type Information does not fit on the memory of ESP8266-03, which
    // is the module(s) I've been using to test the code with.
    // Working around it by using a super-bootlegged template I designed; fugly.
    // =========================================================================
    // Daemon: A schedulable class used to trigger repeating events.
    // =========================================================================
    class Daemon : public Event
    {
    public:
        
        Time executeTimeInterval() const;
        void setExecuteTimeInterval(Time const executeTimeInterval);
        
        virtual bool finished() const;
        
        Daemon(Time const executeTime = 0, Time const executeTimeInterval = 0);
        virtual ~Daemon();
        
    protected:
        
        Time _executeTimeInterval;

        void _executeTimeIntervalDidChange(Time const executeTimeIntervalDelta);

    };
    
    bool enqueue(std::shared_ptr<Event> const &event);
    bool dequeue(std::shared_ptr<Event> const &event);

    bool scheduled(std::shared_ptr<Event> const &event) const;
    
    static void UpdateInstances(Time const time);
    
    Scheduler();
    virtual ~Scheduler();
    
protected:
    // =========================================================================
    // Task: A wrapper for events to avoid potentially deleted memory, also
    // used to keep equal-priority elements together in a set.
    // =========================================================================
    typedef std::unordered_set<std::shared_ptr<Event>> EventPtrSet;

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
        mutable EventPtrSet events;
        
        // For the assignment operator, the value below must be modifiable.
        Time priority; // Non-const becuase it's casted as const anyway by set.
        
        Task(Task const &task);
        Task(std::shared_ptr<Event> const &event);
        Task(Time const priority);
    };

    // I'm using a Task set because if I were to use a map I wouldn't know which
    // priority I should stop at, and it's relevant because events must be
    // executed following their priority, so iterating over the map is impossible.
    // [Explanation: Because iterating over the map may result in mixed results.]
    typedef std::set<Task> TaskSet;

    typedef std::pair<TaskSet *, TaskSet::const_iterator> EventLocation;

    static const uint8_t _TaskSetsMax = 2;

    TaskSet  _taskSets[_TaskSetsMax];
    TaskSet *_taskSetPrimary;
    TaskSet *_taskSetSecondary;

    Time _lastTime; // Last update cycle time.

    void _processEventsForTime(Time const time);
    void _processTasksForTime(TaskSet const &tasks, Time const time);
    void _processTasks(TaskSet const &tasks);

    bool _enqueueEvent(std::shared_ptr<Event> const &event, TaskSet * const taskSet = nullptr);
    bool _dequeueEvent(std::shared_ptr<Event> const &event, TaskSet * const taskSet = nullptr);

    static EventLocation _GetEventLocation(Scheduler const * const scheduler,
                                           std::shared_ptr<Scheduler::Event> const &event);


    // The following static member holds all instances created of Scheduler,
    // which the class uses to update by calling the static UpdateInstances
    // method once an update cycle is being executed.
    inline static std::set<Scheduler *> &_InstanceRegister();
#if defined(MJB_MULTITHREAD_CAPABLE)
    static std::mutex _InstanceRegisterLock;
#endif
};

// =========================================================================
// Delegate: A common interface used to interface with other classes.
// =========================================================================
class SchedulerDelegate
{
public:

    virtual void schedulerStartingEvent(Scheduler * const scheduler,
                                        std::shared_ptr<Scheduler::Event> const &event);
    virtual void schedulerCompletedEvent(Scheduler * const scheduler,
                                         std::shared_ptr<Scheduler::Event> const &event,
                                         int result);

    virtual void schedulerEnqueuedEvent(Scheduler * const scheduler,
                                        std::shared_ptr<Scheduler::Event> const &event);
    virtual void schedulerDequeuedEvent(Scheduler * const scheduler,
                                        std::shared_ptr<Scheduler::Event> const &event);

    virtual ~SchedulerDelegate();
};

#endif /* Scheduler_hpp */

