//
//  main.cpp
//  HFT-Backtester
//
//  Created by Alec Goldberg on 7/21/18.
//  Copyright © 2018 Alec Goldberg. All rights reserved.
//

#include <iostream>
#include <map>
#include <boost/multiprecision/cpp_dec_float.hpp>

int main(int argc, const char * argv[]) {
    typedef boost::multiprecision::cpp_dec_float_50 decimal;
    std::map<decimal, std::string> myMap;
    myMap[decimal("1.233")] = "boo";
    myMap[decimal("1.234")] = "foo";
    myMap[decimal("1.232")] = "poo";
    for (auto& p : myMap)
    {
        std::cout << p.second << std::endl;
    }
    std::cout << "Hello, World!\n";
    return 0;
}
