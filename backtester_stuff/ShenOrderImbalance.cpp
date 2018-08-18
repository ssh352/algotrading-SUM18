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

// TODO remove magic numbers

namespace fs = boost::filesystem;
namespace Backtester {
    
    ////////////
    // PUBLIC //
    ////////////
    
    // dataDir should be the path to a directory containing only CBOE Gemini orderbook data csv's, will fill
    // dataFiles with an std::string corresponding to each file in dataDir
    ShenOrderImbalance::ShenOrderImbalance(std::string dataDir)
    : ShenOrderImbalance(200, 60, decimal("0.003"), dataDir) // initialize with 200ms latency and 60s lockout by default
    {        
 
    }
    
    ShenOrderImbalance::ShenOrderImbalance(unsigned _LATENCY, unsigned _lockoutLength, decimal _takerFee,
                                           std::string dataDir)
    : Engine(_LATENCY, _lockoutLength), takerFee(_takerFee), firstStep(true), warmedUp(false), minForecastDelta("0.01"), csvIndex(0),
      numLaggedTicks(50), forecastWindow(100), currentAverageTradePrice(decimal("0.0"))
    {
        for (fs::directory_entry& d : fs::directory_iterator(dataDir))
        {
            dataFiles.push_back(d.path().native());
        }
        
        std::sort(dataFiles.begin(), dataFiles.end());
        std::ifstream in_f(dataFiles.front());
        csv = std::make_shared<Gem_CSV_File>(in_f);
        book = Level2OrderBook(csv);
    }
    
    // here the ctor is passed the actual list of (sorted chronologically) files to use
    ShenOrderImbalance::ShenOrderImbalance(std::vector<std::string> _dataFiles)
    : ShenOrderImbalance(200, 60, decimal("0.003"), _dataFiles)
    {
        // process csv files
    }
    
    ShenOrderImbalance::ShenOrderImbalance(unsigned _LATENCY, unsigned _lockoutLength, decimal _takerFee,
                                           std::vector<std::string> _dataFiles)
    : Engine (_LATENCY, _lockoutLength), dataFiles(_dataFiles), takerFee(_takerFee), firstStep(true), warmedUp(false), csvIndex(0),
      numLaggedTicks(50), forecastWindow(100), currentAverageTradePrice(decimal("0.0"))
    {
        // process csv files w latency
    }
    
    ///////////////
    // PROTECTED //
    ///////////////
    
    void ShenOrderImbalance::onInitialize()
    {
        // TODO calculate the first day's factors and then acquire the coefficients for the regression model
        std::vector<decimal> daysVOI, daysOIR, daysMPB, daysMidPrice;
        // this will be the first index where each of the factors has a valid value, mainly need to have MPB defined
        long long warmedUpIndex = -1;
        while (true)
        {
            daysMidPrice.push_back(book.getMidPrice());
            daysVOI.push_back(calculateVOI(firstStep));
            daysOIR.push_back(calculateOIR());
            daysMPB.push_back(calculateMPB(firstStep));
            // MPB is most likely going to be the last factor that becomes defined since OIR is instaneous and VOI
            // is defined on the second tick
            if (warmedUpIndex == -1 && daysMPB.back() != std::numeric_limits<decimal>::quiet_NaN())
                warmedUpIndex = daysMPB.size() - 1;
            
            try
            {
                book.processCSVLine(csv->peekNextLine());
                csv->removeNextLine();
            }
            catch (...) // probably poor style to catch every single exception but eh
            {
                break;
            }
        }
        firstStep = true;
        secondStep = false;

        // calculating change in mid-price between each event
        for (size_t i = 1; i < daysMidPrice.size(); ++i)
        {
            daysMidPrice[i] = daysMidPrice[i] - daysMidPrice[i-1];
        }
        daysMidPrice.front() = 0;
        
        // if MPB is defined on the first tick of the day, we need to advance the index so that VOI is defined
        if (!warmedUpIndex)
            ++warmedUpIndex;
        
        assert(!daysMidPrice.empty() && warmedUpIndex > 0 && warmedUpIndex < daysMidPrice.size());
        
        std::vector<decimal> avgMidChanges;
        
        // I'm worried about numerical precision here. We are using high precision decimal class but still.....
        for (size_t i = warmedUpIndex + numLaggedTicks - 1; i < daysMidPrice.size() - forecastWindow; ++i)
        {
            // see page 22 in Shen, the formula for the "k-step average mid-price change"
            avgMidChanges.push_back(std::accumulate(daysMidPrice.begin() + i + 1,
                                                    daysMidPrice.begin() + i + forecastWindow + 1,
                                                    decimal("0"), [i, &daysMidPrice](decimal a, decimal b){
                                                        return a + b - daysMidPrice[i];
                                                    }) / forecastWindow);
        }
        
    }
    
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
    
    // returns std::numeric_limits<decimal>::quiet_NaN() if not defined
    decimal ShenOrderImbalance::calculateVOI(bool firstStep)
    {
        if (firstStep)
        {
            pastBestAsk = book.getBestAsk();
            pastBestBid = book.getBestBid();
            return std::numeric_limits<decimal>::quiet_NaN();
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
    
    // returns std::numeric_limits<decimal>::quiet_NaN() if not defined
    decimal ShenOrderImbalance::calculateMPB(bool firstStep)
    {
        if (firstStep)
        {
            pastMidPrice = book.getMidPrice();
            // set pastTransactionVolume
            // set pastTradeVolumeCurrency
            firstStep = false;
            secondStep = true;
            return std::numeric_limits<decimal>::quiet_NaN();
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
            return std::numeric_limits<decimal>::quiet_NaN();
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
    
    void ShenOrderImbalance::processNextCSV()
    {
        ++csvIndex;
        if (csvIndex < dataFiles.size())
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
