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
std::set<Scheduler *> Scheduler::_InstanceRegister;
std::map<Scheduler::Event *, Scheduler *> Scheduler::_AssociationRegister;


// =============================================================================
// Scheduler::Event : Implementation
// =============================================================================
Scheduler::Time Scheduler::Event::executeTime() const
{
    return this->_executeTime;
}

void Scheduler::Event::setExecuteTime(Scheduler::Time const executeTime)
{
    Time const lastExecuteTime = this->executeTime();
    this->_executeTime = executeTime;
    if (lastExecuteTime != this->executeTime()) this->_executeTimeReprioritize(lastExecuteTime);
}

void Scheduler::Event::_executeTimeReprioritize(Scheduler::Time const lastPriority)
{
    Scheduler::_ReprioritizeEvent(this, lastPriority);
}

Scheduler::Event::Event(Scheduler::Time const executeTime):
_executeTime(executeTime)
{
    Identifiable<Event>::Register(this); // RTTI Substitute
    Scheduler::_AssociateEventToScheduler(this);
}

Scheduler::Event::~Event()
{
    Identifiable<Event>::Unregister(this); // RTTI Substitute
    Scheduler::_DissasociateEvent(this);
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

Scheduler::Daemon::Daemon(Scheduler::Time const executeTime,
                          Scheduler::Time const executeTimeInterval):
Scheduler::Event(executeTime),
_executeTimeInterval(executeTimeInterval)
{
    Identifiable<Daemon>::Register(this); // RTTI Substitute
}

Scheduler::Daemon::~Daemon()
{
    Identifiable<Daemon>::Unregister(this); // RTTI Substitute
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
// Scheduler::Task : Implementation
// =============================================================================
bool Scheduler::Task::operator<(Scheduler::Task const &other) const
{
    return this->priority < other.priority;
}

bool Scheduler::Task::operator>(Scheduler::Task const &other) const
{
    return this->priority > other.priority;
}

bool Scheduler::Task::operator==(Scheduler::Task const &other) const
{
    return this->priority == other.priority;
}

Scheduler::Task::Task(Scheduler::Task const &task):
events(task.events),
priority(task.priority)
{
    
}

Scheduler::Task::Task(Scheduler::Event * const event):
priority(event->executeTime())
{
    this->events.insert(event);
}

Scheduler::Task::Task(Scheduler::Time const priority):
priority(priority)
{
    
}


// =============================================================================
// Scheduler : Implementation
// =============================================================================
bool Scheduler::enqueue(Scheduler::Event * const event)
{
    if (!event) return false;
    
    if (!Scheduler::_EnqueueTasksEvent(this->_tasks, event)) return false;
    
    // Associate enqueueing Event to the Scheduler instance on registry.
    Scheduler::_AssociateEventToScheduler(event, this);
    
    if (this->delegate) this->delegate->schedulerEnqueuedEvent(this, event);
    
    return true;
}

bool Scheduler::dequeue(Scheduler::Event * const event)
{
    if (!Scheduler::_DequeueTasksEvent(this->_tasks, event))
    {
#if defined DEBUG && defined SCHEDULER_LOGS
#ifdef HARDWARE_INDEPENDENT
        std::cout << "[Scheduler <" << std::hex << this << ">] ERROR: Event <" << std::hex << event << "> WASN'T FOUND (MISSING)!!!" << std::endl;
#else
        Serial.print("[Scheduler <");
        Serial.print((unsigned long) this, HEX);
        Serial.print(">] ERROR: Event <");
        Serial.print((unsigned long) event);
        Serial.print("> WASN'T FOUND (MISSING)!!!");
#endif
#endif
        return false;
    }
    
    // Disassociate Event from Scheduler instance on registry.
    Scheduler::_AssociateEventToScheduler(event, nullptr);
    
    if (this->delegate) this->delegate->schedulerDequeuedEvent(this, event);
    
    return true;
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
    
    for (Scheduler * const scheduler : Scheduler::_InstanceRegister)
    {
#if defined DEBUG && defined SCHEDULER_LOGS
#ifdef HARDWARE_INDEPENDENT
        std::cout << "[Scheduler <" << std::hex << scheduler << "> IS STARTING]" << std::endl;
        for (Scheduler::Task const &task : scheduler->_tasks)
        {
            std::cout << "[Scheduler <" << std::hex << scheduler << ">] Holding " << task.events.size() << " Event(s) <P: " << task.priority << "> pending runtime." << std::endl;
        }
#else
        Serial.print("[Scheduler <");
        Serial.print((unsigned long) scheduler, HEX);
        Serial.println("> IS STARTING]");
        for (Scheduler::Task const &task : scheduler->_tasks)
        {
            Serial.print("[Scheduler <");
            Serial.print((unsigned long) scheduler, HEX);
            Serial.print(">] Holding ");
            Serial.print(task.events.size());
            Serial.print(" Event(s) <P: ");
            Serial.print(task.priority);
            Serial.println("> pending runtime.");
        }
#endif
#endif
        scheduler->_processEventsForTime(time);
        
#if defined DEBUG && defined SCHEDULER_LOGS
#ifdef HARDWARE_INDEPENDENT
        std::cout << "[Scheduler <" << std::hex << scheduler << "> IS PAUSING]" << std::endl;
#else
        Serial.print("[Scheduler <");
        Serial.print((unsigned long) scheduler, HEX);
        Serial.println("> IS PAUSING]");
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

void Scheduler::_processEventsForTime(Scheduler::Time const time)
{
    // Check for potential time overflow, start with those & reset the overflowed Tasks set.
    if (this->_lastTime > time) {this->_tasks = this->_tasksOverflowed; this->_tasksOverflowed.clear();}
    
    // We'll be copying the Event instances which are going to be executed this cycle to the variable below
    // for safety. That's because we might be modifying the _tasks container leading to undefined behaviour.
    Scheduler::Tasks executableTasks; // Stores Task instances which will be executed in this update cycle.
    
    // Get a copy of all Task instances (pointers) we'll be executing this update cycle for safety.
    for (Scheduler::Task const &task : this->_tasks)
    {
        // All Event instances of equal execution time are stored in the same Task instance,
        // these Task instances are then stored in std::map and are sorted (prioritized) incrementally.
        // That means Task instances with lower priority come first, beacuse Event instances containing
        // lower (earlier) execution times must be executed before those of higher (later) execution times.
        // This means only Task instances with priority less than or equal to time must be executed now.
        if (task.priority > time) break;
        
        executableTasks.insert(task);
    }
    
    for (Scheduler::Task const &task : executableTasks)
    {
        for (Scheduler::Event * const event : task.events)
        {
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
            Serial.print((unsigned long) this, HEX);
            Serial.print(">] Event <");
            Serial.print((unsigned long) event, HEX);
            Serial.println("> running.");
#endif
#endif
            int const error = event->execute(time);
            
            if (error)
            {
#if defined DEBUG && defined SCHEDULER_LOGS
#ifdef HARDWARE_INDEPENDENT
                std::cout << "[Scheduler <" << std::hex << this << ">] Event <"
                << std::hex << event << "> returned error code " << std::dec << error << std::endl;
#else
                Serial.print("[Scheduler <");
                Serial.print((unsigned long) this, HEX);
                Serial.print(">] Event <");
                Serial.print((unsigned long) event, HEX);
                Serial.print("> returned error code ");
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
            
            // Check for special case, being Daemon instances.
            if (Identifiable<Daemon>::Instanced(event))
            {
                // Since this is a Daemon, and Daemons repeat until finished,
                // calcualte next execution time and request scheduler priority update.
                Daemon * const daemon = static_cast<Daemon *>(event);
                if (!daemon->finished())
                {
                    // Calculate the Daemon instance's next execution time.
                    Scheduler::Time const executeTime = time + daemon->executeTimeInterval();
                    
                    // Check for potential Scheduler::Time integer overflow.
                    if (executeTime < time)
                    {
                        // Remove from main Task instance set and reinsert into overflowed set.
                        if (Scheduler::_DequeueTasksEvent(this->_tasks, event))
                        {
                            // Since we've dequeued the event, it's not going to attempt to reprioritize.
                            event->setExecuteTime(executeTime); // Preventing reprioritizing here.
                            Scheduler::_EnqueueTasksEvent(this->_tasksOverflowed, event);
                        }
                    }
                    // Update the execution time, but notice this reprioritizes the event.
                    else daemon->setExecuteTime(executeTime);
                }
                // These will only be dequeued with notification when they're really done.
                else this->dequeue(static_cast<Event *>(daemon));
            }
            // These will only be dequeued with notification when they're really done.
            else this->dequeue(event);
        }
    }
    
    this->_lastTime = time;
}

bool Scheduler::_EnqueueTasksEvent(Scheduler::Tasks &tasks, Scheduler::Event * const event)
{
    // Passing priority to prevent the task instance inserting the element (temporary task obj.)
    // We're just concerend with getting the Task instance matching this one's priority.
    // NOTE: Sets ONLY return const_iterator and implicitly const iterator (using mutable to work around it).
    // The "mutable" keyword works in this case because it doesn't affect the priority or position in the map.
    Scheduler::Tasks::const_iterator const task = tasks.find(Scheduler::Task(event->executeTime()));
    
    if (task == tasks.end()) tasks.insert(Scheduler::Task(event));
    else task->events.insert(event);
    
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
    
    return true;
}

bool Scheduler::_DequeueTasksEvent(Scheduler::Tasks &tasks, Scheduler::Event * const event)
{
    Scheduler::Tasks::const_iterator const task = tasks.find(Scheduler::Task(event->executeTime()));
    
    // If the event's priority task is found, it may be found inside the task event's set.
    // If category doesn't exist, event definitely isn't in the tasks set.
    if (task != tasks.end() && task->events.count(event))
    {
        // Event based objects are unregistered by the Event destructor.
        if (task->events.size() > 1) task->events.erase(event);
        else tasks.erase(task);
        
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
        
        return true;
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
    
    return false;
}

bool Scheduler::_DequeueTasksEventWithPriority(Scheduler::Tasks &tasks,
                                               Scheduler::Event * const event,
                                               Scheduler::Time const priority)
{
    Scheduler::Tasks::const_iterator const task = tasks.find(Scheduler::Task(priority));
    
    // If the event's priority task is found, it may be found inside the task event's set.
    // If category doesn't exist, event definitely isn't in the tasks set.
    if (task != tasks.end() && task->events.count(event))
    {
        // Event based objects are unregistered by the Event destructor.
        if (task->events.size() > 1) task->events.erase(event);
        else tasks.erase(task);
        
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
        
        return true;
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
    
    return false;
}

void Scheduler::_AssociateEventToScheduler(Scheduler::Event *  const event,
                                           Scheduler * const scheduler)
{
    Scheduler::_AssociationRegister[event] = scheduler;
    
#if defined DEBUG && defined SCHEDULER_LOGS
#ifdef HARDWARE_INDEPENDENT
    if (scheduler)
    {
        std::cout << "[Scheduler] Event <" << std::hex << event << " associated to Scheduler <" << std::hex << scheduler << ">." << std::endl;
    } else {
        std::cout << "[Scheduler] Event <" << std::hex << event << "> disassociated." << std::endl;
    }
    
    std::cout << "[Scheduler] " << Scheduler::_AssociationRegister.size() << " Event(s) registered." << std::endl;
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
    Serial.print(Scheduler::_AssociationRegister.size());
    Serial.println(" Event(s) registered.");
#endif
#endif
}

void Scheduler::_DissasociateEvent(Scheduler::Event * const event)
{
    Scheduler::_AssociationRegister.erase(event);
    
#if defined DEBUG && defined SCHEDULER_LOGS
#ifdef HARDWARE_INDEPENDENT
    std::cout << "[Scheduler] Event <" << std::hex << event << "> unregistered." << std::endl;
    std::cout << "[Scheduler] " << Scheduler::_AssociationRegister.size() << " Event(s) registered." << std::endl;
#else
    Serial.print("[Scheduler] Event <");
    Serial.print((unsigned long) event, HEX);
    Serial.println("> unregistered.");
    
    Serial.print("[Scheduler] ");
    Serial.print(Scheduler::_AssociationRegister.size());
    Serial.println(" Event(s) registered.");
#endif
#endif
}

bool Scheduler::_ReprioritizeEvent(Scheduler::Event * const event, Time const lastPriority)
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
    
    Scheduler * const scheduler = Scheduler::_AssociationRegister[event];
    
    // No need to reprioritize if not in a scheduler.
    if (scheduler) {
        if (!Scheduler::_DequeueTasksEventWithPriority(scheduler->_tasks, event, lastPriority))
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
        
        if (!Scheduler::_EnqueueTasksEvent(scheduler->_tasks, event))
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

Scheduler::Scheduler():
delegate(nullptr),
_lastTime(0)
{
    Scheduler::_InstanceRegister.insert(this);
}

Scheduler::~Scheduler()
{
    Scheduler::_InstanceRegister.erase(this);
}

