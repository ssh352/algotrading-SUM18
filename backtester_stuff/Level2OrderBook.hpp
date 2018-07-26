//
//  Level2OrderBook.hpp
//  Backtester
//
//  Created by Peter Tolsma on 7/26/18.
//  Copyright © 2018 Peter Tolsma. All rights reserved.
//

#ifndef Level2OrderBook_hpp
#define Level2OrderBook_hpp

#include <stdio.h>
#include <vector>
#include <iostream>
#include <utility>
#include <boost/multiprecision/cpp_dec_float.hpp>

#include "CSV_File.hpp"

namespace Backtester
{
    typedef boost::multiprecision::cpp_dec_float_50 decimal;
    class Level2OrderBook
    {
    public:
        
        Level2OrderBook() = default;
        Level2OrderBook(Gem_CSV_File csv);
        Level2OrderBook(std::istream& in_f);
        
        void addToPriceLevel(decimal price, decimal quantity);
        void removeFromPriceLevel(decimal price, decimal quantity);
        
        decimal midPrice;
    
    private:
        
        void updateMid();
        
        // first will be price level, second will be quantity at level
        std::vector<std::pair<decimal, decimal>> asks;
        std::vector<std::pair<decimal, decimal>> bids;
    };
    
}
#endif /* Level2OrderBook_hpp */
