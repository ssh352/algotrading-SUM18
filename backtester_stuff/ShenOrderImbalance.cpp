//
//  ShenOrderImbalance.cpp
//  Backtester
//
//  Created by Peter Tolsma on 7/29/18.
//  Copyright © 2018 Peter Tolsma. All rights reserved.
//

#include "ShenOrderImbalance.hpp"

#include <algorithm>
#include <fstream>
#include <boost/filesystem.hpp>
#include <vector>

namespace fs = boost::filesystem;
namespace Backtester {
    
    ////////////
    // PUBLIC //
    ////////////
    
    // dataDir should be the path to a directory containing only CBOE Gemini orderbook data csv's, will fill
    // dataFiles with an std::string corresponding to each file in dataDir
    ShenOrderImbalance::ShenOrderImbalance(std::string dataDir)
    : Engine()
    {
        std::vector<std::string> tmpVec;
        for (fs::directory_entry& d: fs::directory_iterator(dataDir))
            dataFiles.push_back(d.path().native());
        std::sort(dataFiles.begin(), dataFiles.end());
        std::ifstream in_f(dataFiles.front());
        csv = std::make_shared<Gem_CSV_File>(in_f);
        book = Level2OrderBook(csv);
    }
    
    ShenOrderImbalance::ShenOrderImbalance(unsigned _LATENCY, unsigned _lockoutLength, std::string dataDir)
    : Engine(_LATENCY, _lockoutLength)
    {
        std::string path = dataDir;
        for (auto& p : fs::directory_iterator(path))
            dataFiles.push_back(p.path().native());
        
        std::sort(dataFiles.begin(), dataFiles.end());

    }
    
    // here the ctor is passed the actual list of (sorted chronologically) files to use
    ShenOrderImbalance::ShenOrderImbalance(std::vector<std::string> _dataFiles)
    : Engine(), dataFiles(_dataFiles)
    {

    }
    
    ShenOrderImbalance::ShenOrderImbalance(unsigned _LATENCY, unsigned _lockoutLength, std::vector<std::string> _dataFiles)
    : Engine (_LATENCY, _lockoutLength), dataFiles(_dataFiles)
    {

    }
    
    ///////////////
    // PROTECTED //
    ///////////////
    
    // what to do at the next "step" after the current timestamp/event has been processed, this includes setting the
    // currentTime variable and anything else specific to the BACKTESTER such as latency handling. Algorithm logic
    // goes in algoLogic
    void ShenOrderImbalance::onStep()
    {
        auto nextRow = csv->peekNextLine();
        currentTime = boost::posix_time::time_from_string(nextRow["EventDate"] + ' ' + nextRow["EventTime"] + '.'
                                                          + nextRow["EventMillis"]);
    }
    // user implemented method that requires data handling logic to be implemented
    void ShenOrderImbalance::algoLogic()
    {
        
    }
    
    // user implemented method that determines what happens an order reaches the "exchange" (after some latency)
    void ShenOrderImbalance::onOrderArrival(Order& order)
    {
        
    }
    
    /////////////
    // PRIVATE //
    /////////////
    
}
