//
//  Scheduler.cpp
//  Thermostat
//
//  Created by Matias Barcenas on 11/12/16.
//  Copyright © 2016 Matias Barcenas. All rights reserved.
//

#include "Scheduler.hpp"

// =============================================================================
#pragma mark - Scheduler : Static Variables Declaration
// =============================================================================
std::set<Scheduler *> Scheduler::_Register;
std::map<Scheduler::Event *, Scheduler *> Scheduler::_EventSchedulerRegister;


// =============================================================================
#pragma mark - Scheduler::Event : Implementation
// =============================================================================
bool Scheduler::Event::PtrCompare::operator()(Scheduler::Event const * const eventL,
                                              Scheduler::Event const * const eventR)
{
    return (*eventL) < (*eventR);
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

Scheduler::Event::Event(Scheduler::Time executeTime):
_executeTime(executeTime)
{
    Scheduler::_RegisterEventOfScheduler(this);
}

Scheduler::Event::~Event()
{
    Scheduler::_UnregisterEvent(this);
}


// =============================================================================
#pragma mark - Scheduler::Daemon : Implementation
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
    
}


// =============================================================================
#pragma mark - Scheduler::Delegate : Implementation
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
#pragma mark - Scheduler : Implementation
// =============================================================================
bool Scheduler::enqueue(Scheduler::Event * const event)
{
    if (!event) return false;
    this->_events.insert(event);
    if (this->delegate) this->delegate->schedulerEnqueuedEvent(this, event);
    return true;
}

bool Scheduler::dequeue(Scheduler::Event * const event)
{
    auto target = std::find(this->_events.begin(), this->_events.end(), event);
    if (target == this->_events.end()) return false;
    this->_events.erase(target);
    if (this->delegate) this->delegate->schedulerDequeuedEvent(this, event);
    return true;
}

void Scheduler::UpdateAll(Scheduler::Time const time)
{
    for (Scheduler *scheduler : Scheduler::_Register) {
        scheduler->_update(time);
    }
}

void Scheduler::_update(Scheduler::Time const time)
{
    for (auto eventi = this->_events.begin(); eventi != this->_events.end(); eventi++) {
        Event * const event = *eventi;
        
        if (event->executeTime() > time) break;
        
        if (this->delegate) this->delegate->schedulerStartingEvent(this, event);
        event->execute(time);
        
        Scheduler::Daemon * daemon = dynamic_cast<Scheduler::Daemon*>(event);
        
        // Only retain daemons iff they haven't yet finished.
        if (daemon && !daemon->finished()) continue;
        
        this->_events.erase(eventi);
        
        if (this->delegate) this->delegate->schedulerCompletedEvent(this, event);
    }
}

void Scheduler::_RegisterEventOfScheduler(Scheduler::Event * event,
                                          Scheduler * scheduler)
{
    _EventSchedulerRegister[event] = scheduler;
}

void Scheduler::_UnregisterEvent(Scheduler::Event * event)
{
    _EventSchedulerRegister.erase(event);
}

void Scheduler::_RecalculateEventPriority(Scheduler::Event * event)
{
    if (!event) return;
    Scheduler * scheduler = Scheduler::_EventSchedulerRegister[event];
    scheduler->dequeue(event);
    scheduler->enqueue(event);
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
