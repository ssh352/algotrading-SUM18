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

Level2OrderBook::Level2OrderBook(const Gem_CSV_File& csv)
:file(csv)
{
    // initialization step puts stuff on the book that was there from the previous day
    std::vector<Gem_CSV_Row> initials = file.getInitials();
    bids.reserve(initials.size()); // could even possibly reserve csv size for each
    asks.reserve(initials.size()); // it would avoid ANY resizing
    
    std::vector<std::pair<decimal, decimal>>* temp;
    for (Gem_CSV_Row& row : initials)
    {
        temp = (row["Side"] == "buy" ? &bids : &asks);
        
        if (temp->empty())
            temp->push_back({ decimal(row["LimitPrice"]), decimal(row["OriginalQ"]) });
        else
        {
            // we may want to insert and then sort after for better performance
            // depending on size of "initial"
            decimal level = decimal(row["LimitPrice"]);
            decimal quantity = decimal(row["OriginalQ"]);
            auto it = std::lower_bound(temp->begin(), temp->end(), level,
                                       [](std::pair<decimal, decimal> p, decimal price){ return p.first > price; }
                                       );
            if (it->first == level)
                it->second += quantity;
            else
                temp->insert(it, { level, quantity });
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
    if(price > midPrice)
    {
        it = asks.begin();
        while(it->first < price)
            ++it;
    }
    else{
        it = bids.begin();
        while(it->first > price)
            ++it;
    }
    
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
    
    if(price > midPrice)
    {
        it = asks.begin();
        while(it->first < price)
            ++it;
    }
    else{
        it = bids.begin();
        while(it->first > price)
            ++it;
    }
    
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

void Level2OrderBook::fillOrder(decimal price, decimal quantity, bool buy)
{
    if (quantity == 0)
        return;
    
    std::vector<std::pair<decimal, decimal>>::iterator it;
    
    // seems like these should work
    if(buy)
    {
        it = bids.begin();
        while(it->first > price)
            ++it;
    }
    else{
        it = asks.begin();
        while(it->first < price)
            ++it;
    }
    
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

void Level2OrderBook::processCSVLine(const Gem_CSV_Row& line)
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
        if(line["OrderType"] == "Limit")
        {
            fillOrder(decimal(line["LimitPrice"]), decimal(line["OriginalQ"]), (line["Side"] == "buy"));
        }
    }
    else
    {
        throw std::runtime_error("Expected EventType to be of [Place, Cancel, Fill] but got " + type);
    }
}

std::pair<std::vector<std::pair<decimal, decimal>>,
std::vector<std::pair<decimal, decimal>>> Level2OrderBook::nClosestLevels(size_t n)
{
    return std::make_pair(nClosestBids(n),
            nClosestAsks(n));
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

// seems like it needs to take into account if we have 0 quantity in a price
void Level2OrderBook::updateMid()
{
    midPrice = (asks.front().first + bids.front().first) / 2;
}
