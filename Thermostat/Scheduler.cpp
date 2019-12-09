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
std::set<Scheduler * const> &Scheduler::_InstanceRegister()
{
    static std::set<Scheduler * const> _instanceRegister;
    return _instanceRegister;
}

#if defined(MJB_MULTITHREAD_CAPABLE)
std::mutex Scheduler::_InstanceRegisterLock;
#endif

// =============================================================================
// Scheduler::Event : Implementation
// =============================================================================
Scheduler::Time Scheduler::Event::executeTime() const
{
    return _executeTime;
}

bool Scheduler::Event::setExecuteTime(Scheduler::Time const executeTime)
{
    Time const lastExecuteTime = this->executeTime();

    // If the event's execute time changed, we've got to reprioritize.
    if (lastExecuteTime != executeTime)
    {
        std::weak_ptr<Scheduler> scheduler = _scheduler;
        if (unschedule())
        {
            _executeTime = executeTime; // Update execute time.
            _executeTimeDidChange(executeTime - lastExecuteTime);

             return schedule(scheduler);
        }
    }

    return false;
}

std::weak_ptr<Scheduler> const &Scheduler::Event::scheduler() const
{
    return _scheduler;
}

bool Scheduler::Event::schedule(std::weak_ptr<Scheduler> const &scheduler)
{
    // Retrieve the scheduler to enqueue the event.
    std::shared_ptr<Scheduler> const valid_scheduler = scheduler.lock();

    return valid_scheduler && valid_scheduler->enqueue(std::static_pointer_cast<Event>(self()));
}

bool Scheduler::Event::unschedule()
{
    // Retrieve the scheduler to dequeue the event.
    std::shared_ptr<Scheduler> const valid_scheduler = _scheduler.lock();

    return valid_scheduler && valid_scheduler->dequeue(std::static_pointer_cast<Event>(self()));
}

bool Scheduler::Event::setScheduler(std::weak_ptr<Scheduler> const &scheduler)
{
    std::shared_ptr<Scheduler> const _valid_scheduler = _scheduler.lock();

    if ((!_valid_scheduler) || (!_valid_scheduler->scheduled(std::static_pointer_cast<Event>(self()))))
    {
        _scheduler = scheduler;
        return true;
    }
    return false;
}

void Scheduler::Event::_executeTimeDidChange(Scheduler::Time const executeTimeDelta)
{
    // The following done to suppress unused variable warnings.
    (void) executeTimeDelta;
}

Scheduler::Event::Event(Scheduler::Time const executeTime):
_executeTime(executeTime)
{
    Identifiable<Event>::Register(this); // RTTI Substitute
}

Scheduler::Event::~Event()
{
    Identifiable<Event>::Unregister(this); // RTTI Substitute
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
    Scheduler::Time const lastExecuteTimeInterval = this->executeTimeInterval();

    this->_executeTimeInterval = executeTimeInterval;
    this->_executeTimeIntervalDidChange(executeTimeInterval - lastExecuteTimeInterval);
}

bool Scheduler::Daemon::finished() const
{
    return false;
}

void Scheduler::Daemon::_executeTimeIntervalDidChange(Scheduler::Time const executeTimeIntervalDelta)
{
    // The following done to suppress unused variable warnings.
    (void) executeTimeIntervalDelta;
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
void SchedulerDelegate::schedulerStartingEvent(Scheduler * const scheduler,
                                               std::shared_ptr<Scheduler::Event> const &event)
{
    // The following done to suppress unused variable warnings.
    (void) scheduler;
    (void) event;
    return; // By default, skip.
}

void SchedulerDelegate::schedulerCompletedEvent(Scheduler * const scheduler,
                                                std::shared_ptr<Scheduler::Event> const &event,
                                                int result)
{
    // The following done to suppress unused variable warnings.
    (void) scheduler;
    (void) event;
    (void) result;
    return; // By default, skip.
}

void SchedulerDelegate::schedulerEnqueuedEvent(Scheduler * const scheduler,
                                               std::shared_ptr<Scheduler::Event> const &event)
{
    // The following done to suppress unused variable warnings.
    (void) scheduler;
    (void) event;
    return; // By default, skip.
}

void SchedulerDelegate::schedulerDequeuedEvent(Scheduler * const scheduler,
                                               std::shared_ptr<Scheduler::Event> const &event)
{
    // The following done to suppress unused variable warnings.
    (void) scheduler;
    (void) event;
    return; // By default, skip.
}

SchedulerDelegate::~SchedulerDelegate()
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

Scheduler::Task::Task(std::shared_ptr<Scheduler::Event> const &event):
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
bool Scheduler::enqueue(std::shared_ptr<Scheduler::Event> const &event)
{
    bool enqueuedEvent = false; // Assume failure.

    if (Scheduler::_EnqueueTasksEvent(_tasks, event))
    {
        if (event->setScheduler(std::static_pointer_cast<Scheduler>(self())))
        {
            _delegate([this, &event](std::shared_ptr<SchedulerDelegate> const &delegate) -> bool {
                delegate->schedulerEnqueuedEvent(this, event);
                return true;
            });

            enqueuedEvent = true;
        }
    }

    return enqueuedEvent;
}

bool Scheduler::dequeue(std::shared_ptr<Scheduler::Event> const &event)
{
    bool dequeuedEvent = false; // Assume failure.

    if (Scheduler::_DequeueTasksEvent(_tasks, event))
    {
        if (event->setScheduler(std::weak_ptr<Scheduler>()))
        {
            _delegate([this, &event](std::shared_ptr<SchedulerDelegate> const &delegate) -> bool {
                delegate->schedulerDequeuedEvent(this, event);
                return true;
            });

            dequeuedEvent = true;
        }
    }

    return dequeuedEvent;
}

bool Scheduler::scheduled(std::shared_ptr<Scheduler::Event> const &event) const
{
    std::set<Scheduler::Task>::const_iterator const &task_i = _tasks.find(event->executeTime());
    return (task_i != this->_tasks.end()) && task_i->events.count(event) > 0;
}

void Scheduler::UpdateInstances(Scheduler::Time const time)
{
#if defined(MJB_DEBUG_LOGGING_SCHEDULER)
    MJB_DEBUG_LOG_LINE("");
    MJB_DEBUG_LOG_LINE("==============");
#endif
    
    for (Scheduler * const scheduler : Scheduler::_InstanceRegister())
    {

#if defined(MJB_DEBUG_LOGGING_SCHEDULER)
        MJB_DEBUG_LOG("[scheduler <");
        MJB_DEBUG_LOG_FORMAT((unsigned long) scheduler, MJB_DEBUG_LOG_HEX);
        MJB_DEBUG_LOG_LINE("> is starting]");
        for (Scheduler::Task const &task : scheduler->_tasks)
        {
            MJB_DEBUG_LOG("[scheduler <");
            MJB_DEBUG_LOG_FORMAT((unsigned long) scheduler, MJB_DEBUG_LOG_HEX);
            MJB_DEBUG_LOG(">] holding ");
            MJB_DEBUG_LOG(task.events.size());
            MJB_DEBUG_LOG(" event(s) <p: ");
            MJB_DEBUG_LOG(task.priority);
            MJB_DEBUG_LOG_LINE("> pending runtime.");
        }
#endif
        scheduler->_processEventsForTime(time);
        

#if defined(MJB_DEBUG_LOGGING_SCHEDULER)
        MJB_DEBUG_LOG("[scheduler <");
        MJB_DEBUG_LOG_FORMAT((unsigned long) scheduler, MJB_DEBUG_LOG_HEX);
        MJB_DEBUG_LOG_LINE("> is pausing]");
#endif
    }
    
#if defined(MJB_DEBUG_LOGGING_SCHEDULER)
    MJB_DEBUG_LOG_LINE("==============\n");
#endif
}

void Scheduler::_processEventsForTime(Scheduler::Time const time)
{
    // check for potential time overflow, start with those & reset the overflowed tasks set.
    if (_lastTime > time)
    {
        _tasks = _tasksOverflowed;
        _tasksOverflowed.clear();
    }
    
    // We'll be copying Event instances which will be executed this cycle, to the variable below,
    // for safety, since we might modify the _tasks container (leading to undefined behaviour).
    Scheduler::Tasks executableTasks; // Stores Task instances to be executed this update cycle.
    
    // Get a copy of all Task instances (pointers) we'll be executing this update cycle for safety.
    for (Scheduler::Task const &task : _tasks)
    {
        // All Event instances of equal execution time are stored in the same Task instance,
        // these Task instances are stored in std::map and are sorted (prioritized) incrementally.
        // That means Task instances with lower priority come first, beacuse Event instances
        // containing lower (earlier) execution times must be executed before those of higher
        // (later) execution times.
        // Only Task instances with priority less than, or equal to, time, must be executed now.
        if (task.priority > time) break;
        
        executableTasks.insert(task);
    }
    
    for (Scheduler::Task const &task : executableTasks)
    {
        for (std::shared_ptr<Scheduler::Event> const &event : task.events)
        {
            _delegate([this, &event](std::shared_ptr<SchedulerDelegate> const &delegate) -> bool {
                delegate->schedulerStartingEvent(this, event);
                return true;
            });

#if defined(MJB_DEBUG_LOGGING_SCHEDULER)
            MJB_DEBUG_LOG_LINE("\n==========");
            MJB_DEBUG_LOG("[Scheduler <");
            MJB_DEBUG_LOG_FORMAT((unsigned long) this, MJB_DEBUG_LOG_HEX);
            MJB_DEBUG_LOG(">] Event <");
            MJB_DEBUG_LOG_FORMAT((unsigned long) event.get(), MJB_DEBUG_LOG_HEX);
            MJB_DEBUG_LOG_LINE("> running.");
#endif

            int const error = event->execute(time);
            
            if (error)
            {

#if defined(MJB_DEBUG_LOGGING_SCHEDULER)
                MJB_DEBUG_LOG("[Scheduler <");
                MJB_DEBUG_LOG_FORMAT((unsigned long) this, MJB_DEBUG_LOG_HEX);
                MJB_DEBUG_LOG(">] Event <");
                MJB_DEBUG_LOG_FORMAT((unsigned long) event.get(), MJB_DEBUG_LOG_HEX);
                MJB_DEBUG_LOG("> returned error code ");
                MJB_DEBUG_LOG_LINE(error);
#endif
            }

#if defined(MJB_DEBUG_LOGGING_SCHEDULER)
            MJB_DEBUG_LOG("[Scheduler <");
            MJB_DEBUG_LOG_FORMAT((unsigned long) this, MJB_DEBUG_LOG_HEX);
            MJB_DEBUG_LOG(">] Event <");
            MJB_DEBUG_LOG_FORMAT((unsigned long) event.get(), MJB_DEBUG_LOG_HEX);
            MJB_DEBUG_LOG_LINE("> halting.");
            MJB_DEBUG_LOG_LINE("==========\n");
#endif

            _delegate([this, &event, error](std::shared_ptr<SchedulerDelegate> const &delegate) -> bool {
                delegate->schedulerCompletedEvent(this, event, error);
                return true;
            });

            // Check for special case, being Daemon instances.
            if (Identifiable<Daemon>::Instanced(event.get()))
            {
                // Since this is a Daemon, and Daemons repeat until finished,
                // calcualte next execution time and request scheduler priority update.
                std::shared_ptr<Daemon> daemon = std::static_pointer_cast<Scheduler::Daemon>(event);

                // Daemon * const daemon = static_cast<Daemon *>(event.get());
                if (!daemon->finished())
                {
                    // Calculate the Daemon instance's next execution time.
                    Scheduler::Time const executeTime = time + daemon->executeTimeInterval();
                    
                    // Check for potential Scheduler::Time integer overflow.
                    if (executeTime < time)
                    {
                        // Remove from main Task instance set and reinsert into overflowed set.
                        if (Scheduler::_DequeueTasksEvent(_tasks, event))
                        {
                            // Since we've dequeued the event, it's not going to attempt to reprioritize.
                            event->setExecuteTime(executeTime); // Preventing reprioritizing here.
                            Scheduler::_EnqueueTasksEvent(_tasksOverflowed, event);
                        }
                    }
                    // Update the execution time, but notice this reprioritizes the event.
                    else daemon->setExecuteTime(executeTime);
                }
                // These will only be dequeued with notification when they're really done.
                else dequeue(event);
            }
            // These will only be dequeued with notification when they're really done.
            else dequeue(event);
        }
    }
    
    _lastTime = time;
}

bool Scheduler::_EnqueueTasksEvent(Scheduler::Tasks &tasks,
                                   Scheduler::Task::Event const &event)
{
    // Passing priority to prevent the task instance inserting the element (temporary task obj.)
    // We're just concerend with getting the Task instance matching this one's priority.
    // NOTE: A set can only return either const_iterator, or const iterator implicitly, but we
    // require events to be modifiable, so we work around it using the mutable keyword.
    // The "mutable" keyword works here since it doesn't affect task's priority/position in the set.
    Scheduler::Tasks::const_iterator const task = tasks.find(Scheduler::Task(event->executeTime()));
    
    if (task == tasks.end())
    {   // If no tasks with corresponding priority was found, create one with the event.
        tasks.insert(Scheduler::Task(event));
    }
    else
    {   // If a tasks with corresponding priority was found, add the event to its events.
        task->events.insert(event);
    }

#if defined(MJB_DEBUG_LOGGING_SCHEDULER)
    MJB_DEBUG_LOG("[Scheduler] Event <");
    MJB_DEBUG_LOG_FORMAT((unsigned long) event.get(), MJB_DEBUG_LOG_HEX);
    MJB_DEBUG_LOG_LINE("> enqueued.");
#endif
    
    return true;
}

bool Scheduler::_DequeueTasksEvent(Scheduler::Tasks &tasks,
                                   Scheduler::Task::Event const &event)
{
    bool dequeuedTaskEvent = false; // Assume we won't find the event within any task.
    Scheduler::Tasks::const_iterator const task = tasks.find(Scheduler::Task(event->executeTime()));

    // If a task matching the event's priority exsists in the set of tasks,
    // the event might be contained within that priority task.
    // If a task matching the event's priority doesn't exist in the set of tasks,
    // we can assume the event was never enqueued due to a lack of priority task.
    if ((task != tasks.end()) && (task->events.count(event) > 0))
    {
        // If the task has no remaining events, erase it from the tasks' set,
        // otherwise, remove the dequeued event from the task's events.
        if (task->events.size() > 1)
        { // If the task has more than just the event being erased, only erase the event.
            task->events.erase(event);
        }
        else
        { // If the task has only the event, save processing time by erasing it instead.
            tasks.erase(task);
        }

        dequeuedTaskEvent = true;

#if defined(MJB_DEBUG_LOGGING_SCHEDULER)
        MJB_DEBUG_LOG("[Scheduler] Event <");
        MJB_DEBUG_LOG_FORMAT((unsigned long) event.get(), MJB_DEBUG_LOG_HEX);
        MJB_DEBUG_LOG_LINE("> dequeued.");
#endif
    }

#if defined(MJB_DEBUG_LOGGING_SCHEDULER)
    if (!dequeuedTaskEvent)
    {
        MJB_DEBUG_LOG("[Scheduler] WARNING: THE EVENT <");
        MJB_DEBUG_LOG_FORMAT((unsigned long) event.get(), MJB_DEBUG_LOG_HEX);
        MJB_DEBUG_LOG_LINE("> WASN'T FOUND!");
    }
#endif

    return dequeuedTaskEvent;
}

Scheduler::Scheduler():
_lastTime(0)
{
#if defined(MJB_MULTITHREAD_CAPABLE)
    // Enable the _InstanceRegisterLock immediately upon entering the constructor.
    // NOTICE: The lock is driven by the code block, released on block-exit.
    std::lock_guard<std::mutex> const lock(Scheduler::_InstanceRegisterLock);
#endif
    Scheduler::_InstanceRegister().insert(this);
}

Scheduler::~Scheduler()
{
#if defined(MJB_MULTITHREAD_CAPABLE)
    // Enable the _InstanceRegisterLock immediately upon entering the destructor.
    // NOTICE: The lock is driven by the code block, released on block-exit.
    std::lock_guard<std::mutex> const lock(Scheduler::_InstanceRegisterLock);
#endif
    Scheduler::_InstanceRegister().erase(this);
}

