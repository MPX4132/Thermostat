//
//  Accessible.cpp
//  Thermostat
//
//  Created by Matias Barcenas on 11/6/18.
//  Copyright © 2018 Matias Barcenas. All rights reserved.
//

#include "Accessible.hpp"

std::shared_ptr<Accessible> const &Accessible::self() const
{
    return _self;
}

Accessible::Accessible():
_self(this, [](Accessible * const accessible){(void) accessible;})
{}

Accessible::~Accessible()
{}
