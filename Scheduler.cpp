// =============================================================================
// Scheduler : Implementation
// =============================================================================

#include "Scheduler.hpp"

bool Scheduler::enqueue(Scheduler::Event * const event)
{
	_events.push_back(event);
}

bool Scheduler::dequeue(Scheduler::Event const * const event)
{
	std::vector<Event *>::const_iterator targetEvent = this->_events.begin();
	
	while (targetEvent++ != this->_events.end())
	{
		if (event != targetEvent) continue;
		
		this->_events.erase(targetEvent);
		return true;
	}
	
	return false;
}

// Simple linear updater... I'll optimize this later.
void Scheduler::update(Scheduler::Time const updateTime)
{
	std::vector<Event *>::iterator event = this->_events.begin();
	
	while (event++ != this->_events.end())
	{
		Time const eventTime = event->time();
		
		if (updateTime >= eventTime)
			event->execute(updateTime, updateTime - eventTime);
		
		if (event->finished()) this->_events.erase(event);
	}
	
	this->_lastUpdateTime = updateTime;
}

Scheduler::Scheduler():
_lastUpdateTime(0)
{
}

Scheduler::~Scheduler()
{
}