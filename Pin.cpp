// =============================================================================
// Pin : Implementation
// =============================================================================

#include "Pin.hpp"

Pin::Identifier Pin::identity() const
{
	return this->_identity;
}

bool Pin::ready() const
{
	return this->mode() != Pin::Mode::Invalid;
}

Pin::Mode Pin::mode() const
{
	return this->_mode;
}

bool Pin::setMode(Pin::Mode const mode)
{
	
}

