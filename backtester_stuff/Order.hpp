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

#endif /* Order_hpp */

namespace backtester
{

enum OrderTypes : int {
    Limit,
    Market
};
    
class Order
{
private:
    int orderType;
    
public:
    
    Order();
    
    Order(int _orderType);
    
    Order(const Order &otherOrder);
    
};
    
}
