//
//  CSV_Row.hpp
//  Backtester
//
//  Created by Peter Tolsma on 7/20/18.
//  Copyright © 2018 Peter Tolsma. All rights reserved.
//

#ifndef CSV_Row_hpp
#define CSV_Row_hpp

#include <stdio.h>
#include <iostream>
#include <vector>
#include <queue>

// gonna throw stuff related to processing the CBOE CSV's in here for now

namespace Backtester {
    
    constexpr unsigned NUM_COLUMNS = 18; // number of columns in each csv

    class Gem_CSV_Row {
        
    public:
        
        Gem_CSV_Row();
        
        friend std::istream& operator>>(std::istream& is, Gem_CSV_Row &row) {
            row.dat.resize(NUM_COLUMNS, "");
            
            // reads each field until the next comma
            for (int i = 0; i < NUM_COLUMNS-1; ++i) {
                std::getline(is, row.dat[i], ',');
            }
            
            std::getline(is, row.dat.back());
            
            return is;
        }
        
        void PrintRow();
        
    private:
        
       std::vector<std::string> dat;
        
    };

  
}
#endif /* CSV_Row_hpp */
