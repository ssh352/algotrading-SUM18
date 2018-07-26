//
//  main.cpp
//  HFT-Backtester
//
//  Created by Alec Goldberg on 7/21/18.
//  Copyright © 2018 Alec Goldberg. All rights reserved.
//

#include <iostream>
#include <fstream>
#include <map>
#include "CSV_File.hpp"
#include "Level2OrderBook.hpp"
#include <boost/multiprecision/cpp_dec_float.hpp>

#include <functional>

using namespace Backtester;

int main(int argc, const char * argv[]) {
    typedef boost::multiprecision::cpp_dec_float_50 decimal;
    auto cmp = std::greater<decimal>();
    decimal d("1.5");
    decimal d2("2");
    std::cout << cmp(d2, d) << std::endl;
    return 0;
}
