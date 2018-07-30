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
#include <boost/date_time/posix_time/posix_time.hpp>

#include <functional>

using namespace Backtester;
using namespace boost::posix_time;
using std::string;

int main(int argc, const char * argv[]) {
    typedef boost::multiprecision::cpp_dec_float_50 decimal;
    string s1 = "2018-06-11 00:00:47.524";
    string s2 = "2018-06-11 00:00:47.782";
    ptime t1 = time_from_string(s1);
    ptime t2 = time_from_string(s2);
    time_duration td = t2 - t1;
    std::cout << td.total_milliseconds() << std::endl;
    return 0;
}
