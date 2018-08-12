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
    : Engine(), firstStep(true)
    {
        std::vector<std::string> tmpVec;
        
        for (fs::directory_entry& d: fs::directory_iterator(dataDir))
        {
            dataFiles.push_back(d.path().native());
        }
        
        std::sort(dataFiles.begin(), dataFiles.end());
        std::ifstream in_f(dataFiles.front());
        csv = std::make_shared<Gem_CSV_File>(in_f);
        book = Level2OrderBook(csv);
    }
    
    ShenOrderImbalance::ShenOrderImbalance(unsigned _LATENCY, unsigned _lockoutLength, std::string dataDir)
    : Engine(_LATENCY, _lockoutLength), firstStep(true)
    {
        for (auto& p : fs::directory_iterator(dataDir))
        {
            dataFiles.push_back(p.path().native());
        }
        
        std::sort(dataFiles.begin(), dataFiles.end());
    }
    
    // here the ctor is passed the actual list of (sorted chronologically) files to use
    ShenOrderImbalance::ShenOrderImbalance(std::vector<std::string> _dataFiles)
    : Engine(), dataFiles(_dataFiles), firstStep(true)
    {
        // process csv files
    }
    
    ShenOrderImbalance::ShenOrderImbalance(unsigned _LATENCY, unsigned _lockoutLength, std::vector<std::string> _dataFiles)
    : Engine (_LATENCY, _lockoutLength), dataFiles(_dataFiles), firstStep(true)
    {
        // process csv files w latency
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
        
        // calculate VOI
        currentVOI = calculateVOI(firstStep);
        
        // calculate OIR
        currentOIR = calculateOIR();
        
        // calculate MDP
        currentMDP = calculateMDP(firstStep);
        
        // calculate instantaneous bid-ask spread
        currentBidAskSpread = calculateBidAskSpread();
        
        pastBestAsk = book.getBestAsk();
        pastBestBid = book.getBestBid();
    }
    
    decimal ShenOrderImbalance::calculateVOI(bool firstStep)
    {
        if(firstStep)
        {
            pastBestAsk = book.getBestAsk();
            pastBestBid = book.getBestBid();
            return -1;
        }
        else
        {
            decimal lambdaBidVolume = 0;
            decimal lambdaAskVolume = 0;
            std::pair<decimal,decimal> currentBestAsk = book.getBestAsk();
            std::pair<decimal,decimal> currentBestBid = book.getBestBid();
            
            // calculate bid lambda
            if(currentBestBid.first < pastBestBid.first)
            {
                lambdaBidVolume = 0;
            }
            else if(currentBestAsk.first > pastBestAsk.first)
            {
                lambdaBidVolume = currentBestBid.second - pastBestBid.second;
            }
            else
            {
                lambdaBidVolume = currentBestBid.second;
            }
            
            // calculate ask lambda
            if(currentBestAsk.first < pastBestAsk.first)
            {
                lambdaAskVolume = 0;
            }
            else if(currentBestAsk.first > pastBestAsk.first)
            {
                lambdaAskVolume = currentBestAsk.second - pastBestAsk.second;
            }
            else
            {
                lambdaAskVolume = currentBestAsk.second;
            }
             return lambdaBidVolume - lambdaAskVolume;
        }
    }
    
    decimal ShenOrderImbalance::calculateOIR()
    {
        std::pair<decimal,decimal> currentBestAsk = book.getBestAsk();
        std::pair<decimal,decimal> currentBestBid = book.getBestBid();
        
        return (currentBestBid.second - currentBestAsk.second)/(currentBestBid.second + currentBestAsk.second);
    }
    
    decimal ShenOrderImbalance::calculateMDP(bool firstStep)
    {
        if(firstStep)
        {
            pastMidPrice = book.getMidPrice();
            // set pastTransactionVolume
            // set pastTradeVolumeCurrency
            firstStep = false;
            secondStep = true;
            return -1;
        }
        else if(secondStep)
        {
            decimal temp = pastMidPrice;
            pastMidPrice = book.getMidPrice();
            pastAverageTradePrice = temp;
            // set pastTransactionVolume
            // set pastTradeVolumeCurrency
            return book.getMidPrice() - (book.getMidPrice() + temp)/2;
        }
        else
        {
            // calculate MDP (average trade price at time t - Mid price average of t,t-1)
            return -1;
        }
    }
    
    decimal ShenOrderImbalance::calculateBidAskSpread()
    {
        return book.getBestAsk().first - book.getBestBid().first;
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
                decimal costOfBuy = executeMarketBuy(order);
                if (cash >= costOfBuy)
                    cash -= costOfBuy;
                else
                    std::cerr << "Attempted market purchase of " << order.quantityOrdered << " units but didn't have "
                              << "enough capital!\n";
            }
            // market sell
            else
            {
                cash += executeMarketSell(order);
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
        return book.calculateTotalOrderCost(order.quantityOrdered, true);
    }
    
    // computes the total received fiat from a market sell walking the book (doesn't actually remove liquidity)
    decimal ShenOrderImbalance::executeMarketSell(Order& order)
    {
       return book.calculateTotalOrderCost(order.quantityOrdered, false);
    }
}
