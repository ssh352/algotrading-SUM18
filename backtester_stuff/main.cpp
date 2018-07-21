//
//  main.cpp
//  Backtester
//
//  Created by Peter Tolsma on 6/13/18.
//  Copyright © 2018 Peter Tolsma. All rights reserved.
//
#include <fstream>
#include <iostream>
//#include "Engine.hpp"
//#include "Candle.hpp"
#include "CSV_File.hpp"
#include <boost/multiprecision/cpp_dec_float.hpp>

namespace mp = boost::multiprecision;
using namespace Backtester;




int main(int argc, const char * argv[]) {

    mp::cpp_dec_float_50 d("4.1111234");
    std:: cout << d << std::endl;
    
    std::ifstream inFile("test.csv");
    Gem_CSV_File g(inFile);
    return 0;

}

