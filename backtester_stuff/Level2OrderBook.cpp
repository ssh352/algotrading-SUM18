//
//  Level2OrderBook.cpp
//  Backtester
//
//  Created by Peter Tolsma on 7/26/18.
//  Copyright © 2018 Peter Tolsma. All rights reserved.
//

#include <algorithm>
#include <stdexcept>

#include "Level2OrderBook.hpp"

using namespace Backtester;

////////////
// PUBLIC //
////////////

Level2OrderBook::Level2OrderBook(Gem_CSV_File csv)
{
    // initialization step puts stuff on the book that was there from the previous day
    std::vector<Gem_CSV_Row> initials = csv.getInitials();
    for (Gem_CSV_Row& row : initials)
    {
        if (row["Side"] == "buy")
        {
            if (bids.empty())
                bids.push_back({ decimal(row["LimitPrice"]), decimal(row["OriginalQ"]) });
            else
            {
                decimal level = decimal(row["LimitPrice"]);
                decimal quantity = decimal(row["OriginalQ"]);
                auto it = std::lower_bound(bids.begin(), bids.end(), level,
                                           [](std::pair<decimal, decimal> p, decimal price){ return p.first > price; }
                                           );
                if (it->first == level)
                    it->second += quantity;
                else
                    bids.insert(it, { level, quantity });
            }
        }
        // TODO lots of reduplicated code here... can probably prettify
        else // Side == sell
        {
            if (asks.empty())
                asks.push_back({ decimal(row["LimitPrice"]), decimal(row["OriginalQ"]) });
            else
            {
                decimal level = decimal(row["LimitPrice"]);
                decimal quantity = decimal(row["OriginalQ"]);
                auto it = std::lower_bound(asks.begin(), asks.end(), level,
                                           [](std::pair<decimal, decimal> p, decimal price){ return p.first < price; }
                                           );
                if (it->first == level)
                    it->second += quantity;
                else
                    asks.insert(it, { level, quantity });
            }
        }
    }
    updateMid();
#ifdef DEBUG
    for (int i = 0; i < 10; ++i)
    {
        std::cout << '[' << asks[i].first << " : " << asks[i].second << "] ";
    }
    std::cout << std::endl;
    for (int i = 0; i < 10; ++i)
    {
        std::cout << '[' << bids[i].first << " : " << bids[i].second << "] ";
    }
    std::cout << std::endl;
#endif
}

Level2OrderBook::Level2OrderBook(std::istream& in_f)
    : Level2OrderBook(Gem_CSV_File(in_f)) {}

// both addToPriceLevel and removeFromPriceLevel do a linear search for the level, because most updates are near the mid
void Level2OrderBook::addToPriceLevel(decimal price, decimal quantity)
{
    if (quantity == 0)
        return;
    
    if (price > midPrice) // look in asks
    {
        auto it = asks.begin();
        while (price < it->first)
            ++it;
        if (it->first == price)
            it->second += quantity;
        else
            asks.insert(it, { price, quantity });
    }
    else // look in bids
    {
        auto it = bids.begin();
        while (price > it->first)
            ++it;
        if (it->first == price)
            it->second += quantity;
        else
            bids.insert(it, { price, quantity });
    }
    updateMid();
}

void Level2OrderBook::removeFromPriceLevel(decimal price, decimal quantity)
{
    if (quantity == 0)
        return;
    
    if (price > midPrice) // look in asks
    {
        auto it = asks.begin();
        while (price < it->first)
            ++it;
        if (it->first == price)
        {
            if (it->second > quantity)
                it->second -= quantity;
            else
            {
                throw std::invalid_argument("Attempted to remove " + std::string(quantity) + " from level "
                                            + std::string(price) + " which only has " + std::string(it->second));
            }
        }
        else
        {
            throw std::invalid_argument("Attempted to remove " + std::string(quantity) + " from level "
                                        + std::string(price) + " but level doesn't exist");
        }
    }
    else // look in bids
    {
        auto it = bids.begin();
        while (price > it->first)
            ++it;
        if (it->first == price)
        {
            if (it->second > quantity)
                it->second -= quantity;
            else
            {
                throw std::invalid_argument("Attempted to remove " + std::string(quantity) + " from level "
                                            + std::string(price) + " which only has " + std::string(it->second));
            }
        }
        else
        {
            throw std::invalid_argument("Attempted to remove " + std::string(quantity) + " from level "
                                        + std::string(price) + " but level doesn't exist");
        }
    }
    updateMid();
}
/////////////
// PRIVATE //
/////////////

void Level2OrderBook::updateMid()
{
    midPrice = (asks.front().first + bids.front().first) / 2;
}
