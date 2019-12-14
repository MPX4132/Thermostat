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
std::set<Scheduler *> &Scheduler::_InstanceRegister()
{
    // TODO: Make type below "Scheduler * const" when compiler gets its shit together.
    static std::set<Scheduler *> _instanceRegister;
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
    Scheduler::Time const lastExecuteTime = this->executeTime();

    bool operation_result = false; // Assume failure by default.

    // If the event's execute time changed, we've got to reprioritize.
    if (lastExecuteTime != executeTime)
    {
        std::shared_ptr<Scheduler> const scheduler = _scheduler.lock();
        bool const scheduler_is_invalid = (scheduler == nullptr);
        
        if (scheduler_is_invalid || unschedule())
        {
            _executeTime = executeTime; // Update execute time.

            // NOTICE: Re-scheduling must be done prior to any notifications,
            // else, the operation fails if called from _executeTimeDidChange.
            operation_result = scheduler_is_invalid || schedule(scheduler);

            _executeTimeDidChange(executeTime - lastExecuteTime);
        }
    }

    return operation_result;
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
    std::shared_ptr<Scheduler> const valid_scheduler = scheduler.lock();

    if ((!valid_scheduler) || valid_scheduler->scheduled(std::static_pointer_cast<Event>(self())))
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
    if (_enqueueEvent(event))
    {
        _delegate([this, &event](std::shared_ptr<SchedulerDelegate> const &delegate) -> bool {
            delegate->schedulerEnqueuedEvent(this, event);
            return true;
        });
        return true;
    }
    return false;
}

bool Scheduler::dequeue(std::shared_ptr<Scheduler::Event> const &event)
{
    if (_dequeueEvent(event))
    {
        _delegate([this, &event](std::shared_ptr<SchedulerDelegate> const &delegate) -> bool {
            delegate->schedulerDequeuedEvent(this, event);
            return true;
        });
        return true;
    }
    return false;
}

bool Scheduler::scheduled(std::shared_ptr<Scheduler::Event> const &event) const
{
    return (Scheduler::_GetEventLocation(this, event).first != nullptr);
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
        MJB_DEBUG_LOG("[Scheduler <");
        MJB_DEBUG_LOG_FORMAT((unsigned long) scheduler, MJB_DEBUG_LOG_HEX);
        MJB_DEBUG_LOG_LINE("> is starting]");
        for (Scheduler::TaskSet const * const taskSet : {scheduler->_taskSetPrimary, scheduler->_taskSetSecondary})
        {
            for (Scheduler::Task const &task : *taskSet)
            {
                MJB_DEBUG_LOG("[Scheduler <");
                MJB_DEBUG_LOG_FORMAT((unsigned long) scheduler, MJB_DEBUG_LOG_HEX);
                MJB_DEBUG_LOG(">] TaskSet <");
                MJB_DEBUG_LOG_FORMAT((unsigned long) taskSet, MJB_DEBUG_LOG_HEX);
                MJB_DEBUG_LOG("> Task <");
                MJB_DEBUG_LOG_FORMAT((unsigned long) &task, MJB_DEBUG_LOG_HEX);
                MJB_DEBUG_LOG("> holding ");
                MJB_DEBUG_LOG_FORMAT(task.events.size(), MJB_DEBUG_LOG_DEC);
                MJB_DEBUG_LOG(" event(s) <p: ");
                MJB_DEBUG_LOG(task.priority);
                MJB_DEBUG_LOG_LINE("> pending runtime.");
            }
        }
#endif
        scheduler->_processEventsForTime(time);
        

#if defined(MJB_DEBUG_LOGGING_SCHEDULER)
        MJB_DEBUG_LOG("[Scheduler <");
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
    // Copy Task instances to be executed this cycle to the temporary task set declared below.
    // NOTE: Elements are copied for safety since the _tasks instance may need to be modified
    // on event execution, which may lead to undefined behavior while being iterated over.
    Scheduler::TaskSet executableTasks;

    if (_lastTime > time) // Check for time overflow.
    {
        // On time overflow, update the current and previous task sets.
        _taskSetSecondary = _taskSetPrimary;
        _taskSetPrimary = _taskSets + (((_taskSetPrimary + 1) - _taskSets) % Scheduler::_TaskSetsMax);

        // The tasks that didn't get executed and didn't overflow must be executed immediately.
        // WARNING: The following must be done after current/previous set pointers are updated.
        // This is due to the fact the events could reschedule, and they should schedule on the
        // updated _taskSetPrimary, otherwise it will not properly execute.
        executableTasks = *_taskSetSecondary;

        // NOTE: No tasks for _taskPrevious are removed at this point since we'll have to
        // dequeue events and potentially requeue them in the different, updated, task set.

        // Execute the tasks that didn't get to execute, and didn't overflow to the next cycle.
        _processTasksForTime(executableTasks, time);

        executableTasks.clear(); // Reset to prepare to execute current tasks.
    }

    // Iterate over the set up to where the task priority is within the executable priorities.
    // All priorities that are within that range will be added to the executable tasks set.
    for (Scheduler::Task const &task : (*_taskSetPrimary))
    {
        // All Event instances of equal execution time are stored in the same Task instance,
        // these Task instances are stored in std::set and are sorted (prioritized) incrementally.
        // That means Task instances with lower priority come first, because Event instances
        // containing lower (earlier) execution times must be executed before those of higher
        // (later) execution times.

        // Only Task instances with priority less than, or equal to, time, must be executed now.
        // Stop iterating at the point where the priority threshold is met.
        if (task.priority > time) break;
        
        executableTasks.insert(task);
    }

    _processTasksForTime(executableTasks, time);

    _lastTime = time;
}

void Scheduler::_processTasksForTime(Scheduler::TaskSet const &tasks, Scheduler::Time const time)
{
    for (Scheduler::Task const &task : tasks)
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
                        if (_dequeueEvent(event))
                        {
                            // Since we've dequeued the event, it's not going to attempt to reprioritize.
                            event->setExecuteTime(executeTime); // Preventing reprioritizing here.
                            _enqueueEvent(event, _taskSetSecondary);
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
}

bool Scheduler::_enqueueEvent(std::shared_ptr<Event> const &event, Scheduler::TaskSet * const taskSet)
{
    bool operationSuccess = false; // Assume operation failed by default.

    if (event != nullptr) // Only attempt operation with valid event.
    {
        // NOTE: Constructing temporary Task instance to check for existance within the TaskSet.
        // NOTE: Sets can only return const_iterator or const iterator implicity, however,
        // we require a mutable EventPtrSet to add the new event; the workaround is "mutable".
        // The mutable keyword works since it doesn't affect the task's priority in the set.
        // The considered TaskSet is taskSet if valid, or _taskSetPrimary by default.
        Scheduler::EventLocation const origin = taskSet?
        std::make_pair(taskSet, taskSet->find(Scheduler::Task(event->executeTime()))) :
        std::make_pair(_taskSetPrimary, _taskSetPrimary->find(Scheduler::Task(event->executeTime())));

        // Check for TaskSet instance existance.
        if (origin.first != nullptr)
        {
            // On valid TaskSet instance, attempt inserting the event to it.

            // Check for existing Task instance with corresponding priority.
            if (origin.second == origin.first->end())
            {
                // If no matching Task instance exists, insert it with event.
                std::pair<Scheduler::TaskSet::const_iterator, bool> result(origin.first->insert(Scheduler::Task(event)));

                // The operation succeeds when the event has accepted the scheduler.
                operationSuccess = result.second && event->setScheduler(std::static_pointer_cast<Scheduler>(self()));

                // On failure to accept scheduler, restore original TaskSet state.
                if (!operationSuccess) origin.first->erase(result.first);
            }
            else
            {
                // If matching Task instance exists, insert event to it.
                std::pair<Scheduler::EventPtrSet::const_iterator, bool> result(origin.second->events.insert(event));

                // The operation succeeds when the event has accepted the scheduler.
                operationSuccess = result.second && event->setScheduler(std::static_pointer_cast<Scheduler>(self()));

                // On failure to accept scheduler, restore original Task state.
                if (!operationSuccess) origin.second->events.erase(result.first);
            }
        }
    }

#if defined(MJB_DEBUG_LOGGING_SCHEDULER)
    MJB_DEBUG_LOG((operationSuccess? "[Scheduler] Event <" : "[Scheduler] ERROR: Event <"));
    MJB_DEBUG_LOG_FORMAT((unsigned long) event.get(), MJB_DEBUG_LOG_HEX);
    MJB_DEBUG_LOG_LINE((operationSuccess? "> enqueued." : " > failed to enqueue!"));
#endif

    return operationSuccess;
}

bool Scheduler::_dequeueEvent(std::shared_ptr<Event> const &event, Scheduler::TaskSet * const taskSet)
{
    bool operationSuccess = false; // Assume operation failed by default.

    if (event != nullptr) // Only attempt operation with valid event.
    {
        // The considered TaskSet is taskSet if valid, or any TaskSet by default.
        Scheduler::EventLocation const origin = taskSet?
        std::make_pair(taskSet, taskSet->find(Scheduler::Task(event->executeTime()))) :
        Scheduler::_GetEventLocation(this, event);

        // Check for existance of TaskSet, and a valid iterator within it.
        if ((origin.first != nullptr) && (origin.second != origin.first->end()))
        {
            // If existance confirmed, attempt erasing.
            if (origin.second->events.erase(event) > 0)
            {
                // On successful erase, attempt to clear event scheduler.
                if (event->setScheduler(std::weak_ptr<Scheduler>()))
                {
                    // On successful clear, check if Task is empty.
                    if (origin.second->events.empty())
                    {
                        // If the task is empty, remove it.
                        origin.first->erase(origin.second);
                    }
                    operationSuccess = true;
                }
                else
                {
                    // On failure, attempt to restore original TaskSet state.
                    origin.second->events.insert(event);
                }
            }
        }
    }

#if defined(MJB_DEBUG_LOGGING_SCHEDULER)
    MJB_DEBUG_LOG((operationSuccess? "[Scheduler] Event <" : "[Scheduler] ERROR: Event <"));
    MJB_DEBUG_LOG_FORMAT((unsigned long) event.get(), MJB_DEBUG_LOG_HEX);
    MJB_DEBUG_LOG_LINE((operationSuccess? "> dequeued." : " > failed to dequeue!"));
#endif

    return operationSuccess;
}

Scheduler::EventLocation Scheduler::_GetEventLocation(Scheduler const * const scheduler,
                                                      std::shared_ptr<Event> const &event)
{
    if ((scheduler != nullptr) && (event != nullptr))
    {
        for (TaskSet * const taskSet : {scheduler->_taskSetPrimary, scheduler->_taskSetSecondary})
        {
            Scheduler::TaskSet::const_iterator const &taskSetIterator = taskSet->find(Scheduler::Task(event->executeTime()));
            if ((taskSetIterator != taskSet->end()) && (taskSetIterator->events.count(event) > 0))
            {
                return Scheduler::EventLocation(taskSet, taskSetIterator);
            }
        }
    }
    return Scheduler::EventLocation();
}

Scheduler::Scheduler():
_taskSetPrimary(&_taskSets[0]),
_taskSetSecondary(&_taskSets[1]),
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

