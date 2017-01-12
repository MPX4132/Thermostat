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
                                              Scheduler::Event const * const eventR)
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
    if (lastExecuteTime != this->executeTime()) Scheduler::_RecalculateEventPriority(this);
}

Scheduler::Time Scheduler::Event::executeTimeInterval() const
{
    return this->_executeTimeInterval;
}

void Scheduler::Event::setExecuteTimeInterval(Scheduler::Time const executeTimeInterval)
{
    this->_executeTimeInterval = executeTimeInterval;
}

bool Scheduler::Event::finished() const
{
    return !this->executeTimeInterval();
}

Scheduler::Time Scheduler::Event::_executeTimeUpdate(Scheduler::Time const updateTime)
{
    this->setExecuteTime(updateTime + this->executeTimeInterval());
    return this->executeTime();
}

Scheduler::Event::Event(Scheduler::Time const executeTime,
                        Scheduler::Time const executeTimeInterval):
_executeTime(executeTime),
_executeTimeInterval(executeTimeInterval)
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
/*Scheduler::Time Scheduler::Daemon::executeTimeInterval() const
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
 
 Scheduler::Time Scheduler::Daemon::_executeTimeUpdate(Scheduler::Time const updateTime)
 {
 this->setExecuteTime(updateTime + this->executeTimeInterval());
 return this->executeTime();
 }
 
 Scheduler::Daemon::Daemon(Scheduler::Time executeTime,
 Scheduler::Time executeTimeInterval):
 Event(executeTime),
 _executeTimeInterval(executeTimeInterval)
 {
 
 }*/


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
    
    Scheduler::_RegisterEventOfScheduler(event, this); // Associate event to this
    this->_events.insert(event);
    
#ifdef DEBUG
#ifdef HARDWARE_INDEPENDENT
    std::cout << "[Scheduler <" << std::hex << this << ">] Enqueued Event <"
              << std::hex << event << "> (Tracking " << this->_events.size() << " Events(s))." << std::endl;
#else
    Serial.print("[Scheduler <");
    Serial.print((unsigned long)this, HEX);
    Serial.print(">] Enqueued Event <");
    Serial.print((unsigned long)event, HEX);
    Serial.print("> (Tracking ");
    Serial.print(this->_events.size());
    Serial.println(" Event(s)).");
#endif
#endif
    
    if (this->delegate) this->delegate->schedulerEnqueuedEvent(this, event);
    return true;
}

bool Scheduler::dequeue(Scheduler::Event * const event)
{
    auto target = std::find(this->_events.begin(), this->_events.end(), event);

    if (target == this->_events.end()) return false;
    
    this->_dequeue(target);
    
    return true;
}

void Scheduler::UpdateAll(Scheduler::Time const time)
{
#ifdef DEBUG
#ifdef HARDWARE_INDEPENDENT
    std::cout << std::endl << "==============" << std::endl;
#else
    Serial.println("");
    Serial.println("==============");
#endif
#endif

    for (Scheduler * const scheduler : Scheduler::_Register)
    {
#ifdef DEBUG
#ifdef HARDWARE_INDEPENDENT
        std::cout << "[Scheduler <" << std::hex << scheduler << ">] Starting." << std::endl;
#else
        Serial.print("[Scheduler <");
        Serial.print((unsigned long) scheduler, HEX);
        Serial.println(">] Starting.");
#endif
#endif
        scheduler->_update(time);
        
#ifdef DEBUG
#ifdef HARDWARE_INDEPENDENT
        std::cout << "[Scheduler <" << std::hex << scheduler << ">] Stopping." << std::endl;
#else
        Serial.print("[Scheduler <");
        Serial.print((unsigned long) scheduler, HEX);
        Serial.println(">] Stopping.");
#endif
#endif
    }
    
#ifdef DEBUG
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
    Scheduler::Events::iterator eventi = this->_events.begin();
    while (eventi != this->_events.end())
    {
        Event * const event = *eventi;
        
        //if (event->executeTime() > time) break;
        if (event->executeTime() > time)
        {
#ifdef DEBUG
#ifdef HARDWARE_INDEPENDENT
            std::cout << "[Scheduler <" << std::hex << this << ">] Event <"
                      << std::hex << event << "> will execute @ " << std::dec << event->executeTime() << "; Time: " << time << std::endl;
#else
            Serial.print("[Scheduler <");
            Serial.print((unsigned long)this, HEX);
            Serial.print(">] Event <");
            Serial.print((unsigned long) event, HEX);
            Serial.print("> will execute @ ");
            Serial.print(event->executeTime());
            Serial.print("; Time: ");
            Serial.println(time);
#endif
#endif
            ++eventi; continue;
        }
        
        if (this->delegate) this->delegate->schedulerStartingEvent(this, event);
        
#ifdef DEBUG
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
        Serial.print("> running.");
        Serial.println(time);
#endif
#endif
        
        int const errorCode = event->execute(time);
        
        if (errorCode)
        {
#ifdef DEBUG
#ifdef HARDWARE_INDEPENDENT
            std::cout << "Event returned error code " << errorCode << std::endl;
#else
            Serial.print("Event returned error code ");
            Serial.println(errorCode);
#endif
#endif
        }
        
#ifdef DEBUG
#ifdef HARDWARE_INDEPENDENT
        std::cout << "[Scheduler <" << std::hex << this << ">] Event <"
                  << std::hex << event << "> halting." << std::endl;
        std::cout << "==========" << std::endl << std::endl;
#else
        Serial.print("[Scheduler <");
        Serial.print((unsigned long)this, HEX);
        Serial.print(">] Event <");
        Serial.print((unsigned long)(event), HEX);
        Serial.print("> halting.");
        Serial.println(time);
        Serial.println("==========");
        Serial.println("");
#endif
#endif
        
        
        // Can't use the line below, since RTTI isn't enabled, and it wont fit
        // if enabled for all files including core and other bs. Can't figure out how to enable it
        // for independent files to only enable if for this class.
        
        // Try to cast the event as daemon, if nullptr is returned it's not a daemon!
        //Scheduler::Daemon * daemon = dynamic_cast<Scheduler::Daemon*>(event);
        
        // Only retain daemons iff they haven't yet finished.
        //if (daemon && !daemon->finished()) continue;

        //if (!event->finished()) continue;
        if (!event->finished())
        {
            ++eventi; continue;
        }
        
        eventi = this->_dequeue(eventi);//this->_events.erase(eventi);
        
        if (this->delegate) this->delegate->schedulerCompletedEvent(this, event);
    }
}

Scheduler::Events::const_iterator Scheduler::_dequeue(Events::const_iterator &event)
{
    Scheduler::_RegisterEventOfScheduler(*event); // Disassociate event from this
    
    Events::const_iterator const nextEvent = this->_events.erase(event);
    
    if (this->delegate) this->delegate->schedulerDequeuedEvent(this, *event);
    
#ifdef DEBUG
#ifdef HARDWARE_INDEPENDENT
    std::cout << "[Scheduler <" << std::hex << this << ">] Dequeued Event <"
              << std::hex << *event << "> (Tracking " << this->_events.size() << " Events(s))." << std::endl;
#else
    Serial.print("[Scheduler <");
    Serial.print((unsigned long)this, HEX);
    Serial.print(">] Dequeued Event <");
    Serial.print((unsigned long) *event, HEX);
    Serial.print("> (Tracking ");
    Serial.print(this->_events.size());
    Serial.println(" Event(s)).");
#endif
#endif
    
    return nextEvent;
}

void Scheduler::_RegisterEventOfScheduler(Scheduler::Event * event,
                                          Scheduler * scheduler)
{
    _EventSchedulerRegister[event] = scheduler;

#ifdef DEBUG
#ifdef HARDWARE_INDEPENDENT
    std::cout << "[Scheduler] Scheduler <" << std::hex << scheduler << "> associated to Event <"
              << std::hex << event << ">." << std::endl;
    
    std::cout << "[Scheduler] " << _EventSchedulerRegister.size() << " Event(s) registered." << std::endl;
#else
    Serial.print("[Scheduler] Scheduler <");
    Serial.print((unsigned long) scheduler, HEX);
    Serial.print("> associated to Event <");
    Serial.print((unsigned long) event, HEX);
    Serial.println(">.");
    
    Serial.print("[Scheduler] ");
    Serial.print(_EventSchedulerRegister.size());
    Serial.println(" Event(s) registered.");
#endif
#endif
}

void Scheduler::_UnregisterEvent(Scheduler::Event * event)
{
    _EventSchedulerRegister.erase(event);
    
#ifdef DEBUG
#ifdef HARDWARE_INDEPENDENT
    std::cout << "[Scheduler] Event <" << std::hex << event << "> unregistered." << std::endl;
    std::cout << "[Scheduler] " << _EventSchedulerRegister.size() << " Event(s) registered." << std::endl;
#else
    Serial.print("[Scheduler] Event <");
    Serial.print((unsigned long) event, HEX);
    Serial.println("> unregistered.");
    
    Serial.print("[Scheduler] ");
    Serial.print(_EventSchedulerRegister.size());
    Serial.println(" Event(s) registered.");
#endif
#endif
}

void Scheduler::_RecalculateEventPriority(Scheduler::Event * event)
{
    if (!event) return;
    
#ifdef DEBUG
#ifdef HARDWARE_INDEPENDENT
    std::cout << "[Scheduler] Recalculating Event <" << std::hex << event << "> priority." << std::endl;
#else
    Serial.print("[Scheduler] Recalculating Event <");
    Serial.print((unsigned long) event, HEX);
    Serial.println("> priority.");
#endif
#endif
    //Serial.println("[Scheduler] About to recaltulate the priority!");
    Scheduler * scheduler = Scheduler::_EventSchedulerRegister[event];
    /*std::cout << "[Scheduler] Retrived event's scheduler <");
    std::cout << (unsigned long) scheduler, HEX);
    Serial.println(">...");*/
    scheduler->dequeue(event);
    //Serial.println("[Scheduler] Extracted event...");
    scheduler->enqueue(event);
    //Serial.println("[Scheduler] Inserted event, recalucate successful!");
    
#ifdef DEBUG
#ifdef HARDWARE_INDEPENDENT
    std::cout << "[Scheduler] Recalculated Event <" << std::hex << event << "> priority." << std::endl;
#else
    Serial.print("[Scheduler] Recalculated Event <");
    Serial.print((unsigned long) event, HEX);
    Serial.println("> priority.");
#endif
#endif
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

