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

using namespace Backtester;


Gem_CSV_Row::Gem_CSV_Row()
{
}

void Gem_CSV_Row::PrintRow()
{
    for(auto &val: dat)
    {
        std::cout << val << " ";
    }
    std::cout << "\n";
}
