// =============================================================================
// Sensor : This class abstracts the functionality of Sensor I/O modules.
// The Sensor class is capable of reading data from a module in the I/O rail.
// =============================================================================

#include "Data.hpp"

class Sensor : protected Actuator
{
public:
	Data evaluate();
	bool ready();
	Sensor();
};