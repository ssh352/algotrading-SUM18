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
        onStep();
        updateRunningReturnMean();
        updateRunningReturnVar();
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
        while (time_duration(currentTime - sentOrders.front().second).total_milliseconds() >= LATENCY)
        {
            onOrderArrival(sentOrders.front().first);
            sentOrders.pop();
        }
    }

    /////////////
    // PRIVATE //
    /////////////
    
    void Engine::updateRunningReturnMean()
    {
        // TODO implement per the wikipedia article
    }
    
    void Engine::updateRunningReturnVar()
    {
        // TODO implement per the wikipedia article
    }
}
