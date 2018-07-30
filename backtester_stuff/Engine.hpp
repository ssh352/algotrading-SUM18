//
//  Backtester.hpp
//  Backtester
//
//  Created by Peter Tolsma on 6/13/18.
//  Copyright © 2018 Peter Tolsma. All rights reserved.
//

#ifndef Backtester_hpp
#define Backtester_hpp

#include "Order.hpp"
#include "Time_Event.hpp"

#include <stdio.h>
#include <vector>
#include <queue>
#include <utility>
#include <boost/date_time/posix_time/posix_time.hpp>

namespace Backtester
{
    
    class Engine
    {
    private:
        
        // Will store the PNL for each tick in time
        std::vector<decimal> PNL_curve;
        double maxDrawdown;
        
        // Online algorithm will update mean and var as per the following link:
        // https://en.wikipedia.org/wiki/Algorithms_for_calculating_variance#Online_algorithm
        decimal runningReturnMean;
        // see above
        decimal runningReturnVariance;
        // 3-Month T-Bill ~2% used as proxy for rf rate
        const decimal RISK_FREE_RATE = 2;
        
        // the (constant for now) latency in milliseconds we assume to be between sending and receiving an order
        const unsigned LATENCY;
        
        // this is just an idea.. But to prevent overestimation of profits, might be a good idea to not allow the algo
        // to do too many trades in a short time. This is because in reality we'd have at most a few trades before we
        // move the price too much, but in this backtester we can't really simulate this (as this would require some
        // crazy fucking math). This bool will tell the derived class that we implement whether we're allowed to trade.
        bool inLockoutPeriod;
        
        // how long we wish to be in trade lockout in seconds
        unsigned lockoutLength;
        
        // see wiki page above
        void updateRunningReturnMean();
        // see above
        void updateRunningReturnVar();
        
    public:

        Engine();
        Engine(unsigned _LATENCY, unsigned _lockoutLength);
        // (eventually) the big papi method, will run the whole shebang
        void runBacktest();

    protected:
        
        // if we are in lockout, when the lockout started
        boost::posix_time::ptime lockoutStartTime;
        
        // the datetime that the backtester is at. Crucial that this is set currently so that latency handling is done
        boost::posix_time::ptime currentTime;
        
        // will hold orders that have been sent but not yet received (due to latency) by the "exchange"
        std::queue<std::pair<Order, boost::posix_time::ptime>> sentOrders;
        
        // resolves an orders that have arrived after the next timestep has arrived and before any further algo logic
        void resolveOrders();
        
        // what to do at the next "step" after the current timestamp/event has been processed, this includes setting the
        // currentTime variable and anything else specific to the BACKTESTER such as latency handling. Algorithm logic
        // goes in algoLogic
        virtual void onStep() = 0;
        // user implemented method that requires data handling logic to be implemented
        virtual void algoLogic() = 0;
        
        // user implemented method that determines what happens an order reaches the "exchange" (after some latency)
        virtual void onOrderArrival(Order& order) = 0;
    };

}
#endif /* Backtester_hpp */
