// =============================================================================
// Actuator : This class abstracts the functionality of Actuators, only being
// able to send output signals via the I/O rail.
// =============================================================================

#include "Pin.hpp"

class Actuator : public Pin::Set
{
public:

	class Sequence // Timed states?
	{
	public:
		Sequence(
	};

	bool performSequence(Sequence const sequence);
	
protected:
};