//
//  ShenOrderImbalance.hpp
//  Backtester
//
//  Created by Peter Tolsma on 7/29/18.
//  Copyright © 2018 Peter Tolsma. All rights reserved.
//

#ifndef ShenOrderImbalance_hpp
#define ShenOrderImbalance_hpp

#include <armadillo>
#include <memory>
#include <stdio.h>
#include <vector>
#include <deque>

#include "Engine.hpp"
#include "CSV_File.hpp"
#include "Level2OrderBook.hpp"

namespace Backtester {
    
    class ShenOrderImbalance : public Engine
    {
    public:
        /*  dataDir should be the path to a directory containing only CBOE Gemini orderbook data csv's, will fill dataFiles with an std::string corresponding to each file in dataDir
         */
        ShenOrderImbalance(std::string dataDir);
        ShenOrderImbalance(unsigned _LATENCY, unsigned _lockoutLength, decimal _takerFee, std::string dataDir);
        // here the ctor is passed the actual list of (sorted chronologically) files to use
        ShenOrderImbalance(std::vector<std::string> _dataFiles);
        ShenOrderImbalance(unsigned _LATENCY, unsigned _lockoutLength, decimal _takerFee, std::vector<std::string> _dataFiles);
    protected:
        
        /* what to do at the next "step", immediately after a new timestamp is entered, this includes setting currentTime, also anything else specific to the BACKTESTER such as latency handling andkeeping track of PnL in PNL_curve.
         */
        void onStep() override;
        
        // Algorithm logic goes in algoLogic
        // user implemented method that requires data handling logic to be implemented
        void algoLogic() override;
        
        // user implemented method that determines what happens an order reaches the "exchange" (after some latency)
        void onOrderArrival(Order& order) override;
    private:
        
        // the csv object the L2OrderBook object will reference
        std::shared_ptr<Gem_CSV_File> csv;
        
        // the orderbook corresponding to the current UTC date, this will need to be reinitialized for every passing day
        Level2OrderBook book;
        
        // stores a list of std:string's corresponding to gemini csv files the strategy will be run against
        std::vector<std::string> dataFiles;
        
        // represents how much inventory we have, >0 => long position, <0 => short position, 0 => no position
        decimal inventory;
        
        // how much fiat we have on hand
        decimal cash;
        
        // fee (in absolute rate, eg 3% = 0.03) that the exchange enforces for taking liquidity from the book
        decimal takerFee;
        
        // the minimum forecasted change needed to justify a change in position (denominated in USD)
        const decimal minForecastDelta;
        
        bool firstStep;
        bool secondStep;
        bool warmedUp;
        std::pair<decimal, decimal> pastBestBid;
        std::pair<decimal, decimal> pastBestAsk;
        decimal pastMidPrice;
        decimal pastAverageTradePrice;
        decimal pastTradeVolumeCurrency;
        decimal pastTransactionVolume;
        
        decimal currentVOI;
        decimal currentOIR;
        decimal currentMDP;
        decimal currentBidAskSpread;
        
        const unsigned numLaggedTicks;
        
        std::deque<decimal> VOI;
        std::deque<decimal> OIR;
        std::deque<decimal> MPB;
        
        // METHODS //
        
        // computes the total cost of a market buy from walking the book (doesn't actually remove liquidity)
        decimal executeMarketBuy(Order& order);
        
        // computes the total received fiat from a market sell walking the book (doesn't actually remove liquidity)
        decimal executeMarketSell(Order& order);
        
        decimal calculateVOI(bool firstStep);
        
        decimal calculateOIR() const;
        
        decimal calculateMPB(bool firstStep);
        
        decimal calculateBidAskSpread() const;
        
        // calculates new factors (VOI, OIR, etc. and pushes back to the respective deque's)
        void pushBackFactors();
        // pops the front of the factor deque's
        void popFrontFactors();
        
        // Given a matrix equation y=X*B...
        // observations are the "y" vector, each colvec in features is a column vector of the coefficient matrix B
        // NOTE: the dimensions of each colvec MUST be the same
        arma::mat getLinRegCoefficients(arma::mat observations, std::vector<arma::colvec> features);
        // observations are the "y" vector, features is the coefficient matrix B
        arma::mat getLinRegCoefficients(arma::mat observations, arma::mat features);
        
    };
    
}
#endif /* ShenOrderImbalance_hpp */
