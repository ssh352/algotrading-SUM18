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

Gem_CSV_File::Gem_CSV_File()
:index(0)
{}

Gem_CSV_File::Gem_CSV_File(std::istream& in_f)
:index(0)
{
    Gem_CSV_Row row;
    std::string tmp;
    getline(in_f, tmp); // getting rid of header
    while (in_f >> row)
    {
        rows.push_back(row);
//        row.PrintRow();
    }
}

Gem_CSV_File::Gem_CSV_File(const Gem_CSV_File & other)
:index(0),rows(other.rows)
{
}


Gem_CSV_Row Gem_CSV_File::getNextLine() const
{
    return rows[index++];
}

std::vector<Gem_CSV_Row> Gem_CSV_File::getInitials() const
{
    std::vector<Gem_CSV_Row> initials;
    
    while (rows.front()["EventType"] == "Initial")
    {
        initials.push_back(getNextLine());
    }
    
    return initials;
}

size_t  Gem_CSV_File::getNumRows() const
{
    return rows.size();
}


