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
        Candle() = default;
        Candle(std::string _open, std::string _high, std::string _low, std::string _close, std::string _volume):
            open(_open), high(_high), low(_low), close(_close), volume(_volume) {}
    private:
        decimal open;
        decimal high;
        decimal low;
        decimal close;
        decimal volume;
    };
}

#endif /* Candle_hpp */
