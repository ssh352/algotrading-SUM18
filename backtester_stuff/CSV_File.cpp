//
//  CSV_File.cpp
//  HFT-Backtester
//
//  Created by Alec Goldberg on 7/21/18.
//  Copyright © 2018 Alec Goldberg. All rights reserved.
//

#include "CSV_File.hpp"
#include <queue>
#include <fstream>
#include <stdio.h>
#include <iostream>

using namespace Backtester;

Gem_CSV_File::Gem_CSV_File(){
    
}

Gem_CSV_File::Gem_CSV_File(std::istream& in_f){
    
    Gem_CSV_Row row;
    while(in_f >> row)
    {
        rows.push(row);
        row.PrintRow();
    }
}


