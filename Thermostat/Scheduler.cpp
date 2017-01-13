//
//  Scheduler.cpp
//  Thermostat
//
//  Created by Matias Barcenas on 11/12/16.
//  Copyright © 2016 Matias Barcenas. All rights reserved.
//

#include "Scheduler.hpp"

// =============================================================================
// Scheduler : Static Variables Declaration
// =============================================================================
std::set<Scheduler *> Scheduler::_Register;
std::map<Scheduler::Event *, Scheduler *> Scheduler::_EventSchedulerRegister;


// =============================================================================
// Scheduler::Event : Implementation
// =============================================================================
bool Scheduler::Event::PtrCompare::operator()(Scheduler::Event const * const eventL,
                                              Scheduler::Event const * const eventR) const
{
    return (eventL->executeTime() == eventR->executeTime())? (eventL < eventR) : (*eventL) < (*eventR);
}

bool Scheduler::Event::operator==(Scheduler::Event const &event) const
{
    return this->executeTime() == event.executeTime();
}

bool Scheduler::Event::operator<(Scheduler::Event const &event) const
{
    return this->executeTime() < event.executeTime();
}

bool Scheduler::Event::operator>(Scheduler::Event const &event) const
{
    return this->executeTime() > event.executeTime();
}

Scheduler::Time Scheduler::Event::executeTime() const
{
    return this->_executeTime;
}

void Scheduler::Event::setExecuteTime(Scheduler::Time const executeTime)
{
    Time const lastExecuteTime = this->executeTime();
    this->_executeTime = executeTime;
    if (lastExecuteTime != this->executeTime()) this->_executeTimeReprioritize();
}

void Scheduler::Event::_executeTimeReprioritize()
{
    Scheduler::_RecalculateEventPriority(this);
}

Scheduler::Event::Event(Scheduler::Time const executeTime):
_executeTime(executeTime)
{
    Scheduler::_RegisterEventOfScheduler(this);
}

Scheduler::Event::~Event()
{
    Scheduler::_UnregisterEvent(this);
}


// =============================================================================
// Scheduler::Daemon : Implementation
// =============================================================================
Scheduler::Time Scheduler::Daemon::executeTimeInterval() const
{
    return this->_executeTimeInterval;
}

void Scheduler::Daemon::setExecuteTimeInterval(Scheduler::Time const executeTimeInterval)
{
    this->_executeTimeInterval = executeTimeInterval;
}

bool Scheduler::Daemon::finished() const
{
    return false;
}

void Scheduler::Daemon::_executeTimeReprioritize()
{
    Scheduler::_RecalculateDaemonPriority(this); // Daemon priority update.
}

Scheduler::Time Scheduler::Daemon::_executeTimeUpdate(Scheduler::Time const updateTime)
{
    this->setExecuteTime(updateTime + this->executeTimeInterval());
    return this->executeTime();
}

Scheduler::Daemon::Daemon(Scheduler::Time const executeTime,
                          Scheduler::Time const executeTimeInterval):
Event(executeTime),
_executeTimeInterval(executeTimeInterval)
{
    
}

Scheduler::Daemon::~Daemon()
{
    
}


// =============================================================================
// Scheduler::Delegate : Implementation
// =============================================================================
void Scheduler::Delegate::schedulerStartingEvent(Scheduler * const scheduler,
                                                 Scheduler::Event * const event)
{
    return; // By default, skip.
}

void Scheduler::Delegate::schedulerCompletedEvent(Scheduler * const scheduler,
                                                  Scheduler::Event * const event)
{
    return; // By default, skip.
}

void Scheduler::Delegate::schedulerEnqueuedEvent(Scheduler * const scheduler,
                                                 Scheduler::Event * const event)
{
    return; // By default, skip.
}

void Scheduler::Delegate::schedulerDequeuedEvent(Scheduler * const scheduler,
                                                 Scheduler::Event * const event)
{
    return; // By default, skip.
}

Scheduler::Delegate::~Delegate()
{
    
}


// =============================================================================
// Scheduler : Implementation
// =============================================================================
bool Scheduler::enqueue(Scheduler::Event * const event)
{
    if (!event) return false;
    
    this->_eventsEnqueued.insert(event);
    
#warning This may need to be done somewhere else.
    if (this->delegate) this->delegate->schedulerEnqueuedEvent(this, event);

    
#if defined DEBUG && defined SCHEDULER_LOGS
#ifdef HARDWARE_INDEPENDENT
    std::cout << "[Scheduler <" << std::hex << this << ">] Event <" << std::hex << event << "> is pending enqueue (" << this->_eventsEnqueued.size() << " pending Event(s))." << std::endl;
#else
    Serial.print("[Scheduler <");
    Serial.print((unsigned long) this, HEX);
    Serial.print(">] Event <");
    Serial.print((unsigned long) event, HEX);
    Serial.print("> is pending enqueue (");
    Serial.print(this->_events.size());
    Serial.println(" pending Event(s)).");
#endif
#endif
    return true;
}

bool Scheduler::enqueue(Scheduler::Daemon * const daemon)
{
    if (!daemon) return false;
    
    this->_daemonsEnqueued.insert(daemon);

#warning This may need to be done somewhere else.
    if (this->delegate) this->delegate->schedulerEnqueuedEvent(this, static_cast<Event *>(daemon));
    
#if defined DEBUG && defined SCHEDULER_LOGS
#ifdef HARDWARE_INDEPENDENT
    std::cout << "[Scheduler <" << std::hex << this << ">] Daemon <" << std::hex << daemon << "> is pending enqueue (" << this->_daemonsEnqueued.size() << " pending Daemon(s))." << std::endl;
#else
    Serial.print("[Scheduler <");
    Serial.print((unsigned long) this, HEX);
    Serial.print(">] Daemon <");
    Serial.print((unsigned long) daemon, HEX);
    Serial.print("> is pending enqueue (");
    Serial.print(this->_events.size());
    Serial.println(" pending Daemon(s)).");
#endif
#endif
    return true;
}

bool Scheduler::dequeue(Scheduler::Event * const event)
{
#if defined DEBUG && defined SCHEDULER_LOGS
#ifdef HARDWARE_INDEPENDENT
    std::cout << "[Scheduler <" << std::hex << this << ">] Event <" << std::hex << event << "> pending dequeue." << std::endl;
#else
    Serial.print("[Scheduler <");
    Serial.print((unsigned long) this, HEX);
    Serial.print(">] Event <");
    Serial.print((unsigned long) event);
    Serial.print("> pending dequeue.");
#endif
#endif
    if (this->_events.count(event))
    {
        this->_eventsDequeued.insert(event);
        return true;
    }
    
    if (this->_eventsEnqueued.count(event))
    {
        this->_eventsEnqueued.insert(event);
        return true;
    }
    
    return false;
}

bool Scheduler::dequeue(Scheduler::Daemon * const daemon)
{
#if defined DEBUG && defined SCHEDULER_LOGS
#ifdef HARDWARE_INDEPENDENT
    std::cout << "[Scheduler <" << std::hex << this << ">] Daemon <" << std::hex << daemon << "> pending dequeue." << std::endl;
#else
    Serial.print("[Scheduler <");
    Serial.print((unsigned long) this, HEX);
    Serial.print(">] Daemon <");
    Serial.print((unsigned long) daemon);
    Serial.print("> pending dequeue.");
#endif
#endif
    if (this->_daemons.count(daemon))
    {
        this->_daemonsDequeued.insert(daemon);
        if (this->delegate) this->delegate->schedulerDequeuedEvent(this, static_cast<Event *>(daemon));
        return true;
    }
    
    if (this->_daemonsEnqueued.count(daemon))
    {
        if (this->delegate) this->delegate->schedulerDequeuedEvent(this, static_cast<Event *>(daemon));
        this->_daemonsEnqueued.insert(daemon);
        return true;
    }
    
    return false;
}

void Scheduler::UpdateInstances(Scheduler::Time const time)
{
#if defined DEBUG && defined SCHEDULER_LOGS
#ifdef HARDWARE_INDEPENDENT
    std::cout << std::endl << "==============" << std::endl;
#else
    Serial.println("");
    Serial.println("==============");
#endif
#endif
    
    for (Scheduler * const scheduler : Scheduler::_Register)
    {
#if defined DEBUG && defined SCHEDULER_LOGS
#ifdef HARDWARE_INDEPENDENT
        std::cout << "[Scheduler <" << std::hex << scheduler << "> IS STARTING]" << std::endl;
        std::cout << "[Scheduler <" << std::hex << scheduler << ">] Holding " << scheduler->_events.size() << " scheduled Event(s)." << std::endl;
        std::cout << "[Scheduler <" << std::hex << scheduler << ">] Holding " << scheduler->_eventsDequeued.size() << " Event(s) for dequeue." << std::endl;
        std::cout << "[Scheduler <" << std::hex << scheduler << ">] Holding " << scheduler->_eventsEnqueued.size() << " Event(s) for enqueue." << std::endl;
        std::cout << "[Scheduler <" << std::hex << scheduler << ">] Holding " << scheduler->_daemons.size() << " scheduled Daemon(s)." << std::endl;
        std::cout << "[Scheduler <" << std::hex << scheduler << ">] Holding " << scheduler->_daemonsDequeued.size() << " Daemon(s) for dequeue." << std::endl;
        std::cout << "[Scheduler <" << std::hex << scheduler << ">] Holding " << scheduler->_daemonsEnqueued.size() << " Daemon(s) for enqueue." << std::endl;
#else
        Serial.print("[Scheduler <");
        Serial.print((unsigned long) scheduler, HEX);
        Serial.println(">] Starting.");
        Serial.print("[Scheduler <");
        Serial.print((unsigned long) scheduler, HEX);
        Serial.print(">] Holding ");
        Serial.print(scheduler->_events.size());
        Serial.println(" scheduled Event(s).");
        Serial.print("[Scheduler <");
        Serial.print((unsigned long) scheduler, HEX);
        Serial.print(">] Holding ");
        Serial.print(scheduler->_eventsDequeued.size());
        Serial.println(" Event(s) for dequeue.");
        Serial.print("[Scheduler <");
        Serial.print((unsigned long) scheduler, HEX);
        Serial.print(">] Holding ");
        Serial.print(scheduler->_eventsEnqueued.size());
        Serial.println(" Event(s) for enqueue.");
        
        Serial.print("[Scheduler <");
        Serial.print((unsigned long) scheduler, HEX);
        Serial.print(">] Holding ");
        Serial.print(scheduler->_daemons.size());
        Serial.println(" scheduled Daemon(s).");
        Serial.print("[Scheduler <");
        Serial.print((unsigned long) scheduler, HEX);
        Serial.print(">] Holding ");
        Serial.print(scheduler->_eventsDequeued.size());
        Serial.println(" Event(s) for dequeue.");
        Serial.print("[Scheduler <");
        Serial.print((unsigned long) scheduler, HEX);
        Serial.print(">] Holding ");
        Serial.print(scheduler->_eventsEnqueued.size());
        Serial.println(" Daemon(s) for enqueue.");
#endif
#endif
        scheduler->_update(time);
        
#if defined DEBUG && defined SCHEDULER_LOGS
#ifdef HARDWARE_INDEPENDENT
        std::cout << "[Scheduler <" << std::hex << scheduler << ">  IS PAUSING]" << std::endl;
#else
        Serial.print("[Scheduler <");
        Serial.print((unsigned long) scheduler, HEX);
        Serial.println(">  IS PAUSING]");
#endif
#endif
    }
    
#if defined DEBUG && defined SCHEDULER_LOGS
#ifdef HARDWARE_INDEPENDENT
    std::cout << "==============" << std::endl << std::endl;
#else
    Serial.println("==============");
    Serial.println("");
#endif
#endif
}

void Scheduler::_update(Scheduler::Time const time)
{
    for (Scheduler::Event * const event : this->_events)
    {
        if (event->executeTime() > time) break;
        if (this->delegate) this->delegate->schedulerStartingEvent(this, event);
        
#if defined DEBUG && defined SCHEDULER_LOGS
#ifdef HARDWARE_INDEPENDENT
        std::cout << std::endl << "==========" << std::endl;
        std::cout << "[Scheduler <" << std::hex << this << ">] Event <"
        << std::hex << event << "> running." << std::endl;
#else
        Serial.println("");
        Serial.println("==========");
        Serial.print("[Scheduler <");
        Serial.print((unsigned long)this, HEX);
        Serial.print(">] Event <");
        Serial.print((unsigned long)event, HEX);
        Serial.println("> running.");
#endif
#endif
        int const error = event->execute(time);
        
        if (error)
        {
#if defined DEBUG && defined SCHEDULER_LOGS
#ifdef HARDWARE_INDEPENDENT
            std::cout << "Event returned error code " << error << std::endl;
#else
            Serial.print("Event returned error code ");
            Serial.println(error);
#endif
#endif
        }
        
#if defined DEBUG && defined SCHEDULER_LOGS
#ifdef HARDWARE_INDEPENDENT
        std::cout << "[Scheduler <" << std::hex << this << ">] Event <"
        << std::hex << event << "> halting." << std::endl;
        std::cout << "==========" << std::endl << std::endl;
#else
        Serial.print("[Scheduler <");
        Serial.print((unsigned long) this, HEX);
        Serial.print(">] Event <");
        Serial.print((unsigned long) event, HEX);
        Serial.println("> halting.");
        Serial.println("==========");
        Serial.println("");
#endif
#endif
        
        if (this->delegate) this->delegate->schedulerCompletedEvent(this, event);
        this->dequeue(event);
    }
    
    
    for (Scheduler::Daemon * const daemon : this->_daemons)
    {
        if (daemon->executeTime() > time) break;
        if (this->delegate) this->delegate->schedulerStartingEvent(this, static_cast<Event *>(daemon));
        
#if defined DEBUG && defined SCHEDULER_LOGS
#ifdef HARDWARE_INDEPENDENT
        std::cout << std::endl << "==========" << std::endl;
        std::cout << "[Scheduler <" << std::hex << this << ">] Daemon <"
        << std::hex << daemon << "> running." << std::endl;
#else
        Serial.println("");
        Serial.println("==========");
        Serial.print("[Scheduler <");
        Serial.print((unsigned long)this, HEX);
        Serial.print(">] Daemon <");
        Serial.print((unsigned long)daemon, HEX);
        Serial.println("> running.");
#endif
#endif
        int const error = daemon->execute(time);
        
        if (error)
        {
#if defined DEBUG && defined SCHEDULER_LOGS
#ifdef HARDWARE_INDEPENDENT
            std::cout << "Event returned error code " << error << std::endl;
#else
            Serial.print("Event returned error code ");
            Serial.println(error);
#endif
#endif
        }
        
#if defined DEBUG && defined SCHEDULER_LOGS
#ifdef HARDWARE_INDEPENDENT
        std::cout << "[Scheduler <" << std::hex << this << ">] Daemon <"
        << std::hex << daemon << "> halting." << std::endl;
        std::cout << "==========" << std::endl << std::endl;
#else
        Serial.print("[Scheduler <");
        Serial.print((unsigned long) this, HEX);
        Serial.print(">] Daemon <");
        Serial.print((unsigned long) daemon, HEX);
        Serial.println("> halting.");
        Serial.println("==========");
        Serial.println("");
#endif
#endif
        if (this->delegate) this->delegate->schedulerCompletedEvent(this, static_cast<Event *>(daemon));
        
        if (daemon->finished()) this->dequeue(daemon);
    }
    
    this->_processPendingDequeueEvents();
    this->_processPendingEnqueueEvents();
}

void Scheduler::_processPendingEnqueueEvents()
{
    // Enqueue any pending Events. It's safe at this point since there's
    // nothing modifying _eventsEnqueued at this point.
    // Enqueued Event has already been called in the enqueue method.
    for (Scheduler::Event * const event : this->_eventsEnqueued)
    {
        this->_events.insert(event);
#if defined DEBUG && defined SCHEDULER_LOGS
#ifdef HARDWARE_INDEPENDENT
        std::cout << "[Scheduler] Event <" << std::hex
        << event << "> enqueued." << std::endl;
#else
        Serial.print("[Scheduler] Event <");
        Serial.print((unsigned long) event);
        Serial.println("> enqueued.");
#endif
#endif
        // Associate enqueueing Event to the scheduler instance on registry.
        Scheduler::_RegisterEventOfScheduler(event, this);
    }
    
    this->_eventsEnqueued.clear(); // Reset enqueued Events holder
    
    // Enqueue any pending Daemons. It's safe at this point since there's
    // nothing modifying _daemonsEnqueued at this point.
    // Enqueued Daemon has already been called in the enqueue method.
    for (Scheduler::Daemon * const daemon : this->_daemonsEnqueued)
    {
        this->_daemons.insert(daemon);
#if defined DEBUG && defined SCHEDULER_LOGS
#ifdef HARDWARE_INDEPENDENT
        std::cout << "[Scheduler] Daemon <" << std::hex
        << daemon << "> enqueued." << std::endl;
#else
        Serial.print("[Scheduler] Daemon <");
        Serial.print((unsigned long) daemon);
        Serial.println("> enqueued.");
#endif
#endif
        // Associate enqueueing Event to the scheduler instance on registry.
        Scheduler::_RegisterEventOfScheduler(static_cast<Event *>(daemon), this);
    }
    
    this->_daemonsEnqueued.clear(); // Reset enqueued Daemons holder
}

void Scheduler::_processPendingDequeueEvents()
{
    // Dequeue any pending events at this point to keep _events set
    // as small as possible when inserting, for better performance.
    for (Scheduler::Event * const event : this->_eventsDequeued)
    {
        Scheduler::Events::const_iterator target = this->_events.find(event);
        
        // Attempt to find the event in the _events set (usual).
        if (target != this->_events.end())
        {
            // Any Event based object is unregistered Event destructor.
            this->_events.erase(target);
            if (this->delegate) this->delegate->schedulerDequeuedEvent(this, event);
            
#if defined DEBUG && defined SCHEDULER_LOGS
#ifdef HARDWARE_INDEPENDENT
            std::cout << "[Scheduler] Event <" << std::hex
            << event << "> dequeued." << std::endl;
#else
            Serial.print("[Scheduler] Event <");
            Serial.print((unsigned long) event);
            Serial.println("> dequeued.");
#endif
#endif
            continue;
        }
        
        // Attempt to find the event in the _eventsEnqued set (unusual).
        target = this->_eventsEnqueued.find(event);
        if (target != this->_eventsEnqueued.end())
        {
            // Any Event based object is unregistered Event destructor.
            this->_eventsEnqueued.erase(target);
            if (this->delegate) this->delegate->schedulerDequeuedEvent(this, event);
            
#if defined DEBUG && defined SCHEDULER_LOGS
#ifdef HARDWARE_INDEPENDENT
            std::cout << "[Scheduler] Event <" << std::hex
            << event << "> dequeued." << std::endl;
#else
            Serial.print("[Scheduler] Event <");
            Serial.print((unsigned long) event);
            Serial.println("> dequeued.");
#endif
#endif
            continue;
        }
        
        // If the code reached this stage, event wasn't found!
#if defined DEBUG && defined SCHEDULER_LOGS
#ifdef HARDWARE_INDEPENDENT
        std::cout << std::endl << "[Scheduler] ERROR: THE EVENT <" << std::hex
        << event << "> WASN'T FOUND!" << std::endl;
#else
        Serial.print("[Scheduler] ERROR: THE EVENT <");
        Serial.print((unsigned long) event);
        Serial.println("> WASN'T FOUND!");
#endif
#endif
    }
    
    this->_eventsDequeued.clear(); // Reset dequeued Events holder
    
    // Dequeue any pending daemons at this point to keep _daemons set
    // as small as possible when inserting, for better performance.
    for (Scheduler::Daemon * const daemon : this->_daemonsDequeued)
    {
        Scheduler::Daemons::const_iterator target = this->_daemons.find(daemon);
        
        // Attempt to find the event in the _daemons set (usual).
        if (target != this->_daemons.end())
        {
            this->_daemons.erase(target);
            if (this->delegate) this->delegate->schedulerDequeuedEvent(this, static_cast<Event *>(daemon));
#if defined DEBUG && defined SCHEDULER_LOGS
#ifdef HARDWARE_INDEPENDENT
            std::cout << std::endl << "[Scheduler] Daemon <" << std::hex
            << daemon << "> dequeued." << std::endl;
#else
            Serial.print("[Scheduler] Daemon <");
            Serial.print((unsigned long) daemon);
            Serial.println("> dequeued.");
#endif
#endif
            continue;
        }
        
        // Attempt to find the daemon in the _daemonsEnqued set (unusual).
        target = this->_daemonsEnqueued.find(daemon);
        if (target != this->_daemonsEnqueued.end())
        {
            
            this->_daemonsEnqueued.erase(target);
            if (this->delegate) this->delegate->schedulerDequeuedEvent(this, static_cast<Event *>(daemon));
#if defined DEBUG && defined SCHEDULER_LOGS
#ifdef HARDWARE_INDEPENDENT
            std::cout << std::endl << "[Scheduler] Daemon <" << std::hex
            << daemon << "> dequeued." << std::endl;
#else
            Serial.print("[Scheduler] Daemon <");
            Serial.print((unsigned long) daemon);
            Serial.println("> dequeued.");
#endif
#endif
            continue;
        }
        
        // If the code reached this stage, event wasn't found!
#if defined DEBUG && defined SCHEDULER_LOGS
#ifdef HARDWARE_INDEPENDENT
        std::cout << std::endl << "[Scheduler] ERROR: THE DAEMON <" << std::hex
        << daemon << "> WASN'T FOUND!" << std::endl;
#else
        Serial.print("[Scheduler] ERROR: THE DAEMON <");
        Serial.print((unsigned long) daemon);
        Serial.println("> WASN'T FOUND!");
#endif
#endif
    }
    
    this->_daemonsDequeued.clear(); // Reset dequeued Daemons holder
}

void Scheduler::_RegisterEventOfScheduler(Scheduler::Event *  const event,
                                          Scheduler * const scheduler)
{
    Scheduler::_EventSchedulerRegister[event] = scheduler;
    
#if defined DEBUG && defined SCHEDULER_LOGS
#ifdef HARDWARE_INDEPENDENT
    if (scheduler)
    {
        std::cout << "[Scheduler] Event <" << std::hex << event << " associated to Scheduler <" << std::hex << scheduler << ">." << std::endl;
    } else {
        std::cout << "[Scheduler] Event <" << std::hex << event << "> disassociated." << std::endl;
    }
    
    std::cout << "[Scheduler] " << Scheduler::_EventSchedulerRegister.size() << " Event(s) registered." << std::endl;
#else
    if (scheduler)
    {
        Serial.print("[Scheduler] Event <");
        Serial.print((unsigned long) event, HEX);
        Serial.print("> associated to Scheduler <");
        Serial.print((unsigned long) scheduler, HEX);
        Serial.print(">.");
    } else {
        Serial.print("[Scheduler] Event <");
        Serial.print((unsigned long) event, HEX);
        Serial.println("> disassociated.");
    }
    
    Serial.print("[Scheduler] ");
    Serial.print(Scheduler::_EventSchedulerRegister.size());
    Serial.println(" Event(s) registered.");
#endif
#endif
}

void Scheduler::_UnregisterEvent(Scheduler::Event * const event)
{
    Scheduler::_EventSchedulerRegister.erase(event);
    
#if defined DEBUG && defined SCHEDULER_LOGS
#ifdef HARDWARE_INDEPENDENT
    std::cout << "[Scheduler] Event <" << std::hex << event << "> unregistered." << std::endl;
    std::cout << "[Scheduler] " << Scheduler::_EventSchedulerRegister.size() << " Event(s) registered." << std::endl;
#else
    Serial.print("[Scheduler] Event <");
    Serial.print((unsigned long) event, HEX);
    Serial.println("> unregistered.");
    
    Serial.print("[Scheduler] ");
    Serial.print(Scheduler::_EventSchedulerRegister.size());
    Serial.println(" Event(s) registered.");
#endif
#endif
}

bool Scheduler::_RecalculateEventPriority(Scheduler::Event * const event)
{
    if (!event) return false;
    
#if defined DEBUG && defined SCHEDULER_LOGS
#ifdef HARDWARE_INDEPENDENT
    std::cout << "[Scheduler] Recalculating Event <" << std::hex << event << "> priority." << std::endl;
#else
    Serial.print("[Scheduler] Recalculating Event <");
    Serial.print((unsigned long) event, HEX);
    Serial.println("> priority.");
#endif
#endif
    
    Scheduler * const scheduler = Scheduler::_EventSchedulerRegister[event];
    
    if (!scheduler->dequeue(event))
    {
#if defined DEBUG && defined SCHEDULER_LOGS
#ifdef HARDWARE_INDEPENDENT
        std::cout << "[Scheduler] ERROR: UNABLE TO DEQUEUE Event <" << std::hex << event << ">!!!" << std::endl;
#else
        Serial.print("[Scheduler] ERROR: UNABLE TO DEQUEUE Event <");
        Serial.print((unsigned long) event, HEX);
        Serial.println(">!!!");
#endif
#endif
        return false;
    }
    
    if (!scheduler->enqueue(event))
    {
#if defined DEBUG && defined SCHEDULER_LOGS
#ifdef HARDWARE_INDEPENDENT
        std::cout << "[Scheduler] ERROR: UNABLE TO ENQUEUE Event <" << std::hex << event << ">!!!" << std::endl;
#else
        Serial.print("[Scheduler] ERROR: UNABLE TO ENQUEUE Event <");
        Serial.print((unsigned long) event, HEX);
        Serial.println(">!!!");
#endif
#endif
        return false;
    }
    
#if defined DEBUG && defined SCHEDULER_LOGS
#ifdef HARDWARE_INDEPENDENT
    std::cout << "[Scheduler] Recalculated Event <" << std::hex << event << "> priority." << std::endl;
#else
    Serial.print("[Scheduler] Recalculated Event <");
    Serial.print((unsigned long) event, HEX);
    Serial.println("> priority.");
#endif
#endif
    return true;
}

bool Scheduler::_RecalculateDaemonPriority(Scheduler::Daemon * const daemon)
{
    if (!daemon) return false;
    
#if defined DEBUG && defined SCHEDULER_LOGS
#ifdef HARDWARE_INDEPENDENT
    std::cout << "[Scheduler] Recalculating Daemon <" << std::hex << daemon << "> priority." << std::endl;
#else
    Serial.print("[Scheduler] Recalculating Daemon <");
    Serial.print((unsigned long) daemon, HEX);
    Serial.println("> priority.");
#endif
#endif
    
    Scheduler * const scheduler = Scheduler::_EventSchedulerRegister[static_cast<Scheduler::Event *>(daemon)];
    
    if (!scheduler->dequeue(daemon))
    {
#if defined DEBUG && defined SCHEDULER_LOGS
#ifdef HARDWARE_INDEPENDENT
        std::cout << "[Scheduler] ERROR: UNABLE TO DEQUEUE Daemon <" << std::hex << daemon << ">!!!" << std::endl;
#else
        Serial.print("[Scheduler] ERROR: UNABLE TO DEQUEUE Daemon <");
        Serial.print((unsigned long) daemon, HEX);
        Serial.println(">!!!");
#endif
#endif
        return false;
    }
    
    if (!scheduler->enqueue(daemon))
    {
#if defined DEBUG && defined SCHEDULER_LOGS
#ifdef HARDWARE_INDEPENDENT
        std::cout << "[Scheduler] ERROR: UNABLE TO ENQUEUE Event <" << std::hex << daemon << ">!!!" << std::endl;
#else
        Serial.print("[Scheduler] ERROR: UNABLE TO ENQUEUE Event <");
        Serial.print((unsigned long) daemon, HEX);
        Serial.println(">!!!");
#endif
#endif
        return false;
    }
    
#if defined DEBUG && defined SCHEDULER_LOGS
#ifdef HARDWARE_INDEPENDENT
    std::cout << "[Scheduler] Recalculated Daemon <" << std::hex << daemon << "> priority." << std::endl;
#else
    Serial.print("[Scheduler] Recalculated Daemon <");
    Serial.print((unsigned long) daemon, HEX);
    Serial.println("> priority.");
#endif
#endif
    return true;
}

Scheduler::Scheduler():
delegate(nullptr)
{
    Scheduler::_Register.insert(this);
}

Scheduler::~Scheduler()
{
    Scheduler::_Register.erase(this);
}

