//
//  Pin.hpp
//  Thermostat
//
//  Created by Matias Barcenas on 11/12/16.
//  Copyright © 2016 Matias Barcenas. All rights reserved.
//

#ifndef Pin_hpp
#define Pin_hpp

#include <map>
#include <Arduino.h>

// =============================================================================
// Pin : This class abstracts the I/O pins found on the development board. The
// class keeps track of all pins available.
// =============================================================================
class Pin
{
public:
    typedef unsigned int Identifier;
    typedef int Value;
    
    enum Mode
    {
        Invalid,
        Auto,
        Output,
        Input
    };
    
    struct Configuration {
        Mode mode;
        Value value;
    };
    
    Identifier identity() const;
    
    bool ready() const;
    
    Mode mode() const;
    bool setMode(Mode const mode);
    
    
    Value state() const;
    bool setState(Value const state);
    
    Configuration configuration() const;
    void setConfiguration(Configuration const &configuration);
    
    Pin(Identifier const identifier);
    ~Pin();
    
protected:
    Identifier const _identity;
    Value _value;
    Mode _mode;
    
    static std::map<Identifier, Pin const *> _Reserved;
    
    static bool _Reserve(Pin const * const pin);
    static bool _Release(Pin const * const pin);
};

#endif /* Pin_hpp */

