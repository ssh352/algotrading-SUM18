//
//  Backtester.hpp
//  Backtester
//
//  Created by Peter Tolsma on 6/13/18.
//  Copyright © 2018 Peter Tolsma. All rights reserved.
//

#ifndef Backtester_hpp
#define Backtester_hpp

#include "Time_Event.hpp"

#include <stdio.h>
#include <vector>

namespace Backtester
{
    
template<typename TIME_RES> // the time resolutions aren't finalized, but will be of : tick, 1-min bar, 5-min bar, etc.

    class Engine
    {

    private:

        std::vector<decimal> PNL_curve; // Will store the PNL for each tick in time
        double max_drawdown;
        decimal running_return_mean;     // Online algorithm will update mean and var as per the following link:
        decimal running_return_variance; // https://en.wikipedia.org/wiki/Algorithms_for_calculating_variance#Online_algorithm
        const decimal RISK_FREE_RATE = 2; // 3-Month T-Bill ~2% used as proxy for rf rate 
        
    public:

        void run_backtest();

    private:

        virtual void on_data(TIME_RES t) = 0; // abstract method that requires user to implement with own logic
    };

}
#endif /* Backtester_hpp */
