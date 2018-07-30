//
//  ShenOrderImbalance.hpp
//  Backtester
//
//  Created by Peter Tolsma on 7/29/18.
//  Copyright © 2018 Peter Tolsma. All rights reserved.
//

#ifndef ShenOrderImbalance_hpp
#define ShenOrderImbalance_hpp

#include <stdio.h>
#include <vector>

#include "Engine.hpp"
#include "CSV_File.hpp"
#include "Level2OrderBook.hpp"

namespace Backtester {
    
    class ShenOrderImbalance : public Engine
    {
    public:
        // dataDir should be the path to a directory containing only CBOE Gemini orderbook data csv's, will fill
        // dataFiles with an std::string corresponding to each file in dataDir
        ShenOrderImbalance(std::string dataDir);
        ShenOrderImbalance(unsigned _LATENCY, unsigned _lockoutLength, std::string dataDir);
        // here the ctor is passed the actual list of (sorted chronologically) files to use
        ShenOrderImbalance(std::vector<std::string> _dataFiles);
        ShenOrderImbalance(unsigned _LATENCY, unsigned _lockoutLength, std::vector<std::string> _dataFiles);
    protected:
        // what to do at the next "step" after the current timestamp/event has been processed, this includes setting the
        // currentTime variable and anything else specific to the BACKTESTER such as latency handling. Algorithm logic
        // goes in algoLogic
        void onStep() override;
        
        // user implemented method that requires data handling logic to be implemented
        void algoLogic() override;
        
        // user implemented method that determines what happens an order reaches the "exchange" (after some latency)
        void onOrderArrival(Order& order) override;
    private:
        
        // the csv object the L2OrderBook object will reference
        Gem_CSV_File csv;
        
        // the orderbook corresponding to the current UTC date, this will need to be reinitialized for every passing day
        Level2OrderBook book;
        
        // stores a list of std:string's corresponding to gemini csv files the strategy will be run against
        std::vector<std::string> dataFiles;
        
    };
    
}
#endif /* ShenOrderImbalance_hpp */
