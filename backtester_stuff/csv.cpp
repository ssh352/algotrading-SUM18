//
//  csv.cpp
//  Backtester
//
//  Created by Peter Tolsma on 7/20/18.
//  Copyright © 2018 Peter Tolsma. All rights reserved.
//

#include "csv.hpp"
#include <iterator>

using namespace Backtester;
Gem_CSV_File::Gem_CSV_File(std::ifstream& in_f) {
    Gem_CSV_Row row;
    in_f >> row;
}
