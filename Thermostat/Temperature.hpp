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
    typedef NumericType value_type;

    enum Scale
    {
        Kelvin = 'K',
        Celsius = 'C',
        Fahrenheit = 'F'
    };
    
    bool operator==(Temperature<NumericType> const &other) const
    {
        return this->value() == other.value();
    }
    
    bool operator<(Temperature<NumericType> const &other) const
    {
        return this->value() < other.value();
    }
    
    bool operator>(Temperature<NumericType> const &other) const
    {
        return this->value() > other.value();
    }
    
    bool operator<=(Temperature<NumericType> const &other) const
    {
        return this->value() <= other.value();
    }
    
    bool operator>=(Temperature<NumericType> const &other) const
    {
        return this->value() >= other.value();
    }
    
    Temperature& operator=(Temperature<NumericType> const &newValue)
    {
        this->setValue(newValue);
        return *this;
    }
    
    NumericType operator=(NumericType const newValue)
    {
        this->setValue(newValue);
        return this->value();
    }
    
    Temperature<NumericType>& operator*=(Temperature<NumericType> const &other)
    {
        this->setValue(this->value() * other.value());
        return *this;
    }
    
    NumericType operator*=(NumericType const value)
    {
        this->setValue(this->value() * value);
        return this->value();
    }
    
    Temperature<NumericType>& operator/=(Temperature<NumericType> const &other)
    {
        this->setValue(this->value() / other.value());
        return *this;
    }
    
    NumericType operator/=(NumericType const value)
    {
        this->setValue(this->value() / value);
        return this->value();
    }
    
    Temperature<NumericType>& operator+=(Temperature<NumericType> const &other)
    {
        this->setValue(this->value() + other.value());
        return *this;
    }
    
    NumericType operator+=(NumericType const value)
    {
        this->setValue(this->value() + value);
        return this->value();
    }
    
    Temperature<NumericType>& operator-=(Temperature<NumericType> const &other)
    {
        this->setValue(this->value() - other.value());
        return *this;
    }
    
    NumericType operator-=(NumericType const value)
    {
        this->setValue(this->value() - value);
        return this->value();
    }
    
    Temperature<NumericType> operator*(Temperature<NumericType> const &other) const {
        return Temperature<NumericType>(this->value() * other.value());
    }
    
    NumericType operator*(NumericType const other) const {
        return this->value() * other;
    }
    
    Temperature<NumericType> operator/(Temperature<NumericType> const &other) const {
        return Temperature<NumericType>(this->value() / other.value());
    }
    
    NumericType operator/(NumericType const other) const {
        return this->value() / other;
    }
    
    Temperature<NumericType> operator+(Temperature<NumericType> const &other) const {
        return Temperature<NumericType>(this->value() + other.value());
    }
    
    NumericType operator+(NumericType const other) const {
        return this->value() + other;
    }
    
    Temperature<NumericType> operator-(Temperature<NumericType> const &other) const {
        return Temperature<NumericType>(this->value() - other.value());
    }
    
    NumericType operator-(NumericType const other) const {
        return this->value() - other;
    }
    
    NumericType value(Scale const scale = Kelvin) const
    {
        return Convert(this->_value, Kelvin, scale);
    }
    
    void setValue(NumericType const value, Scale const scale = Kelvin)
    {
        _value = Convert(value, scale, Kelvin);
    }
    
    void setValue(Temperature<NumericType> const &temperature)
    {
        this->setValue(temperature.value());
    }
    
    static NumericType Convert(NumericType const value,
                               Scale const scaleInput,
                               Scale const scaleOutput)
    {
        // If we're not really converting, skip everything...
        if (scaleInput == scaleOutput) return value;
        
        // Fugly code below, brace yourself... Can't think of anything better...
        switch (scaleInput)
        {
            case Celsius:
            {
                switch (scaleOutput)
                {
                    case Fahrenheit: return (value + 40) * 1.80 - 40;
                    case Kelvin: return value + 273.15;
                    default: break;
                }
            }	break;
                
            case Fahrenheit:
            {
                switch (scaleOutput)
                {
                    case Celsius: return (value + 40) / 1.80 - 40;
                    case Kelvin: return Convert(value, Fahrenheit, Celsius) + 273.15;
                    default: break;
                }
            }	break;
            
            case Kelvin:
            {
                switch (scaleOutput) {
                    case Fahrenheit: return Convert(value - 273.15, Celsius, Fahrenheit);
                    case Celsius: return value - 273.15;
                    default: break;
                }
            }   break;
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
    _value(temperature.value())
    {
        
    }
    
    Temperature(NumericType const value = 295.37222222222, Scale const scale = Kelvin):
    _value(value) // 72F By default
    {
        if (scale != Kelvin) this->setValue(value, scale);
    }
    
protected:
    
    NumericType _value; // Internal value is always stored as Kelvin.
    
};

#endif /* Temperature_hpp */

