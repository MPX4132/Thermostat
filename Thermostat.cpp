// =============================================================================
// Thermostat : Implementation
// =============================================================================

#include "Thermostat.hpp"

Thermostat::Mode Thermostat::mode() const
{
	return _mode;
}

void Thermostat::setMode(const Thermostat::Mode mode)
{

}

Temperature<int> Thermostat::temperature() const
{
	return _temperature;
}

void Thermostat::setTemperature(const Temperature<int>& temperature)
{
	_temperature = temperature;
}

Thermostat::~Thermostat()
{
}

Thermostat::Thermostat()
{
}