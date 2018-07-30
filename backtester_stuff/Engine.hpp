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

namespace Backtester
{
    
template<typename DAT_TYPE> // the time resolutions aren't finalized, but will be of : tick, 1-min bar, 5-min bar, etc.
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
        
        // TODO set in ctor, the (constant for now) latency in milliseconds we assume to be between sending and
        // receiving an order
        const unsigned LATENCY;
        
        // see wiki page above
        void updateRunningReturnMean();
        // see above
        void updateRunningReturnVar();
        
    public:

        // (eventually) the big papi method, will run the whole shebang
        void runBacktest();

    protected:
        
        // this is just an idea.. But to prevent overestimation of profits, might be a good idea to not allow the algo
        // to do too many trades in a short time. This is because in reality we'd have at most a few trades before we
        // move the price too much, but in this backtester we can't really simulate this (as this would require some
        // crazy fucking math). This bool will tell the derived class that we implement whether we're allowed to trade.
        bool inLockoutPeriod;
        
        // will hold orders that have been sent but not yet received (due to latency) by the "exchange"
        // TODO either the Order class needs to implement a time sent field or we need to store it in the queue somehow,
        //      possible with std::pair (?), then in what format should time be? seconds since epoch?
        std::queue<Order> sentOrders;
        
        // user implemented method that requires data handling logic to be implemented
        virtual void onData(DAT_TYPE dat) = 0;
        
        // user implemented method that determines what happens an order reaches the "exchange" (after some latency)
        virtual void onOrderArrival(Order& order) = 0;
    };

}
#endif /* Backtester_hpp */
