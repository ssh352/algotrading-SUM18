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
#include <boost/filesystem.hpp>
#include <functional>
#include <vector>
#include <algorithm>

using namespace Backtester;
using namespace boost::posix_time;
namespace fs = boost::filesystem;
using std::string;

int main(int argc, const char * argv[]) {
    typedef boost::multiprecision::cpp_dec_float_50 decimal;
    string path = "/Users/pejato/Documents/Money/Data/Gemini/item_000007803";
    std::vector<string> fileList;
    for (auto& p : fs::directory_iterator(path))
        fileList.push_back(p.path().native());
    
    std::sort(fileList.begin(), fileList.end());
    return 0;
}
