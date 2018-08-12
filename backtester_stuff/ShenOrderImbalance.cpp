//
//  ShenOrderImbalance.cpp
//  Backtester
//
//  Created by Peter Tolsma on 7/29/18.
//  Copyright © 2018 Peter Tolsma. All rights reserved.
//

#include "ShenOrderImbalance.hpp"

#include <algorithm>
#include <exception>
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
    : Engine(), takerFee("0.003")
    {
        std::vector<std::string> tmpVec;
        for (fs::directory_entry& d: fs::directory_iterator(dataDir))
            dataFiles.push_back(d.path().native());
        std::sort(dataFiles.begin(), dataFiles.end());
        std::ifstream in_f(dataFiles.front());
        csv = std::make_shared<Gem_CSV_File>(in_f);
        book = Level2OrderBook(csv);
    }
    
    ShenOrderImbalance::ShenOrderImbalance(unsigned _LATENCY, unsigned _lockoutLength, decimal _takerFee,
                                           std::string dataDir)
    : Engine(_LATENCY, _lockoutLength), takerFee(_takerFee)
    {
        for (auto& p : fs::directory_iterator(dataDir))
            dataFiles.push_back(p.path().native());
        
        std::sort(dataFiles.begin(), dataFiles.end());

    }
    
    // here the ctor is passed the actual list of (sorted chronologically) files to use
    ShenOrderImbalance::ShenOrderImbalance(std::vector<std::string> _dataFiles)
    : Engine(), dataFiles(_dataFiles), takerFee("0.003")
    {

    }
    
    ShenOrderImbalance::ShenOrderImbalance(unsigned _LATENCY, unsigned _lockoutLength, decimal _takerFee,
                                           std::vector<std::string> _dataFiles)
    : Engine (_LATENCY, _lockoutLength), dataFiles(_dataFiles), takerFee(_takerFee)
    {

    }
    
    ///////////////
    // PROTECTED //
    ///////////////
    
    // what to do at the next "step", immediately after a new timestamp is entered, this includes setting currentTime
    // Also anything else specific to the BACKTESTER such as latency handling and keeping track of PnL in PNL_curve.
    // Algorithm logic goes in algoLogic
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
        // Market order
        if (order.orderType == OrderTypes::Market)
        {
            // market buy
            if (order.quantityOrdered > 0)
            {
                decimal costOfBuy = executeMarketBuy(order) * (decimal(1) + takerFee);
                if (cash >= costOfBuy)
                    cash -= costOfBuy;
                else
                    std::cerr << "Attempted market purchase of " << order.quantityOrdered << " units but didn't have "
                              << "enough capital!\n";
            }
            // market sell
            else
            {
                cash += executeMarketSell(order) * (decimal(1) - takerFee);
            }
        }
        // Limit order
        else
        {
            throw std::runtime_error("This algorithm shouldn't be submitting limit orders!\n");
        }
    }
    
    /////////////
    // PRIVATE //
    /////////////
    
    // computes the total cost of a market buy from walking the book (doesn't actually remove liquidity)
    decimal ShenOrderImbalance::executeMarketBuy(Order& order)
    {
        decimal quantity = order.quantityOrdered;
        decimal totalCost = 0;
        auto best = book.bestAsk;
        while (quantity > 0)
        {
            // case where we can completely fill at one level
            if (quantity <= best->second)
            {
                totalCost += quantity * best->first;
                quantity -= best->second;
            }
            // case where we must walk the book to get completely filled
            else
            {
                quantity -= best->second;
                totalCost += best->second * best->first;
                ++best;
            }
        }
        
        return totalCost;
    }
    
    // computes the total received fiat from a market sell walking the book (doesn't actually remove liquidity)
    decimal ShenOrderImbalance::executeMarketSell(Order& order)
    {
        decimal quantity = order.quantityOrdered;
        decimal totalRecv = 0;
        auto best = book.bestBid;
        while (quantity < 0)
        {
            // case where we can completely fill at one level
            if (quantity <= best->second)
            {
                totalRecv += -1 * quantity * best->first;
                quantity += best->second;
            }
            // case where we must walk the book to get completely filled
            else
            {
                quantity += best->second;
                totalRecv += best->second * best->first;
                ++best;
            }
        }
        
        return totalRecv;
    }
}
