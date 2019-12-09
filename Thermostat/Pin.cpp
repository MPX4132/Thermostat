//
//  Pin.cpp
//  Thermostat
//
//  Created by Matias Barcenas on 11/12/16.
//  Copyright © 2016 Matias Barcenas. All rights reserved.
//

#include "Pin.hpp"

// =============================================================================
// Pin : Static Variables Declaration
// =============================================================================
Pin::Set &Pin::_Reserved()
{
    // TODO: Move out as static class member when the static-initialization
    // order fiasco is fucking finally resovled.
    static Pin::Set _reserved;
    return _reserved;
}


// =============================================================================
// Pin : Implementation
// =============================================================================
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
    if (this->mode() == Pin::Mode::Invalid) return false;
    return (this->_mode = mode);
}

Pin::Value Pin::state() const
{
    switch (this->mode())
    {
        case Pin::Mode::Auto:
        case Pin::Mode::Input: {
#if defined(MJB_ARDUINO_LIB_API)
            // TODO: Potential issue; chaning pin mode in const method.
            pinMode(this->identity(), INPUT);
#endif

#if defined(MJB_DEBUG_LOGGING_PIN)
#if defined(MJB_HW_IO_PINS_AVAILABLE)
            Pin::Value const value = digitalRead(this->identity());
#else
            Pin::Value const value = 0;
#endif
            MJB_DEBUG_LOG_LINE("[HW] Set Pin  Mode: INPUT");
            MJB_DEBUG_LOG("[HW] Set Pin Value: ");
            MJB_DEBUG_LOG_LINE(value);
#endif

#if defined(MJB_HW_IO_PINS_AVAILABLE)
            return digitalRead(this->identity());
#else
            return 0;
#endif
        }   break;
        default: break;
    }
    return this->_value;
}

bool Pin::setState(Pin::Value const state)
{
    switch (this->mode())
    {
        case Pin::Mode::Auto:
        case Pin::Mode::Output: {
            this->_value = state;
#if defined(MJB_DEBUG_LOGGING_PIN)
            MJB_DEBUG_LOG_LINE("[HW] Set Pin  Mode: OUTPUT");
            MJB_DEBUG_LOG("[HW] Set Pin Value: ");
            MJB_DEBUG_LOG_LINE(state);
#endif

#if defined(MJB_HW_IO_PINS_AVAILABLE)
            pinMode(this->identity(), OUTPUT);
            digitalWrite(this->identity(), state);
#endif
        }   return true;
        default: break;
    }
    return false;
}

Pin::Configuration Pin::configuration() const
{
    return {this->mode(), this->state()};
}

void Pin::setConfiguration(Pin::Configuration const &configuration)
{
    this->setMode(configuration.mode);
    this->setState(configuration.value);
}

Pin::Set Pin::MakeSet(Pin::Arrangement const &pins)
{
    Pin::Set set;
    for (Pin::Identifier const identifier : pins)
    {
        set.emplace(identifier, std::make_shared<Pin>(identifier));
    }
    return set;
}

bool Pin::_Reserve(std::shared_ptr<Pin> const &pin)
{
    // Note: Count is optimal here due to the fact _pins is a map log(n).
    if (!pin || Pin::_Reserved().count(pin->identity()) > 0) return false;
    Pin::_Reserved()[pin->identity()] = pin;
    return true;
}

bool Pin::_Release(std::shared_ptr<Pin> const &pin)
{
    if (!pin || !pin->ready()) return false;
    Pin::Identifier const identity = pin->identity();
    // Invalidate current owner, since it's no longer in control.
    Pin::_Reserved()[identity]->_mode = Pin::Mode::Invalid;
    return Pin::_Reserved().erase(identity);
}

Pin::Pin(Pin::Identifier const identifier):
_identity(identifier),
_value(0),
_mode(Pin::Mode::Invalid)
{
#if defined(MJB_HW_IO_PINS_AVAILABLE)
    this->setMode(Pin::_Reserve(std::static_pointer_cast<Pin>(self()))? Pin::Mode::Auto : Pin::Mode::Invalid);
#endif
}

Pin::Pin():
_identity(0),
_value(0),
_mode(Pin::Mode::Invalid)
{
    
}

Pin::~Pin()
{
    Pin::_Release(std::static_pointer_cast<Pin>(self()));
}

