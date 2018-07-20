//
//  Tick.hpp
//  Backtester
//
//  Created by Peter Tolsma on 6/13/18.
//  Copyright © 2018 Peter Tolsma. All rights reserved.
//

#ifndef Tick_hpp
#define Tick_hpp

#include <stdio.h>
#include "Time_Event.hpp"

namespace Gemini {
    enum EventType : int {
        Initial,
        Place,
        Cancel,
        Fill
    };
    enum ExecutionOptions : int {
        NIL,
        maker_or_cancel,
        immediate_or_cancel,
        auction_only, // believe this one won't show up in the CBOE data
        indication_of_interest // believe this one won't show up in the CBOE data
    };
    enum Side : bool {
        buy,
        sell
    };
    enum OrderType : int {
        limit,
        market
    };
}

namespace Backtester {
    // this will be gemini specific for now TOTO move to a templated system to improve generality
    class Tick : public Time_Event {
    public:
        const Gemini::EventType event_t;
        const Gemini::ExecutionOptions ex_opts;
        const Gemini::Side side;
        const Gemini::OrderType order_type;
        const decimal price_level;
        
        // TOTO code the logic for this using a typical line from the gemini CBOE csv as example
        Tick(const std::string& csv_line);
    };
}

#endif /* Tick_hpp */
