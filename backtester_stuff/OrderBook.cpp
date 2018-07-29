//
//  OrderBook.cpp
//  Backtester
//
//  Created by Peter Tolsma on 7/24/18.
//  Copyright © 2018 Peter Tolsma. All rights reserved.
//

#include "OrderBook.hpp"
#include <algorithm>

using namespace Backtester;

OrderBook::OrderBook(Gem_CSV_File &csvFile)
{
    std::vector<Gem_CSV_Row> initials = csvFile.removeInitials();
    // need to load up a hash map with orderID as key, order metadata (how to represent?) as value
    // two vectors to represent the asks and bids
}
