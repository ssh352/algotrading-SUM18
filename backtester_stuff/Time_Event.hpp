//
//  Time_Event.hpp
//  Backtester
//
//  Created by Peter Tolsma on 6/13/18.
//  Copyright © 2018 Peter Tolsma. All rights reserved.
//

#ifndef Time_Event_hpp
#define Time_Event_hpp

#include <stdio.h>
#include <cstdint>
#include <string>
#include <boost/multiprecision/cpp_dec_float.hpp>

namespace mp = boost::multiprecision;
namespace Backtester {

    typedef mp::cpp_dec_float_50 decimal;
    class Time_Event {

        // unix style epoch timestamp
        uint64_t timestamp;
        // may be vendor/exchange supplied identifier, or somehow determined by user
        uint64_t eventID;
        // 6 letter symbol of updated instrument, eg BTCUSD // TOTO better to use c_str() or enum?
        std::string symbol;

    };
}

#endif /* Time_Event_hpp */
