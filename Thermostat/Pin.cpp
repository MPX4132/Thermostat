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
std::map<Pin::Identifier, Pin const *> Pin::_Reserved;


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
#ifdef HARDWARE_INDEPENDENT
#ifdef DEBUG
            std::cout << "[HW] Set Pin  Mode: INPUT" << std::endl;
            std::cout << "[HW] Get Pin Value: 0 (hardcoded zero)" << std::endl;
#endif
            return 0;
#else // Hardware Dependent
#ifdef DEBUG
            Serial.println("[HW] Set Pin  Mode: INPUT");
            pinMode(this->identity(), INPUT);
            Pin::Value const value = digitalRead(this->identity());
            
            return value;
#else
            return digitalRead(this->identity());
#endif
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
#ifdef DEBUG
#ifdef HARDWARE_INDEPENDENT
            std::cout << "[HW] Set Pin  Mode: OUTPUT" << std::endl;
            std::cout << "[HW] Set Pin Value: " << state << std::endl;
#else
            Serial.println("[HW] Set Pin  Mode: OUTPUT");
            Serial.print("[HW] Set Pin Value: ");
            Serial.println(state);
#endif
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

bool Pin::_Reserve(Pin const * const pin)
{
    if (!pin || Pin::_Reserved.count(pin->identity()) > 0) return false;
    Pin::_Reserved[pin->identity()] = pin;
    return true;
}

bool Pin::_Release(Pin const * const pin)
{
    return pin && Pin::_Reserved.erase(pin->identity());
}

Pin::Pin(Pin::Identifier const identifier):
_identity(identifier),
_value(0),
_mode(Pin::Mode::Auto)
{
    this->setMode(Pin::_Reserve(this)? Pin::Mode::Auto : Pin::Mode::Invalid);
}

Pin::~Pin()
{
    Pin::_Release(this);
}

