//
//  Delegable.hpp
//  Thermostat
//
//  Created by Matias Barcenas on 3/16/19.
//  Copyright © 2019 Matias Barcenas. All rights reserved.
//

#ifndef Delegable_hpp
#define Delegable_hpp

#include <memory>
#include <functional>
#include <unordered_set>
#include "Development.hpp"


template <typename DelegateType>
class Delegable {
private:
    class WeakDelegate
    {
    private:
        std::size_t _identifier;
        std::weak_ptr<DelegateType> _reference;

    public:
        struct Hasher
        {
            std::size_t operator()(WeakDelegate const &weakDelegate) const
            {
                return weakDelegate.identifier();
            }
        };

        std::weak_ptr<DelegateType> const & reference() const
        {
            return _reference;
        }

        void setReference(std::shared_ptr<DelegateType> const &reference)
        {
            _identifier = GenerateIdentifier(reference);
            _reference = std::weak_ptr<DelegateType>(reference);
        }

        std::size_t const & identifier() const
        {
            return _identifier;
        }

        bool operator==(WeakDelegate const &weakDelegate) const
        {
            return identifier() == weakDelegate.identifier();
        }

        static std::size_t GenerateIdentifier(std::shared_ptr<DelegateType> const &delegate)
        {
            // TODO: Ensure the reinterpret-cast below will not truncate values.
            return reinterpret_cast<std::size_t>(delegate.get()) / sizeof(WeakDelegate);
        }

        WeakDelegate(std::shared_ptr<DelegateType> const &delegate):
        _identifier(GenerateIdentifier(delegate)),
        _reference(delegate)
        {
        }
    };

    typedef typename std::unordered_set<WeakDelegate, typename WeakDelegate::Hasher> Container;

    // Notice: Utilizing unordered map for constant-time, O(1), lookup.
    // Note: The set isn't built of 'WeakDelegate const' because it appears
    // const references aren't allowed in the template. This appears not to
    // matter since the element is immutable while in the set container.
    Container _delegates;

    bool _hasWeakDelegate(WeakDelegate const &weakDelegate) const
    {
        // Notice: Due to "_delegates" being an unordered_map, relying on
        // hashing, the search done below is done in constant time; O(1).
        // Note: Constant-time realized by using one bucket per element.
        return _delegates.count(weakDelegate) > 0;
    }

protected:
    typedef typename Container::size_type size_type;

    size_type _delegate(std::function<bool (std::shared_ptr<DelegateType> const &)> callback) {
        typename Container::size_type delegated = 0;
        for (WeakDelegate const &weakDelegate : _delegates)
        {
            std::shared_ptr<DelegateType> const delegate(weakDelegate.reference().lock());
            if (delegate)
            {
                if (_willDelegateToDelegate(delegate))
                {
                    if (_didDelegateWithResult(delegate, callback(delegate))) delegated++;
                }
            }
            else
            {
                removeDelegate(delegate);
            }

        }
        return delegated;
    }

    // The following will/did/didNot methods are meant to be overwritten by deriving classes.
    virtual bool _didDelegateWithResult(std::shared_ptr<DelegateType> const &delegate,
                                        bool const delegationResult)
    {
        // The follow is used/are used to suppress unused variable warnings.
        (void) delegate;
        (void) delegationResult;
#if defined(MJB_DEBUG_LOGGING_DELEGABLE)
        MJB_DEBUG_LOG("<Delegable> did delegate to delegate: ");
        MJB_DEBUG_LOG_LINE(delegate);
#endif
        return true;
    }

    virtual bool _willDelegateToDelegate(std::shared_ptr<DelegateType> const &delegate)
    {
        // The follow is used/are used to suppress unused variable warnings.
        (void) delegate;
#if defined(MJB_DEBUG_LOGGING_DELEGABLE)
        MJB_DEBUG_LOG("<Delegable> will delegate to delegate: ");
        MJB_DEBUG_LOG_LINE(delegate);
#endif
        return true;
    }

    virtual bool _didAddDelegate(std::shared_ptr<DelegateType> const &delegate)
    {
        // The follow is used/are used to suppress unused variable warnings.
        (void) delegate;
#if defined(MJB_DEBUG_LOGGING_DELEGABLE)
        MJB_DEBUG_LOG("<Delegable> did add delegate: ");
        MJB_DEBUG_LOG_LINE(delegate);
#endif
        return true;
    }

    virtual bool _didNotAddDelegate(std::shared_ptr<DelegateType> const &delegate)
    {
        // The follow is used/are used to suppress unused variable warnings.
        (void) delegate;
#if defined(MJB_DEBUG_LOGGING_DELEGABLE)
        MJB_DEBUG_LOG("<Delegable> did not add delegate: ");
        MJB_DEBUG_LOG_LINE(delegate);
#endif
        return true;
    }

    virtual bool _didRemoveDelegate(std::shared_ptr<DelegateType> const &delegate)
    {
        // The follow is used/are used to suppress unused variable warnings.
        (void) delegate;
#if defined(MJB_DEBUG_LOGGING_DELEGABLE)
        MJB_DEBUG_LOG("<Delegable> did remove delegate: ");
        MJB_DEBUG_LOG_LINE(delegate);
#endif
        return true;
    }

    virtual bool _didNotRemoveDelegate(std::shared_ptr<DelegateType> const &delegate)
    {
        // The follow is used/are used to suppress unused variable warnings.
        (void) delegate;
#if defined(MJB_DEBUG_LOGGING_DELEGABLE)
        MJB_DEBUG_LOG("<Delegable> did not remove delegate: ");
        MJB_DEBUG_LOG_LINE(delegate);
#endif
        return true;
    }

public:
    bool addDelegate(std::shared_ptr<DelegateType> const &delegate)
    {
        WeakDelegate const weakDelegate(delegate);
        typename Container::iterator const weakDelegatei = _delegates.find(weakDelegate);
        if (weakDelegatei != _delegates.end())
        {
            std::shared_ptr<DelegateType> const existingDelegate(weakDelegatei->reference().lock());

            if (delegate != existingDelegate)
            {
                if (!removeDelegate(existingDelegate))
                {
                    return false;
                }
            }
        }
        return _delegates.insert(weakDelegate).second?
        _didAddDelegate(delegate) :
        _didNotAddDelegate(delegate);
    }

    bool hasDelegate(std::shared_ptr<DelegateType> const &delegate) const
    {
        WeakDelegate const weakDelegate(delegate);
        return _hasWeakDelegate(weakDelegate);
    }

    bool removeDelegate(std::shared_ptr<DelegateType> const &delegate)
    {
        WeakDelegate const weakDelegate(delegate);
        return (_delegates.erase(weakDelegate) > 0)?
        _didRemoveDelegate(delegate) :
        _didNotRemoveDelegate(delegate);
    }

    Delegable() {}
    virtual ~Delegable() {}
};

#endif /* Delegable_hpp */
