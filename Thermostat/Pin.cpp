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
    return _identity;
}

bool Pin::ready() const
{
    return mode() != Pin::Mode::Invalid;
}

Pin::Mode Pin::mode() const
{
    return _mode;
}

bool Pin::setMode(Pin::Mode const mode)
{
    if (this->mode() == Pin::Mode::Invalid) {
#if defined(MJB_DEBUG_LOGGING_PIN)
        MJB_DEBUG_LOG("[Pin <");
        MJB_DEBUG_LOG_FORMAT((unsigned long) this, MJB_DEBUG_LOG_HEX);
        MJB_DEBUG_LOG(">] ERROR: Failed to set pin ");
        MJB_DEBUG_LOG_FORMAT(identity(), MJB_DEBUG_LOG_DEC);
        MJB_DEBUG_LOG_LINE(" mode; currently invalid!");
#endif
        return false;
    }
    return (_mode = mode);
}

Pin::Value Pin::value() const
{
    switch (mode())
    {
        case Pin::Mode::Auto:
        case Pin::Mode::Input: {
#if defined(MJB_ARDUINO_LIB_API)
            // TODO: Potential issue; chaning pin mode in const method.
            pinMode(identity(), INPUT);
#endif

#if defined(MJB_DEBUG_LOGGING_PIN)
#if defined(MJB_HW_IO_PINS_AVAILABLE)
            Pin::Value const value = digitalRead(identity());
#else
            Pin::Value const value = 0;
#endif
            MJB_DEBUG_LOG_LINE("[HW] Set Pin  Mode: INPUT");
            MJB_DEBUG_LOG("[HW] Set Pin Value: ");
            MJB_DEBUG_LOG_LINE(value);
#endif

#if defined(MJB_HW_IO_PINS_AVAILABLE)
            return digitalRead(identity());
#else
            return 0;
#endif
        }   break;
        default: break;
    }
    return _value;
}

bool Pin::setValue(Pin::Value const value)
{
    switch (mode())
    {
        case Pin::Mode::Auto:
        case Pin::Mode::Output: {
            _value = value;
#if defined(MJB_DEBUG_LOGGING_PIN)
            MJB_DEBUG_LOG_LINE("[HW] Set Pin  Mode: OUTPUT");
            MJB_DEBUG_LOG("[HW] Set Pin Value: ");
            MJB_DEBUG_LOG_LINE(value);
#endif

#if defined(MJB_HW_IO_PINS_AVAILABLE)
            pinMode(identity(), OUTPUT);
            digitalWrite(identity(), value);
#endif
        }   return true;
        default: break;
    }
    return false;
}

Pin::Configuration Pin::configuration() const
{
    return {mode(), value()};
}

void Pin::setConfiguration(Pin::Configuration const &configuration)
{
    setMode(configuration.mode);
    setValue(configuration.value);
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
    Pin::Identifier const identity = pin->identity();

    // Note: Count is optimal here due to the fact _pins is a map log(n).
    if (!pin || Pin::_Reserved().count(identity) > 0) {
#if defined(MJB_DEBUG_LOGGING_PIN)
        MJB_DEBUG_LOG("[Pin <");
        MJB_DEBUG_LOG_FORMAT((unsigned long) pin.get(), MJB_DEBUG_LOG_HEX);
        MJB_DEBUG_LOG(">] ERROR: Failed to reserve pin ");
        MJB_DEBUG_LOG_LINE_FORMAT(identity, MJB_DEBUG_LOG_DEC);
#endif
        return false;
    }

    Pin::_Reserved()[identity] = pin;
    return true;
}

bool Pin::_Release(std::shared_ptr<Pin> const &pin)
{
    Pin::Identifier const identity = pin->identity();

    if (!pin || !pin->ready()) {
#if defined(MJB_DEBUG_LOGGING_PIN)
        MJB_DEBUG_LOG("[Pin <");
        MJB_DEBUG_LOG_FORMAT((unsigned long) pin.get(), MJB_DEBUG_LOG_HEX);
        MJB_DEBUG_LOG(">] ERROR: Failed to release pin ");
        MJB_DEBUG_LOG_LINE_FORMAT(identity, MJB_DEBUG_LOG_DEC);
#endif
        return false;
    }

    return Pin::_Reserved().erase(identity);
}

Pin::Pin(Pin::Identifier const identifier):
_identity(identifier),
_value(0),
_mode(Pin::Mode::Auto) // Will be overwritten below; must not be invalid, otherwise setMode fails!
{
#if defined(MJB_HW_IO_PINS_AVAILABLE)
    setMode(Pin::_Reserve(std::static_pointer_cast<Pin>(self()))? Pin::Mode::Auto : Pin::Mode::Invalid);
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
    setMode(Pin::_Release(std::static_pointer_cast<Pin>(self()))? Pin::Mode::Invalid : mode());
}

