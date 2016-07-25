// =============================================================================
// Scheduler : This class is responsible for scheduling and executing jobs at
// specific times, given some type of clock.
// =============================================================================

#include <vector>

class Scheduler
{
public:
	typedef unsigned long Time;

	// =========================================================================
	// Event: This class is meant to be subclassed for scheduling code.
	// =========================================================================
	class Event {
	public:
		// Realize the implications of this...
		// This means the subclass may change its time, requiring the scheduler
		// to implement a means to update its internal event calling sequence.
		virtual Time time() const = 0;
		virtual bool finished() const = 0;
		virtual int execute(Time const updateTime, Time const updateDelay) = 0;
	};
	
	virtual bool enqueue(Event * const event);
	virtual bool dequeue(Event const * const event);
	
	void update(Time const updateTime);
	
	Scheduler();
	virtual ~Scheduler();
	
protected:
	Time _lastUpdateTime;
	std::vector<Event *> _events;
	
};