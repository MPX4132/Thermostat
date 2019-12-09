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
#include "Development.hpp"

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
        return Identifiable<T>::_Instances().count(instance);
    }
    
    static void Register(void const * instance)
    {
#if defined(MJB_DEBUG_LOGGING_IDENTIFIABLE)
        MJB_DEBUG_LOG("[Class <");
        MJB_DEBUG_LOG((unsigned long) Identifiable<T>::ID());
        MJB_DEBUG_LOG(">] Registering Instance <");
        MJB_DEBUG_LOG((unsigned long) instance);
        MJB_DEBUG_LOG(">, ");
        MJB_DEBUG_LOG(Identifiable<T>::_Instances().size());
        MJB_DEBUG_LOG_LINE(" currently registered.");
#endif

        Identifiable<T>::_Instances().insert(instance);

#if defined(MJB_DEBUG_LOGGING_IDENTIFIABLE)
        MJB_DEBUG_LOG("[Class <");
        MJB_DEBUG_LOG((unsigned long) Identifiable<T>::ID());
        MJB_DEBUG_LOG(">] Registered Instance <");
        MJB_DEBUG_LOG((unsigned long) instance);
        MJB_DEBUG_LOG(">, ");
        MJB_DEBUG_LOG(Identifiable<T>::_Instances().size());
        MJB_DEBUG_LOG_LINE(" now registered.");
#endif
    }
    
    static void Unregister(void const * instance)
    {
#if defined(MJB_DEBUG_LOGGING_IDENTIFIABLE)
        MJB_DEBUG_LOG("[Class <");
        MJB_DEBUG_LOG((unsigned long) Identifiable<T>::ID());
        MJB_DEBUG_LOG(">] Unregistering Instance <");
        MJB_DEBUG_LOG((unsigned long) instance);
        MJB_DEBUG_LOG(">, ");
        MJB_DEBUG_LOG(Identifiable<T>::_Instances().size());
        MJB_DEBUG_LOG_LINE(" currently registered.");
#endif

        Identifiable<T>::_Instances().erase(instance);

#if defined(MJB_DEBUG_LOGGING_IDENTIFIABLE)
        MJB_DEBUG_LOG("[Class <");
        MJB_DEBUG_LOG((unsigned long) Identifiable<T>::ID());
        MJB_DEBUG_LOG(">] Instance <");
        MJB_DEBUG_LOG((unsigned long) instance);
        MJB_DEBUG_LOG("> unregistered, ");
        MJB_DEBUG_LOG(Identifiable<T>::_Instances().size());
        MJB_DEBUG_LOG_LINE(" now registered.");
#endif
    }
    
private:
    static bool const _Address;
    inline static std::set<void const *> &_Instances()
    {
        static std::set<void const *> _instances;
        return _instances;
    }
    
    Identifiable() {}
};

template<typename T>
bool const Identifiable<T>::_Address = false;


#endif /* Identifiable_hpp */
