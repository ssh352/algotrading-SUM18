//
//  CSV_File.hpp
//  HFT-Backtester
//
//  Created by Alec Goldberg on 7/21/18.
//  Copyright © 2018 Alec Goldberg. All rights reserved.
//

#ifndef CSV_File_hpp
#define CSV_File_hpp
#include "CSV_Row.hpp"
#include <stdio.h>
#include <fstream>
#include <queue>
#endif /* CSV_File_hpp */

namespace Backtester {
    
    class Gem_CSV_File {
        
    public:
        
        Gem_CSV_File();
        
        Gem_CSV_File(std::istream& in_f);
        
        Gem_CSV_Row getNextLine();
    private:
        
        std::queue<Gem_CSV_Row> rows;
        
    };
}
