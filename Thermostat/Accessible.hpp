//
//  Accessible.hpp
//  Thermostat
//
//  Created by Matias Barcenas on 11/6/18.
//  Copyright © 2018 Matias Barcenas. All rights reserved.
//

#ifndef Accessible_hpp
#define Accessible_hpp

#include <memory>

class Accessible
{
protected:
    std::shared_ptr<Accessible> const &self() const;

public:
    Accessible();
    virtual ~Accessible();

private:
    std::shared_ptr<Accessible> const _self;
};

#endif /* Accessible_hpp */
