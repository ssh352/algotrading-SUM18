//
//  Order.hpp
//  HFT-Backtester
//
//  Created by Alec Goldberg on 7/23/18.
//  Copyright © 2018 Alec Goldberg. All rights reserved.
//

#ifndef Order_hpp
#define Order_hpp

#include <stdio.h>
#include <boost/multiprecision/cpp_dec_float.hpp>

#endif /* Order_hpp */

namespace Backtester
{

    typedef boost::multiprecision::cpp_dec_float_50 decimal;
    enum OrderTypes : int {
        Limit,
        Market
    };
    
    class Order
    {
    private:
        
    public:
        const OrderTypes orderType;
        const decimal priceLevel; // only a number if orderType == Limit
        const decimal quantityOrdered; // necessary for both Limit & Market, >0 => long, <0 => short
        
        Order() = delete;
        
        Order(decimal _quantityOrdered); // assumed market order
        
        Order(decimal _priceLevel, decimal _quantityOrdered); // assumed limit order
        
        Order(const Order &otherOrder);
        
    };
    
}
