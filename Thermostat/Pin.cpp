//
//  Pin.cpp
//  Thermostat
//
//  Created by Matias Barcenas on 11/12/16.
//  Copyright © 2016 Matias Barcenas. All rights reserved.
//

#include "Pin.hpp"

// =============================================================================
#pragma mark - Pin : Static Variables Declaration
// =============================================================================
std::map<Pin::Identifier, Pin const *> Pin::_Reserved;


// =============================================================================
#pragma mark - Pin : Implementation
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
    return this->_mode = mode;
}

Pin::Value Pin::state() const
{
    switch (this->mode())
    {
        case Pin::Mode::Auto:
        case Pin::Mode::Input:
            pinMode(this->identity(), INPUT);
            return digitalRead(this->identity());
        default: break;
    }
    return this->_value;
}

bool Pin::setState(Pin::Value const state)
{
    switch (this->mode())
    {
        case Pin::Mode::Auto:
        case Pin::Mode::Output:
            pinMode(this->identity(), OUTPUT);
            digitalWrite(this->identity(), (this->_value = state));
            return true;
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
_mode(Pin::Mode::Auto),
_value(0)
{
    this->setMode(Pin::_Reserve(this)? Pin::Mode::Auto : Pin::Mode::Invalid);
}

Pin::~Pin()
{
    Pin::_Release(this);
}

