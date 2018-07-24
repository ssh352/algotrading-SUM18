//
//  Order.cpp
//  HFT-Backtester
//
//  Created by Alec Goldberg on 7/23/18.
//  Copyright © 2018 Alec Goldberg. All rights reserved.
//

#include "Order.hpp"

using namespace backtester;
    
Order::Order()
:orderType(0)
    {
    }

Order::Order(int _orderType)
:orderType(_orderType)
{
    
}

Order::Order(const Order &otherOrder)
:orderType(otherOrder.orderType)
{
        
}

