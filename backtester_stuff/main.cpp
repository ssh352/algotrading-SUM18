//
//  main.cpp
//  HFT-Backtester
//
//  Created by Alec Goldberg on 7/21/18.
//  Copyright © 2018 Alec Goldberg. All rights reserved.
//

#include <armadillo>
#include <iostream>
#include <fstream>
#include <map>
#include "CSV_File.hpp"
#include "Level2OrderBook.hpp"
#include <boost/multiprecision/cpp_dec_float.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/filesystem.hpp>
#include <functional>
#include <limits>
#include <iomanip>
#include <initializer_list>
#include <vector>
#include <algorithm>
#include <sstream>
#include <string>

using namespace Backtester;
using namespace boost::posix_time;
namespace fs = boost::filesystem;
using std::string;

int main(int argc, const char * argv[]) {
    typedef boost::multiprecision::cpp_dec_float_50 decimal;
    std::cout << std::setprecision(std::numeric_limits<double>::max_digits10);
    decimal d = std::numeric_limits<decimal>::quiet_NaN();
    std::cout << d << std::endl;
//    std::ifstream in_f ("mtcars.csv");
//    std::cout << (in_f.is_open() ? "Open" : "Closed") << std::endl;
//    arma::mat X(0, 3);
//    arma::mat Y(0, 1);
//    int xrow = 0, yrow = 0;
//    std::string s;
//    std::getline(in_f, s);
//    while (std::getline(in_f, s))
//    {
////        std::cout << s << std::endl;
//        std::istringstream ss(s);
//        std::vector<double> tmp;
//        tmp.reserve(3);
//        for (int i = 0; i < 11; ++i)
//        {
//            std::getline(ss, s, ',');
//            if (i == 1)
//                Y.insert_rows(yrow++, arma::rowvec({std::stod(s)}));
//            else if (i == 4 || i == 6 || i == 9)
//                tmp.push_back(std::stod(s));
//        }
////        std::getline(ss, s);
////        tmp.push_back(std::stod(s));
//        X.insert_rows(xrow++, arma::rowvec(tmp));
//    }
//    X.insert_cols(0, arma::colvec(X.n_rows, arma::fill::ones));
//    X.print("X: ");
//    Y.print("Y: ");
//    arma::mat estimatedCoefficients = arma::inv(X.t() * X) * X.t() * Y;
//    estimatedCoefficients.print("B hat: ");
//    arma::mat YHat = X * estimatedCoefficients;
//    YHat.print("Y hat: ");
//    YHat.insert_cols(1, Y);
//    YHat.print("Comparison (Y hat left, Y right): ");
//
//    std::cout << std::endl;
//
//    std::cout << "Intercept: " << estimatedCoefficients(0) << std::endl << "hp: " << estimatedCoefficients(1)
//              << std::endl << "wt: " << estimatedCoefficients(2) << std::endl << "am: " << estimatedCoefficients(3)
//              << std::endl;

    return 0;
}
