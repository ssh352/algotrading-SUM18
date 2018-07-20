//
//  csv.hpp
//  Backtester
//
//  Created by Peter Tolsma on 7/20/18.
//  Copyright © 2018 Peter Tolsma. All rights reserved.
//

#ifndef csv_hpp
#define csv_hpp

#include <stdio.h>
#include <iostream>
#include <vector>

constexpr unsigned NUM_COLUMNS = 18; // number of columns in each csv

// gonna throw stuff related to processing the CBOE CSV's in here for now
class Gem_CSV_Row {
public:
    friend std::istream& operator>>(std::istream& is, Gem_CSV_Row row);
private:
    std::vector<std::string> dat;
};

std::istream& operator>>(std::istream& is, Gem_CSV_Row row) {
    row.dat.reserve(NUM_COLUMNS);
    for (int i = 0; i < 17; ++i) {
        std::getline(is, row.dat[i], ',');
    }
    std::getline(is, row.dat.back());
    return is;
}
#endif /* csv_hpp */
