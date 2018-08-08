//
//  Order.cpp
//  HFT-Backtester
//
//  Created by Alec Goldberg on 7/23/18.
//  Copyright © 2018 Alec Goldberg. All rights reserved.
//

#include <limits>

#include "Order.hpp"

namespace Backtester {
    
    // assumed Market order
    Order::Order(decimal _quantityOrdered)
    : orderType(OrderTypes::Market), priceLevel(std::numeric_limits<decimal>::quiet_NaN()),
      quantityOrdered(_quantityOrdered)
    {
        
    }
    
    // assumed Limit order
    Order::Order(decimal _priceLevel, decimal _quantityOrdered)
    : orderType(OrderTypes::Limit), priceLevel(_priceLevel), quantityOrdered(_quantityOrdered)
    {
        
    }
    
    Order::Order(const Order &otherOrder)
    : orderType(otherOrder.orderType), priceLevel(otherOrder.priceLevel), quantityOrdered(otherOrder.quantityOrdered)
    {
        
    }

}

