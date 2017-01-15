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
    if (lastExecuteTime != this->executeTime()) this->_executeTimeReprioritize();
}

void Scheduler::Event::_executeTimeReprioritize()
{
    Scheduler::_ReprioritizeEvent(this);
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

Scheduler::Time Scheduler::Daemon::_executeTimeUpdate(Scheduler::Time const updateTime)
{
    this->setExecuteTime(updateTime + this->executeTimeInterval());
    return this->executeTime();
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
    
    // Passing priority to prevent the task instance inserting the element (temporary task obj.)
    // We're just concerend with getting the task matching this one's priority.
    // Sets can only return const_iterator and implicitly const iterator (using mutable to work around it).
    Scheduler::Tasks::const_iterator task = this->_tasksEnqueued.find(Scheduler::Task(event->executeTime()));
    
    if (task == this->_tasksEnqueued.end()) this->_tasksEnqueued.insert(Scheduler::Task(event));
    else task->events.insert(event);
    
    if (this->delegate) this->delegate->schedulerEnqueuedEvent(this, event);
    
#if defined DEBUG && defined SCHEDULER_LOGS
#ifdef HARDWARE_INDEPENDENT
    std::cout << "[Scheduler <" << std::hex << this << ">] Event <" << std::hex << event << "> is pending enqueue (" << ((task != this->_tasksEnqueued.end())? task->events.size() : 1) << " pending Event(s))." << std::endl;
#else
    Serial.print("[Scheduler <");
    Serial.print((unsigned long) this, HEX);
    Serial.print(">] Event <");
    Serial.print((unsigned long) event, HEX);
    Serial.print("> is pending enqueue (");
    Serial.print((task != this->_tasksEnqueued.end())? task->events.size() : 1);
    Serial.println(" pending Event(s)).");
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
    
    Scheduler::Tasks::const_iterator task; // Used to check existance of event in _tasks or _enqueuedTasks.
    
    task = this->_tasks.find(Scheduler::Task(event->executeTime()));
    
    // If the event's priority task is found, it may be found inside the task event's set.
    // If category doesn't exist, event definitely isn't in the tasks set.
    if (task != this->_tasks.end() && task->events.count(event))
    {
        Scheduler::Tasks::const_iterator holder = this->_tasksDequeued.find(Task(event->executeTime()));
        
        if (holder == this->_tasksDequeued.end()) this->_tasksDequeued.insert(Task(event));
        else holder->events.insert(event);
            
        if (this->delegate) this->delegate->schedulerDequeuedEvent(this, event);
        return true;
    }
    
    task = this->_tasksEnqueued.find(Scheduler::Task(event));
    
    // If the event's priority task is found, it may be found inside the task event's set.
    // If category doesn't exist, event definitely isn't in the tasks set.
    if (task != this->_tasksEnqueued.end() && task->events.count(event))
    {
        Scheduler::Tasks::const_iterator holder = this->_tasksDequeued.find(Task(event->executeTime()));
        
        if (holder == this->_tasksDequeued.end()) this->_tasksDequeued.insert(Task(event));
        else holder->events.insert(event);
        
        if (this->delegate) this->delegate->schedulerDequeuedEvent(this, event);
        return true;
    }
    
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
        for (Scheduler::Task const &task : scheduler->_tasksEnqueued)
        {
            std::cout << "[Scheduler <" << std::hex << scheduler << ">] Holding " << task.events.size() << " Event(s) <P: " << task.priority << "> pending enqueued." << std::endl;
        }
        for (Scheduler::Task const &task : scheduler->_tasksDequeued)
        {
            std::cout << "[Scheduler <" << std::hex << scheduler << ">] Holding " << task.events.size() << " Event(s) <P: " << task.priority << "> pending dequeued." << std::endl;
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
        for (Scheduler::Task const &task : scheduler->_tasksEnqueued)
        {
            Serial.print("[Scheduler <");
            Serial.print((unsigned long) scheduler, HEX);
            Serial.print(">] Holding ");
            Serial.print(task.events.size());
            Serial.print(" Event(s) <P: ");
            Serial.print(task.priority);
            Serial.println("> pending enqueued.");
        }
        for (Scheduler::Task const &task : scheduler->_tasksDequeued)
        {
            Serial.print("[Scheduler <");
            Serial.print((unsigned long) scheduler, HEX);
            Serial.print(">] Holding ");
            Serial.print(task.events.size());
            Serial.print(" Event(s) <P: ");
            Serial.print(task.priority);
            Serial.println("> pending dequeued.");
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
    for (Scheduler::Task const &task : this->_tasks)
    {
        for (Scheduler::Event * const event : task.events)
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
            
            
            if (Identifiable<Daemon>::Instanced(event))
            {
                Daemon * const daemon = static_cast<Daemon *>(event);
                if (daemon->finished()) this->dequeue(daemon);
            }
            else this->dequeue(event);
        }
    }
    
    this->_processPendingDequeueEvents();
    this->_processPendingEnqueueEvents();
}

void Scheduler::_processPendingEnqueueEvents()
{
    // Enqueue any pending Events. It's safe at this point since there's
    // nothing modifying _tasksEnqueued at this point.
    // Enqueued Event has already been called in the enqueue method.
    for (Scheduler::Task const &taskEnqueued : this->_tasksEnqueued)
    {
        // Preemptively attempt to find category (priority Task) in _tasks to save lookups each time.
        // If not found we'll try again in the for loop below only for the first failure.
        Scheduler::Tasks::const_iterator task = this->_tasks.find(Scheduler::Task(taskEnqueued.priority));
        
        for (Scheduler::Event * const event : taskEnqueued.events)
        {
            
            if (task == this->_tasks.end())
            {
                // If no Task for priority was found, create a new Task with the current event.
                this->_tasks.insert(Scheduler::Task(event));
                // Update the task iterator to point to the proper Task.
                task = this->_tasks.find(Scheduler::Task(taskEnqueued.priority));
            } else task->events.insert(event);
            
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
            Scheduler::_AssociateEventToScheduler(event, this);
        }
    }
    
    this->_tasksEnqueued.clear(); // Reset enqueued Events holder
}

void Scheduler::_processPendingDequeueEvents()
{
    // Dequeue any pending events at this point to keep _events set
    // as small as possible when inserting, for better performance.
    for (Scheduler::Task const &taskDequeued : this->_tasksDequeued)
    {
        // Prepare to avoid having to repeat search every time, these are n log( size + n ), I think.
        Scheduler::Tasks::const_iterator const task = this->_tasks.find(Scheduler::Task(taskDequeued.priority));
        Scheduler::Tasks::const_iterator const taskEnqueued = this->_tasksEnqueued.find(Scheduler::Task(taskDequeued.priority));
        
        for (Scheduler::Event * const event : taskDequeued.events)
        {
            
            // Attempt to find the event in the _events set (usual).
            if (task != this->_tasks.end() && task->events.count(event))
            {
                // Any Event based object is unregistered Event destructor.
                task->events.erase(event);
                //this->_tasks.erase(target);
                
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
            if (taskEnqueued != this->_tasksEnqueued.end() && taskEnqueued->events.count(event))
            {
                // Any Event based object is unregistered Event destructor.
                taskEnqueued->events.erase(event);
                
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
    }
    
    this->_tasksDequeued.clear(); // Reset dequeued Events holder
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

bool Scheduler::_ReprioritizeEvent(Scheduler::Event * const event)
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
    
    // No need to reprioritize if singleton set.
    if (scheduler->_tasks.size() > 1) {
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
delegate(nullptr)
{
    Scheduler::_InstanceRegister.insert(this);
}

Scheduler::~Scheduler()
{
    Scheduler::_InstanceRegister.erase(this);
}

