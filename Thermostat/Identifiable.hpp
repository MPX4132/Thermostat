//
//  Identifiable.hpp
//  Thermostat
//
//  Created by Matias Barcenas on 1/13/17.
//  Copyright © 2017 Matias Barcenas. All rights reserved.
//

#ifndef Identifiable_hpp
#define Identifiable_hpp

#include <set>
#include "Development.h"

#ifdef HARDWARE_INDEPENDENT
#include <iostream>
#else
#include <Arduino.h>
#endif

template <typename T>
class Identifiable
{
public:
    static void const * ID()
    {
        return static_cast<void const *>(&Identifiable<T>::_Address);
    }
    
    static bool Instanced(void const * instance)
    {
        return Identifiable<T>::_Instances.count(instance);
    }
    
    static void Register(void const * instance)
    {
#if defined DEBUG && defined RTTI_LOGS
#ifdef HARDWARE_INDEPENDENT
        std::cout << "[Class <" << Identifiable<T>::ID() << ">] Registering Instance <" << std::hex << instance << ">, " << Identifiable<T>::_Instances.size() << " currently registered." << std::endl;
#else
        Serial.print("[Class <");
        Serial.print((unsigned long) Identifiable<T>::ID());
        Serial.print(">] Registering Instance <");
        Serial.print((unsigned long) instance);
        Serial.print(">, ");
        Serial.print(Identifiable<T>::_Instances.size());
        Serial.println(" currently registered.");
#endif
#endif
        Identifiable<T>::_Instances.insert(instance);
#if defined DEBUG && defined RTTI_LOGS
#ifdef HARDWARE_INDEPENDENT
        std::cout << "[Class <" << Identifiable<T>::ID() << ">] Registered Instance <" << std::hex << instance << ">, " << Identifiable<T>::_Instances.size() << " now registered." << std::endl;
#else
        Serial.print("[Class <");
        Serial.print((unsigned long) Identifiable<T>::ID());
        Serial.print(">] Registered Instance <");
        Serial.print((unsigned long) instance);
        Serial.print(">, ");
        Serial.print(Identifiable<T>::_Instances.size());
        Serial.println(" now registered.");
#endif
#endif
    }
    
    static void Unregister(void const * instance)
    {
#if defined DEBUG && defined RTTI_LOGS
#ifdef HARDWARE_INDEPENDENT
        std::cout << "[Class <" << Identifiable<T>::ID() << ">] Unregistering Instance <" << std::hex << instance << ">, " << Identifiable<T>::_Instances.size() << " currently registered." << std::endl;
#else
        Serial.print("[Class <");
        Serial.print((unsigned long) Identifiable<T>::ID());
        Serial.print(">] Unregistering Instance <");
        Serial.print((unsigned long) instance);
        Serial.print(">, ");
        Serial.print(Identifiable<T>::_Instances.size());
        Serial.println(" currently registered.");
#endif
#endif
        Identifiable<T>::_Instances.erase(instance);
#if defined DEBUG && defined RTTI_LOGS
#ifdef HARDWARE_INDEPENDENT
        std::cout << "[Class <" << Identifiable<T>::ID() << ">] Instance <" << std::hex << instance << "> unregistered, " << Identifiable<T>::_Instances.size() << " now registered." << std::endl;
#else
        Serial.print("[Class <");
        Serial.print((unsigned long) Identifiable<T>::ID());
        Serial.print(">] Instance <");
        Serial.print((unsigned long) instance);
        Serial.print("> unregistered, ");
        Serial.print(Identifiable<T>::_Instances.size());
        Serial.println(" now registered.");
#endif
#endif
    }
    
private:
    static bool const _Address;
    static std::set<void const *> _Instances;
    
    Identifiable() {}
};

template<typename T>
bool const Identifiable<T>::_Address = false;

template<typename T>
std::set<void const *> Identifiable<T>::_Instances;


#endif /* Identifiable_hpp */
