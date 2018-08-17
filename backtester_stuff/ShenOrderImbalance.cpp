//
//  ShenOrderImbalance.cpp
//  Backtester
//
//  Created by Peter Tolsma on 7/29/18.
//  Copyright © 2018 Peter Tolsma. All rights reserved.
//

#include "ShenOrderImbalance.hpp"

#include <algorithm>
#include <cassert>
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
    : Engine(), takerFee("0.003"), firstStep(true), warmedUp(false), minForecastDelta("0.01"), numLaggedTicks(50), csvIndex(0)
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
    ShenOrderImbalance::ShenOrderImbalance(unsigned _LATENCY, unsigned _lockoutLength, decimal _takerFee,
                                           std::string dataDir)
    : Engine(_LATENCY, _lockoutLength), takerFee(_takerFee), firstStep(true), warmedUp(false), minForecastDelta("0.01"), csvIndex(0),
      numLaggedTicks(50)
    {
        for (auto& p : fs::directory_iterator(dataDir))
        {
            dataFiles.push_back(p.path().native());
        }
        
        std::sort(dataFiles.begin(), dataFiles.end());
    }
    
    // here the ctor is passed the actual list of (sorted chronologically) files to use
    ShenOrderImbalance::ShenOrderImbalance(std::vector<std::string> _dataFiles)
    : Engine(), dataFiles(_dataFiles), takerFee("0.003"), firstStep(true), warmedUp(false), minForecastDelta("0.01"), csvIndex(0),
      numLaggedTicks(10)
    {
        // process csv files
    }
    
    ShenOrderImbalance::ShenOrderImbalance(unsigned _LATENCY, unsigned _lockoutLength, decimal _takerFee,
                                           std::vector<std::string> _dataFiles)
    : Engine (_LATENCY, _lockoutLength), dataFiles(_dataFiles), takerFee(_takerFee), firstStep(true), warmedUp(false), csvIndex(0),
      numLaggedTicks(10)
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
        
        pastBestAsk = book.getBestAsk();
        pastBestBid = book.getBestBid();
        
        Gem_CSV_Row currentLine = csv->removeNextLine();
        book.processCSVLine(currentLine);
        
        // calculate VOI
        currentVOI = calculateVOI(firstStep);
        
        // calculate OIR
        currentOIR = calculateOIR();
        
        // calculate MDP
        currentMPB = calculateMPB(firstStep);
        
        // calculate instantaneous bid-ask spread
        currentBidAskSpread = calculateBidAskSpread();

        
    }
    
    // user implemented method that requires data handling logic to be implemented
    void ShenOrderImbalance::algoLogic()
    {
        // if first step we just calculate the pastBestBid/Ask
        if (firstStep)
        {
            calculateVOI(firstStep);
            calculateMPB(firstStep);
            return;
        }
        // otherwise safe to calculate and push back
        else
        {
            currentVOI = calculateVOI(firstStep);
            currentOIR = calculateOIR();
            currentMPB = calculateMPB(firstStep);
        }
        
        pushBackFactors();
        
        if (!warmedUp)
        {
            assert(VOI.size() <= numLaggedTicks);
            if (VOI.size() == numLaggedTicks)
                warmedUp = true;
            else
                return;
        }
        
        if (VOI.size() > numLaggedTicks)
            popFrontFactors();
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
        return book.calculateTotalOrderCost(order.quantityOrdered, true);
    }
    
    // computes the total received fiat from a market sell walking the book (doesn't actually remove liquidity)
    decimal ShenOrderImbalance::executeMarketSell(Order& order)
    {
       return book.calculateTotalOrderCost(order.quantityOrdered, false);
    }
    
    decimal ShenOrderImbalance::calculateVOI(bool firstStep)
    {
        if (firstStep)
        {
            pastBestAsk = book.getBestAsk();
            pastBestBid = book.getBestBid();
            return -1;
        }
        else
        {
            decimal deltaBidVolume = 0;
            decimal deltaAskVolume = 0;
            std::pair<decimal,decimal> currentBestAsk = book.getBestAsk();
            std::pair<decimal,decimal> currentBestBid = book.getBestBid();
            
            // calculate bid delta
            if (currentBestBid.first < pastBestBid.first)
            {
                deltaBidVolume = 0;
            }
            else if (currentBestAsk.first > pastBestAsk.first)
            {
                deltaBidVolume = currentBestBid.second;
            }
            else
            {
                deltaBidVolume = currentBestBid.second - pastBestBid.second;
            }
            
            // calculate ask delta
            if (currentBestAsk.first > pastBestAsk.first)
            {
                deltaAskVolume = 0;
            }
            else if (currentBestAsk.first > pastBestAsk.first)
            {
                deltaAskVolume = currentBestAsk.second;
            }
            else
            {
                deltaAskVolume = currentBestAsk.second - pastBestAsk.second;
            }
            return deltaBidVolume - deltaAskVolume;
        }
    }
    
    decimal ShenOrderImbalance::calculateOIR() const
    {
        std::pair<decimal,decimal> currentBestAsk = book.getBestAsk();
        std::pair<decimal,decimal> currentBestBid = book.getBestBid();
        
        return (currentBestBid.second - currentBestAsk.second)/(currentBestBid.second + currentBestAsk.second);
    }
    
    decimal ShenOrderImbalance::calculateMPB(bool firstStep)
    {
        if (firstStep)
        {
            pastMidPrice = book.getMidPrice();
            // set pastTransactionVolume
            // set pastTradeVolumeCurrency
            firstStep = false;
            secondStep = true;
            return -1;
        }
        else if (secondStep)
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
    
    decimal ShenOrderImbalance::calculateBidAskSpread() const
    {
        return book.getBestAsk().first - book.getBestBid().first;
    }
    
    void ShenOrderImbalance::pushBackFactors()
    {
        VOI.push_back(currentVOI);
        OIR.push_back(currentOIR);
        MPB.push_back(currentMPB);
    }
    
    void ShenOrderImbalance::popFrontFactors()
    {
        VOI.pop_front();
        OIR.pop_front();
        MPB.pop_front();
    }
    
    void ShenOrderImbalance::ProcessNextCSV()
    {
        ++csvIndex;
        if(csvIndex < dataFiles.size())
        {
            std::ifstream in_f(dataFiles[csvIndex]);
            csv = std::make_shared<Gem_CSV_File>(in_f);
            book = Level2OrderBook(csv);
        }
    }
    
    // Given a matrix equation y=B*X...
    // observations are the "y" vector, each colvec in features is a column vector of the coefficient matrix B
    // NOTE: the dimensions of each colvec MUST be the same
    arma::mat ShenOrderImbalance::getLinRegCoefficients(arma::mat observations, std::vector<arma::colvec> features)
    {
        arma::mat X(0, features.size());
        for (arma::colvec colvec : features)
        {
            X.insert_cols(0, colvec);
        }
        return arma::inv(X.t() * X) * X * observations;
    }
    
    // observations are the "y" vector, features is the coefficient matrix B with each column
    arma::mat ShenOrderImbalance::getLinRegCoefficients(arma::mat observations, arma::mat features)
    {
        return arma::inv(features.t() * features) * features * observations;
    }
}
