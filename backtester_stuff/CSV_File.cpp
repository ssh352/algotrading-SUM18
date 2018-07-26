//
//  CSV_File.cpp
//  HFT-Backtester
//
//  Created by Alec Goldberg on 7/21/18.
//  Copyright © 2018 Alec Goldberg. All rights reserved.
//

#include "CSV_File.hpp"
#include <iostream>

using namespace Backtester;

Gem_CSV_File::Gem_CSV_File() {}

Gem_CSV_File::Gem_CSV_File(std::istream& in_f)
{
    Gem_CSV_Row row;
    std::string tmp;
    getline(in_f, tmp); // getting rid of header
    while (in_f >> row)
    {
        rows.push(row);
//        row.PrintRow();
    }
}

Gem_CSV_Row Gem_CSV_File::getNextLine()
{
    Gem_CSV_Row temp = rows.front();
    rows.pop();
    return temp;
}

std::vector<Gem_CSV_Row> Gem_CSV_File::getInitials()
{
    std::vector<Gem_CSV_Row> initials;
    
    while (rows.front()["EventType"] == "Initial")
    {
        initials.push_back(getNextLine());
    }
    
    return initials;
}


