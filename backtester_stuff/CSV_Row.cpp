//
//  csv.cpp
//  Backtester
//
//  Created by Peter Tolsma on 7/20/18.
//  Copyright © 2018 Peter Tolsma. All rights reserved.
//

#include "CSV_Row.hpp"
#include <iterator>
#include <iostream>
#include <sstream>
#include <stdexcept>

namespace Backtester {

const std::vector<std::string> Gem_CSV_Row::header = { "EventID", "EventDate", "EventTime", "EventMillis", "OrderID",
                                                 "Options", "EventType", "Symbol", "OrderType", "Side", "LimitPrice",
                                                 "OriginalQ", "GrossNotional", "FillPrice", "FillQ", "TotalExecQ",
                                                 "RemainingQ", "AvgPrice"
};
    
Gem_CSV_Row::Gem_CSV_Row() {}

void Gem_CSV_Row::PrintRow()
{
    for (auto &val: dat)
    {
        std::cout << val.second << " ";
    }
    std::cout << "\n";
}

const std::string& Gem_CSV_Row::operator[](std::string key) const
{
    auto it = dat.find(key);
    if (it != dat.end())
    {
        return it->second;
    }
    else
    {
        throw std::invalid_argument("Invalid key: " + key);
    }
}

}
