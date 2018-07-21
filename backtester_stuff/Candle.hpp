//
//  Candle.hpp
//  Backtester
//
//  Created by Peter Tolsma on 7/20/18.
//  Copyright © 2018 Peter Tolsma. All rights reserved.
//

#ifndef Candle_hpp
#define Candle_hpp

#include <stdio.h>
#include "Time_Event.hpp"

namespace Backtester {
    class Candle : Time_Event {
    public:
        enum Resolution : unsigned long {
            ONEMIN = 1,
            FIVEMIN = 5,
            FIFTMIN = 15,
            ONEHR = 60,
            SIXHR = 360,
            ONEDAY = 1440
        };
        Candle() = default;
        Candle(std::string _open, std::string _high, std::string _low, std::string _close, std::string _volume,
               Resolution _res):
            open(_open), high(_high), low(_low), close(_close), volume(_volume), res(_res) {}
    private:
        decimal open;
        decimal high;
        decimal low;
        decimal close;
        decimal volume;
        Resolution res;
    };
}

#endif /* Candle_hpp */
