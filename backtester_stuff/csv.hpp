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
#include <queue>

// gonna throw stuff related to processing the CBOE CSV's in here for now

namespace Backtester {
    constexpr unsigned NUM_COLUMNS = 18; // number of columns in each csv

    class Gem_CSV_Row {
    public:
        friend std::istream& operator>>(std::istream& is, Gem_CSV_Row row) {
            row.dat.reserve(NUM_COLUMNS);
            for (int i = 0; i < NUM_COLUMNS; ++i) {
                std::getline(is, row.dat[i], ',');
            }
            std::getline(is, row.dat.back());
            return is;
        }
    private:
        std::vector<std::string> dat;
    };

//    std::istream& operator>>(std::istream& is, Gem_CSV_Row row) {
//        row.dat.reserve(NUM_COLUMNS);
//        for (int i = 0; i < NUM_COLUMNS; ++i) {
//            std::getline(is, row.dat[i], ',');
//        }
//        std::getline(is, row.dat.back());
//        return is;
//    }

    class Gem_CSV_File {
    public:
        Gem_CSV_File(std::ifstream& in_f);
        Gem_CSV_Row getNextLine();
    private:
        std::queue<Gem_CSV_Row> rows;
    };
}
#endif /* csv_hpp */
