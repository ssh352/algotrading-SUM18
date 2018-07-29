//
//  OrderBook.hpp
//  Backtester
//
//  Created by Peter Tolsma on 7/24/18.
//  Copyright © 2018 Peter Tolsma. All rights reserved.
//

#ifndef OrderBook_hpp
#define OrderBook_hpp

#include <stdio.h>
#include <vector>
#include <queue>

#include "Order.hpp"
#include "CSV_File.hpp"

namespace Backtester
{
    class OrderBook
    {
    public:
        OrderBook() = default;
        OrderBook(const Gem_CSV_File &csvFile);
    private:
        //std::vector<std::queue<Order>> asks;
       // std::vector<std::queue<Order>> bids;
    public:
    };
}

#endif /* OrderBook_hpp */
