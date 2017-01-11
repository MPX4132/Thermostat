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
    // If both events have the same time, then go by their address.
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
    Serial.print("[Scheduler <");
    Serial.print((unsigned long)this, HEX);
    Serial.print(">] Enqueueing Event <");
    Serial.print((unsigned long)event, HEX);
    Serial.println(">");
    if (!event) return false;
    
    Scheduler::_RegisterEventOfScheduler(event, this); // Associate event to this
    this->_events.insert(event);
    
    Serial.print("[Scheduler <");
    Serial.print((unsigned long)this, HEX);
    Serial.print(">] Enqueued Event <");
    Serial.print((unsigned long)event, HEX);
    Serial.print("> (Tracking ");
    Serial.print(this->_events.size());
    Serial.println(" Event(s)).");
    
    if (this->delegate) this->delegate->schedulerEnqueuedEvent(this, event);
    return true;
}

bool Scheduler::dequeue(Scheduler::Event * const event)
{
    Serial.print("[Scheduler <");
    Serial.print((unsigned long)this, HEX);
    Serial.print(">] Dequeueing Event <");
    Serial.print((unsigned long)event, HEX);
    Serial.println(">");
    
    //Serial.println("[Scheduler] Will search for target...");
    auto target = std::find(this->_events.begin(), this->_events.end(), event);
    //Serial.println("[Scheduler] Found something...");
    if (target == this->_events.end()) return false;
    //Serial.println("[Scheduler] Target found, attempting to erase!");
    this->_events.erase(target);
    //Serial.println("[Scheduler] Target deleted successfully!");
    if (this->delegate) this->delegate->schedulerDequeuedEvent(this, event);
    //Serial.println("[Scheduler] Notifying delegate!");
    
    Serial.print("[Scheduler <");
    Serial.print((unsigned long)this, HEX);
    Serial.print(">] Dequeued Event <");
    Serial.print((unsigned long)event, HEX);
    Serial.print("> (Tracking ");
    Serial.print(this->_events.size());
    Serial.println(" Event(s)).");
    
    return true;
}

void Scheduler::UpdateAll(Scheduler::Time const time)
{
    Serial.println("");
    Serial.println("==============");
    for (Scheduler * const scheduler : Scheduler::_Register)
    {
        Serial.print("[Scheduler <");
        Serial.print((unsigned long) scheduler, HEX);
        Serial.println(">] Starting.");
        scheduler->_update(time);
        Serial.print("[Scheduler <");
        Serial.print((unsigned long) scheduler, HEX);
        Serial.println(">] Stopping.");
    }
    Serial.println("==============");
    Serial.println("");
}

void Scheduler::_update(Scheduler::Time const time)
{
    //std::set<Event *, Event::PtrCompare> _events;
    //std::vector<std::set<Scheduler::Event *, Scheduler::Event::PtrCompare>::iterator> completed;
    std::set<Scheduler::Event *, Scheduler::Event::PtrCompare> events(this->_events);
    for (std::set<Scheduler::Event *, Scheduler::Event::PtrCompare>::iterator eventi = events.begin(); eventi != events.end(); eventi++) {
        Event * const event = *eventi;
        
        //if (event->executeTime() > time) break;
        if (event->executeTime() > time)
        {
            Serial.print("[Scheduler <");
            Serial.print((unsigned long)this, HEX);
            Serial.print(">] Event <");
            Serial.print((unsigned long) event, HEX);
            Serial.print("> will execute @ ");
            Serial.print(event->executeTime());
            Serial.print(", it's currently ");
            Serial.println(time);
            continue;
        }
        
        if (this->delegate) this->delegate->schedulerStartingEvent(this, event);
        
        Serial.println("");
        Serial.println("==========");
        Serial.print("[Scheduler <");
        Serial.print((unsigned long)this, HEX);
        Serial.print(">] Event <");
        Serial.print((unsigned long)event, HEX);
        Serial.print("> starting @ ");
        Serial.println(time);
        
        int const errorCode = event->execute(time);
        
        if (errorCode)
        {
            Serial.print("Event returned error code ");
            Serial.println(errorCode);
        }
        
        Serial.print("[Scheduler <");
        Serial.print((unsigned long)this, HEX);
        Serial.print(">] Event <");
        Serial.print((unsigned long)(event), HEX);
        Serial.print("> stopping @ ");
        Serial.println(time);
        Serial.println("==========");
        Serial.println("");
        
        
        // Can't use the line below, must use old-ass C style of casting.
        // RTTI isn't enabled and if I do enable it the damned sketch doesn't fit
        // on the fucking ESP module... I've wasted enough time looking for how
        // to enable it on a signel file.
        
        // Try to cast the event as daemon, if nullptr is returned it's not a daemon!
        //Scheduler::Daemon * daemon = dynamic_cast<Scheduler::Daemon*>(event);
        
        // Only retain daemons iff they haven't yet finished.
        //if (daemon && !daemon->finished()) continue;

        if (!event->finished()) continue;
        
#warning Removing iterator here might corrupt the iterative for-loop.
        //this->_events.erase(eventi);
        //completed.push_back(eventi);
        
        if (this->delegate) this->delegate->schedulerCompletedEvent(this, event);
    }
    
    /*for (std::vector<std::set<Scheduler::Event *, Scheduler::Event::PtrCompare>::iterator>::iterator eventi : completed)
     {
     this->_events.erase(eventi);
     }*/
}

void Scheduler::_RegisterEventOfScheduler(Scheduler::Event * event,
                                          Scheduler * scheduler)
{
    _EventSchedulerRegister[event] = scheduler;
    if (scheduler)
    {
        Serial.print("[Scheduler <");
        Serial.print((unsigned long) scheduler, HEX);
        Serial.print(">] Event <");
        Serial.print((unsigned long) event, HEX);
        Serial.println("> registered.");
    } else {
        Serial.print("[Scheduler] Event <");
        Serial.print((unsigned long) event, HEX);
        Serial.println("> registered.");
    }
    
    Serial.print("[Scheduler] ");
    Serial.print(_EventSchedulerRegister.size());
    Serial.println(" Event(s) registered.");
}

void Scheduler::_UnregisterEvent(Scheduler::Event * event)
{
    _EventSchedulerRegister.erase(event);
    Serial.print("[Scheduler] Event <");
    Serial.print((unsigned long) event, HEX);
    Serial.println("> unregistered.");
    
    Serial.print("[Scheduler] ");
    Serial.print(_EventSchedulerRegister.size());
    Serial.println(" Event(s) registered.");
}

void Scheduler::_RecalculateEventPriority(Scheduler::Event * event)
{
    if (!event) return;
    Serial.print("[Scheduler] Recalculating Event <");
    Serial.print((unsigned long) event, HEX);
    Serial.println("> priority.");
    //Serial.println("[Scheduler] About to recaltulate the priority!");
    Scheduler * scheduler = Scheduler::_EventSchedulerRegister[event];
    /*Serial.print("[Scheduler] Retrived event's scheduler <");
    Serial.print((unsigned long) scheduler, HEX);
    Serial.println(">...");*/
    scheduler->dequeue(event);
    //Serial.println("[Scheduler] Extracted event...");
    scheduler->enqueue(event);
    //Serial.println("[Scheduler] Inserted event, recalucate successful!");
    Serial.print("[Scheduler] Recalculated Event <");
    Serial.print((unsigned long) event, HEX);
    Serial.println("> priority.");
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

