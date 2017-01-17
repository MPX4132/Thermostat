//
//  Temperature.hpp
//  Thermostat
//
//  Created by Matias Barcenas on 11/12/16.
//  Copyright © 2016 Matias Barcenas. All rights reserved.
//

#ifndef Temperature_hpp
#define Temperature_hpp

#include "Development.hpp"

// Need to change this template to store internal values as kelvin...
// That should simplify the logic quite a bit and also spare the microprocessor
// from having to keep converting shit when dealing with this class.

// =============================================================================
// Temperature : This class abstracts different types of temperatures and sets
// the international standard as the base temperature.
// =============================================================================
template<typename NumericType>
class Temperature
{
public:
    enum Scale
    {
        Internal,
        Celsius,
        Fahrenheit,
        Kelvin
    };
    
    bool operator==(Temperature<NumericType> const &other) const
    {
        return this->value() == other.value(this->scale());
    }
    
    bool operator<(Temperature<NumericType> const &other) const
    {
        return this->value() < other.value(this->scale());
    }
    
    bool operator>(Temperature<NumericType> const &other) const
    {
        return this->value() > other.value(this->scale());
    }
    
    bool operator<=(Temperature<NumericType> const &other) const
    {
        return this->value() <= other.value(this->scale());
    }
    
    bool operator>=(Temperature<NumericType> const &other) const
    {
        return this->value() >= other.value(this->scale());
    }
    
    Temperature& operator=(Temperature<NumericType> const &newValue)
    {
        this->setValue(newValue);
        return *this;
    }
    
    Temperature& operator=(NumericType const newValue)
    {
        this->setValue(newValue);
        return *this;
    }
    
    NumericType operator+=(Temperature<NumericType> const &other)
    {
        this->setValue(this->value() + other.value(this->scale()));
        return this->value();
    }
    
    NumericType operator-=(Temperature<NumericType> const &other)
    {
        this->setValue(this->value() - other.value(this->scale()));
        return this->value();
    }
    
    NumericType operator/=(NumericType const value)
    {
        this->setValue(this->value() / value);
        return this->value();
    }
    
    NumericType operator*=(NumericType const value)
    {
        this->setValue(this->value() * value);
        return this->value();
    }
    
    NumericType operator+(Temperature<NumericType> const &other) {
        return this->value() + other.value(this->scale());
    }
    
    NumericType operator-(Temperature<NumericType> const &other) {
        return this->value() - other.value(this->scale());
    }
    
    NumericType operator+(NumericType const &other) {
        return this->value() + other;
    }
    
    NumericType operator-(NumericType const &other) {
        return this->value() - other;
    }
    
    Scale scale() const
    {
        return this->_scale;
    }
    
    void setScale(Scale const newScale, bool updateValue = true)
    {
        // If it's the "Internal" scale, fuck off...
        if (newScale == Internal) return;
        
        if (updateValue)
            this->setValue(Convert(this->value(), this->scale(), newScale));
        
        _scale = newScale;
    }
    
    NumericType value(Scale const scale = Internal) const
    {
        return Convert(this->_value, this->scale(), (scale == Internal)? this->scale() : scale);
    }
    
    void setValue(NumericType const value)
    {
        _value = value;
    }
    
    void setValue(Temperature<NumericType> const &temperature)
    {
        this->setScale(temperature.scale(), false);
        this->setValue(temperature.value());
    }
    
    static NumericType Convert(Temperature<NumericType> const &temperature,
                               Scale const outputScale)
    {
        return Convert(temperature.value(), temperature.scale(), outputScale);
    }
    
    static NumericType Convert(NumericType const value,
                               Scale const inputScale,
                               Scale const outputScale)
    {
        // If we're not really converting, skip everything...
        if (inputScale == outputScale) return value;
        if (inputScale == Internal || outputScale == Internal) return value;
        
        // Fugly code below, brace yourself... Can't think of anything better...
        switch (inputScale)
        {
            case Celsius:
            {
                switch (outputScale)
                {
                    case Fahrenheit:
                        return (value + 40) * 1.80 - 40;
                        
                    default: break;
                }
            }	break;
                
            case Fahrenheit:
            {
                switch (outputScale)
                {
                    case Celsius:
                        return (value + 40) / 1.80 - 40;
                        
                    default: break;
                }
            }	break;
                
            default: break;
        } 
        
        // If no match was found, return the value that was given.
        return value;
    }
    
    static Temperature<NumericType> Make(NumericType const value, char const scale)
    {
        switch (scale) {
            case 'f': return Temperature<NumericType>(value, Fahrenheit);
                break;
            
            case 'c': return Temperature<NumericType>(value, Celsius);
                break;
                
            case 'k': return Temperature<NumericType>(value, Kelvin);
                break;
                
            default:
                break;
        }
        
        return Temperature<NumericType>();
    }
    
    Temperature(Temperature<NumericType> const &temperature):
    _value(temperature.value()),
    _scale(temperature.scale())
    {
        
    }
    
    Temperature(NumericType const value = 72, Scale const scale = Fahrenheit):
    _value(value),
    _scale(scale)
    {
        if (this->scale() == Internal) 
            this->setScale(Fahrenheit, false);
    }
    
protected:
    NumericType _value;
    Scale _scale;
};

#endif /* Temperature_hpp */

