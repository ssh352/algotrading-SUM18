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
#include <unordered_map>

// gonna throw stuff related to processing the CBOE CSV's in here for now

namespace Backtester {
    
    constexpr unsigned NUM_COLUMNS = 18; // number of columns in each csv

    class Gem_CSV_Row {
        
    public:
        
        Gem_CSV_Row();
        
        friend std::istream& operator>>(std::istream& is, Gem_CSV_Row &row)
        {
            // reads each field until the next comma
            for (int i = 0; i < NUM_COLUMNS-1; ++i) {
                std::getline(is, row.dat[row.header[i]], ',');
            }
            
            std::getline(is, row.dat[row.header.back()]);
            
            return is;
        }
        const std::string& operator[](std::string key) const;
        void PrintRow();
        
    private:
        // the valid keys for each row, they correspond to fields in the actual CSV file
        const std::vector<std::string> header = { "EventID", "EventDate", "EventTime", "EventMillis", "OrderID", "Options", "EventType", "Symbol", "OrderType", "Side", "LimitPrice", "OriginalQ", "GrossNotional", "FillPrice", "FillQ", "TotalExecQ", "RemainingQ", "AvgPrice"
        };
        std::unordered_map<std::string, std::string> dat;
        
    };

}
#endif /* CSV_Row_hpp */
