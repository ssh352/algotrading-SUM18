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
    
    std::vector<std::pair<decimal, decimal>>::iterator it;
    if (price > midPrice) // look in asks
        it = asks.begin();
    else
        it = bids.begin(); // look in bids
    
    
    if (it == asks.begin())
        while (price < it->first) // moving up through the asks
            ++it;
    else
        while (price > it->first) // moving down through the bids
            ++it;
    
    
    if (it->first == price) // the price level exists in the book, so add to its quantity
        it->second += quantity;
    else
        // if price > midPrice we're adding to asks, else we're adding to bids
        (price > midPrice ? asks : bids).insert(it, { price, quantity }); // level doesn't exist yet, so we create it
    
    updateMid();
}

void Level2OrderBook::removeFromPriceLevel(decimal price, decimal quantity)
{
    if (quantity == 0)
        return;
    
    std::vector<std::pair<decimal, decimal>>::iterator it;
    if (price > midPrice) // look in asks
        it = asks.begin();
    else
        it = bids.begin(); // look in bids
    
    
    if (it == asks.begin())
        while (price < it->first) // moving up through the asks
            ++it;
    else
        while (price > it->first) // moving down through the bids
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
    updateMid();
}

void Level2OrderBook::processCSVLine(Gem_CSV_Row line)
{
    std::string type = line["EventType"];
    if (type == "Place")
    {
        addToPriceLevel(decimal(line["LimtPrice"]), decimal(line["OriginalQ"]));
    }
    else if (type == "Cancel")
    {
        removeFromPriceLevel(decimal(line["LimitPrice"]), decimal(line["OriginalQ"]));
    }
    else if (type == "Fill")
    {
        
    }
    else
    {
        throw std::runtime_error("Expected EventType to be of [Place, Cancel, Fill] but got " + type);
    }
}

std::pair<std::vector<std::pair<decimal, decimal>>,
std::vector<std::pair<decimal, decimal>>> Level2OrderBook::nClosestLevels(size_t n)
{
    return { std::vector<std::pair<decimal, decimal>>(bids.begin(), bids.begin() + n),
             std::vector<std::pair<decimal, decimal>>(asks.begin(), asks.begin() + n)
    };
}

std::vector<std::pair<decimal, decimal>> Level2OrderBook::nClosestAsks(size_t n)
{
    return std::vector<std::pair<decimal, decimal>>(asks.begin(), asks.begin() + n);
}
std::vector<std::pair<decimal, decimal>> Level2OrderBook::nClosestBids(size_t n)
{
    return std::vector<std::pair<decimal, decimal>>(bids.begin(), bids.begin() + n);
}

/////////////
// PRIVATE //
/////////////

void Level2OrderBook::updateMid()
{
    midPrice = (asks.front().first + bids.front().first) / 2;
}
