#include "orderbook.hpp"
#include "engine.hpp"
#include <tuple>
#include <mutex>
#include "io.hpp"

int Orderbook::length() {
  return book.size();
}

std::vector<std::tuple<int, int, int, int>> Orderbook::getBook() {
  std::lock_guard<std::mutex> lk(mut);
  return book;
}

void Orderbook::add(int price, int size, int id) {
  std::lock_guard<std::mutex> lk(mut);
  book.push_back(std::make_tuple(price, size, id, 0));
}

void Orderbook::remove(int index) {
  std::lock_guard<std::mutex> lk(mut);
  book.erase(book.begin() + index);

}

bool Orderbook::removeById(int id) {
  for(int i = 0; i<(int)book.size(); i++){
    if (get<2>(book[i]) == id) {   
      std::lock_guard<std::mutex> lk(mut);
	    book.erase(book.begin() + i);
      return true;
    }
  }
  return false;
}

void Orderbook::incrementExId(int index) {
  std::lock_guard<std::mutex> lk(mut);
  get<3>(book[index]) += 1;
}

void Orderbook::decrementCount(int index, int numSubtracted) {
  std::lock_guard<std::mutex> lk(mut);
  get<1>(book[index]) -= numSubtracted;
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
int  Orderbook::findMatch(CommandType cmd, int price, int count, int activeId) {
switch (cmd) {
    case input_buy: {
      // Set sell price equal to buy price
      int sellPrice = price;
      // Track the index of the tuple for the seller with lowest price
      int bestIndex = -1;
      // Loop through the sell book vector and find the lowest seller
      {
        std::lock_guard<std::mutex> lk(mut);
        for(int i = length()-1; i>=0; i--) {
          if (get<0>(book[i]) <= sellPrice) {
            sellPrice = get<0>(book[i]);
            bestIndex = i;
          }
        }
      }
      // If we found a seller...
      if (bestIndex != -1) {
        incrementExId(bestIndex);
        std::lock_guard<std::mutex> lk(mut);
        // If we want to buy more than we're selling, lower our count and remove the sell order
        if (count >= get<1>(book[bestIndex])) {
	        count -= get<1>(book[bestIndex]);
          Output::OrderExecuted(get<2>(book[bestIndex]), activeId, get<3>(book[bestIndex]), get<0>(book[bestIndex]), get<1>(book[bestIndex]), getCurrentTimestamp());
	        remove(bestIndex);
          // Otherwise, set our count to 0 and lower the count of the sell order
        }  else {
          decrementCount(bestIndex, count);
          Output::OrderExecuted(get<2>(book[bestIndex]), activeId, get<3>(book[bestIndex]), get<0>(book[bestIndex]), count, getCurrentTimestamp());
          count = 0;
        }
        // Return 0 if our order is sold, or how many we still need to buy
      } 

      return count;
      break;
    }
    case input_sell: {
      // Set buy price equal to sell price
      int buyPrice = price;
      int bestIndex = -1;
      // Loop through the vector to find the highest seller
      {
        std::lock_guard<std::mutex> lk(mut);
        for(int i = length()-1; i>=0; i--) {
          if (get<0>(book[i]) >= buyPrice) {
            buyPrice = get<0>(book[i]);
            bestIndex = i;
          }
        }
      }
      // If we found a buyer...
      if (bestIndex != -1) {
        incrementExId(bestIndex);
        std::lock_guard<std::mutex> lk(mut);
        // If we are selling more than they are buying, remove the buyer and lower our sell count
        if (count >= get<1>(book[bestIndex])) {
          Output::OrderExecuted(get<2>(book[bestIndex]), activeId, get<3>(book[bestIndex]), get<0>(book[bestIndex]), get<1>(book[bestIndex]), getCurrentTimestamp());
          count -= get<1>(book[bestIndex]);
	        remove(bestIndex);
        // Otherwise, set our count to 0 and subtract our count from the buyers order
        }  else {
          Output::OrderExecuted(get<2>(book[bestIndex]), activeId, get<3>(book[bestIndex]), get<0>(book[bestIndex]), count, getCurrentTimestamp());
          decrementCount(bestIndex, count);
	        count = 0;
        }
      }
      return count;
      break;
    }
    default: {return -2;}
  }
}

