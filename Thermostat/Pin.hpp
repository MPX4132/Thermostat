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
#include "Accessible.hpp"

#if defined(MJB_ARDUINO_LIB_API)
#include <Arduino.h>
#endif

// =============================================================================
// Pin : This class abstracts the I/O pins found on the development board. The
// class keeps track of all pins available.
// =============================================================================
class Pin : public Accessible
{
  public:
    
    typedef unsigned int Identifier;
    typedef short Value;

    typedef std::map<Identifier, std::shared_ptr<Pin>> Set;
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

    static Set MakeSet(Arrangement const &pins);

    Pin(Identifier const identifier);
    Pin();
    ~Pin();

  protected:
    
    Identifier const _identity;
    Value _value;
    Mode _mode;

    inline static Set &_Reserved();

    static bool _Reserve(std::shared_ptr<Pin> const &pin);
    static bool _Release(std::shared_ptr<Pin> const &pin);
};

#endif /* Pin_hpp */

