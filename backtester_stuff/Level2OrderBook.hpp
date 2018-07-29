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
        
        // Default constructor
        Level2OrderBook() = default;
        
        // initializes book from a CBOE Gemini data file
        Level2OrderBook(const Gem_CSV_File& csv);
        
         // initializes book by creating csv object and delegating to ctor above ^
        Level2OrderBook(std::istream& in_f);
        
        // adds quantity to price level @ price
        void addToPriceLevel(decimal price, decimal quantity);
        
        // removes quantity from price level @ price
        void removeFromPriceLevel(decimal price, decimal quantity);
        
        // fills an order by removing quantity from bit bids and asks @ price
        void fillOrder(decimal price, decimal quantity, bool buy);
        
        // processes next line in gemini csv file, whatever that may mean. This may include cancelling a previously made
        // order, placing a new one, or filling a market order
        void processCSVLine(const Gem_CSV_Row& line);
        
        // returns the n closest asks/bids from the midprice
        std::pair<std::vector<std::pair<decimal, decimal>>,
                  std::vector<std::pair<decimal, decimal>>> nClosestLevels(size_t n);
        
        // returns n closest asks from the midprice
        std::vector<std::pair<decimal, decimal>> nClosestAsks(size_t n);
        
        // returns n closest bids from the midprice
        std::vector<std::pair<decimal, decimal>> nClosestBids(size_t n);
        decimal midPrice;
    
    private:
        
        void updateMid();
        
        // first will be price level, second will be quantity at level
        std::vector<std::pair<decimal, decimal>> asks;
        std::vector<std::pair<decimal, decimal>> bids;
    };
    
}
#endif /* Level2OrderBook_hpp */
