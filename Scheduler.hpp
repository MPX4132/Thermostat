// =============================================================================
// Scheduler : This class is responsible for scheduling and executing jobs at
// specific times, given some type of clock.
// =============================================================================


class Scheduler
{
public:
	typedef unsigned long int Time;

	// =========================================================================
	// Task: This class is meant to be subclassed for scheduling code.
	// =========================================================================
	class Task {
	public:
		virtual bool finished() const = 0;
		virtual bool execute(Time const updateTime, Time const updateDelay) = 0;
	};
	
	// Consider enqueueing a Task object and use Job to have a time 
	bool enqueue(Task * const task, Time const startTime);
	bool dequeue(Task * const task);
	
	static bool Update(Time const updateTime);
	
protected:
	struct Job {
		Time const time;
		Task * const task;
		Job(Task * const task, Time const time);
	};

	static Time const _lastUpdateTime;
	
};