//
//  Backtester.cpp
//  Backtester
//
//  Created by Peter Tolsma on 6/13/18.
//  Copyright © 2018 Peter Tolsma. All rights reserved.
//

#include "Engine.hpp"
using namespace boost::posix_time;

namespace Backtester {
    ////////////
    // PUBLIC //
    ////////////
    
    Engine::Engine()
    : LATENCY(200), lockoutLength(60) // arbitrarily set default latency=200ms, lockout=1mn
    {}
    
    Engine::Engine(unsigned _LATENCY, unsigned _lockoutLength)
    : LATENCY(_LATENCY), lockoutLength(_lockoutLength)
    {
    }
    
    void Engine::runBacktest()
    {
        onInitialize();
        // TODO while loop here?
        // Seems like while(receiving orders)
        onStep();
        updateRunningStats();
        resolveOrders();
        if (time_duration(currentTime - lockoutStartTime).total_seconds() >= lockoutLength)
            inLockoutPeriod = false;
        if (!inLockoutPeriod)
            algoLogic();
        if (!sentOrders.empty() && sentOrders.back().second != lockoutStartTime)
        {
            lockoutStartTime = sentOrders.back().second;
            inLockoutPeriod = true;
        }
    }
    
    ///////////////
    // PROTECTED //
    ///////////////
    
    void Engine::resolveOrders()
    {
        if (sentOrders.empty())
            return;
        // processing orders as long as the time they've been in transit is greater than or equal to LATENCY
        while (time_duration(currentTime - sentOrders.front().second).total_milliseconds() >= LATENCY)
        {
            onOrderArrival(sentOrders.front().first);
            sentOrders.pop();
        }
    }

    /////////////
    // PRIVATE //
    /////////////
    
    void Engine::updateRunningStats()
    {
        ++numPeriods;
        if (numPeriods == 1)
        {
            runningReturnMean = PNL_curve.back();
            sumSquaredDiff = 0;
            runningReturnVariance = sumSquaredDiff / 0; // will produce nan, that's ok!
        }
        else
        {
            decimal prevMean = runningReturnMean;
            runningReturnMean += (PNL_curve.back() - runningReturnMean) / numPeriods;
            sumSquaredDiff += (PNL_curve.back() - prevMean)*(PNL_curve.back() - runningReturnMean);
            runningReturnVariance = sumSquaredDiff / (numPeriods - 1);
        }
    }
}
