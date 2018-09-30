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
#include <vector>
#include "Development.hpp"

#ifdef HARDWARE_INDEPENDENT
#include <iostream>
#else
#include <Arduino.h>
#endif

// =============================================================================
// Pin : This class abstracts the I/O pins found on the development board. The
// class keeps track of all pins available.
// =============================================================================
class Pin
{
  public:
    
    typedef unsigned int Identifier;
    typedef short Value;
    
    //typedef std::map<Identifier, Pin> Set;
    typedef std::map<Identifier, Pin *> Set;
    typedef std::vector<Identifier> Arrangement;
    
    enum Mode
    {
      Invalid,
      Auto,
      Output,
      Input
    };

    struct Configuration
    {
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
    
    static Set AllocateSet(Arrangement const &pins);
    static void DeallocateSet(Set const &pins);
    
    Pin(Identifier const identifier);
    //Pin(Pin const &pin);
    Pin();
    ~Pin();

  protected:
    
    //typedef std::map<Identifier, Pin *> Association;
    
    Identifier const _identity;
    Value _value;
    Mode _mode;
    
    //static Association _Reserved;
    static Set _Reserved;

    static bool _Reserve(Pin * const pin);
    static bool _Release(Pin const * const pin);
};

#endif /* Pin_hpp */

