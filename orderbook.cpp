#include "orderbook.hpp"
#include "engine.hpp"
#include <tuple>

int Orderbook::length() {
  return book.size();
}

std::vector<std::tuple<uint32_t, uint32_t>> Orderbook::getBook() {
  return book;
}

void Orderbook::add(uint32_t price, uint32_t size) {
  book.push_back(std::make_tuple(price, size));
}

void Orderbook::remove(int index) {
  book.erase(book.begin() + index);
}

/* 
  Input:
   - Command type of buy or sell
   - Price and quantity of a buy or sell order

  Outpout:
  - 0: The order was completed
  - X > 0: Quantity remaining
  - -1: No Match found
  - -2: Logical error

  The function will remove a resting buy or sell order if fulfilled along the way
*/
uint32_t findMatch(CommandType cmd, Orderbook otherBook, uint32_t price, uint32_t count) {
  switch (cmd) {
    case input_buy: {
      // Set sell price equal to buy price
      uint32_t sellPrice = price;
      // Track the index of the tuple for the seller with lowest price
      int bestIndex = -1;
      // Loop through the sell book vector and find the lowest seller
      for(int i = 0; i<otherBook.length(); i++) {
        if (get<0>(otherBook.getBook()[i]) <= sellPrice) {
          sellPrice = get<0>(otherBook.getBook()[i]);
          bestIndex = i;
        }
      }
      // If we found a seller...
      if (bestIndex != -1) {
        // If we want to buy more than we're selling, lower our count and remove the sell order
        if (count >= get<1>(otherBook.getBook()[bestIndex])) {
          count -= get<1>(otherBook.getBook()[bestIndex]);
          otherBook.remove(bestIndex);
        // Otherwise, set our count to 0 and lower the count of the sell order
        }  else {
          get<1>(otherBook.getBook()[bestIndex]) -= count;
          count = 0;
        }
        // Return 0 if our order is sold, or how many we still need to buy
        return count;
      // Return -1 if we found no match
      } else {
        return -1;
      }
      break;
    }
    case input_sell: {
      // Set buy price equal to sell price
      uint32_t buyPrice = price;
      int bestIndex = -1;
      // Loop through the vector to find the highest seller
      for(int i = 0; i<otherBook.length(); i++) {
        if (get<0>(otherBook.getBook()[i]) >= buyPrice) {
          buyPrice = get<0>(otherBook.getBook()[i]);
          bestIndex = i;
        }
      }
      // If we found a buyer...
      if (bestIndex != -1) {
        // If we are selling more than they are buying, remove the buyer and lower our sell count
        if (count >= get<1>(otherBook.getBook()[bestIndex])) {
          count -= get<1>(otherBook.getBook()[bestIndex]);
          otherBook.remove(bestIndex);
        // Otherwise, set our count to 0 and subtract our count from the buyers order
        }  else {
          get<1>(otherBook.getBook()[bestIndex]) -= count;
          count = 0;
        }
        // Same return logic as buying
        return count;
      } else {
        return -1;
      }
      break;
    }
    default: {return -2;}
  }
}

void handleOrder(std::string ticker, CommandType cmd, uint32_t price, uint32_t count) {
  Engine f;
  orderBookHash orderMap = f.getOrderBookMap();
  // Retrieve otherBook param for findMatch
  Orderbook otherBook;
  switch (cmd) {
  case input_buy: {
    otherBook = get<0>(orderMap.at(ticker));
    break;
  }
  case input_sell: {
    otherBook = get<1>(orderMap.at(ticker));
    break;
  }
  default: {}
  // End switch
  }
  // Find a match such that shares are left
  while (count > 0) {
    count = findMatch(cmd, otherBook, price, count);
  }
  // If count is 0, order is handled
  if (count == 0) {
    return;
  }
  // Otherwise, update buy book if count is non-zero
  if (cmd == input_buy) {
    f.updateBuyBook(ticker, price, count);
  // Update sell book if command is sell
  } else {
    f.updateSellBook(ticker, price, count);
  }
}