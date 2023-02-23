#include "orderbook.hpp"
#include "engine.hpp"
#include <tuple>
#include <mutex>
#include "io.hpp"

size_t Orderbook::length() {
  std::lock_guard<std::mutex> lk(mut);
  wait();
  return book.size();
}

void Orderbook::wait() {}

std::vector<std::tuple<int, int, int, int, long long>> Orderbook::getBook() {
  std::lock_guard<std::mutex> lk(mut);
  wait();
  return book;
}

void Orderbook::add(int price, int size, int id, long long timestamp) {
  std::lock_guard<std::mutex> lk(mut);
  wait();
  book.push_back(std::make_tuple(price, size, id, 0, timestamp));
}

void Orderbook::remove(int index) {
  std::lock_guard<std::mutex> lk(mut);
  wait();
  book.erase(book.begin() + index);

}

bool Orderbook::contains(int id) {
  std::lock_guard<std::mutex> lk(mut);
  wait();
  for(size_t i = 0; i<book.size(); i++){
    if (std::get<2>(book[i]) == id) {   
      return true;
    }
  }
  return false;
}

bool Orderbook::removeById(int id) {
  std::lock_guard<std::mutex> lk(mut);
  wait();
  for(size_t i = 0; i<book.size(); i++){
    if (std::get<2>(book[i]) == id) {   
	    book.erase(book.begin() + (long)i);
      return true;
    }
  }
  return false;
}

void Orderbook::incrementExId(size_t index) {
  std::lock_guard<std::mutex> lk(mut);
  wait();
  std::get<3>(book[index]) += 1;
}

void Orderbook::decrementCount(size_t index, int numSubtracted) {
  std::lock_guard<std::mutex> lk(mut);
  wait();
  std::get<1>(book[index]) -= numSubtracted;
}

void Orderbook::decrementCountById(int id, int numSubtracted) {
  std::lock_guard<std::mutex> lk(mut);
  wait();
  for(size_t i = 0; i<book.size(); i++){
    if (std::get<2>(book[i]) == id) {   
	    std::get<1>(book[i]) = numSubtracted;
    }
  }
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
int  Orderbook::findMatch(CommandType cmd, int price, int count, int activeId, Orderbook* otherBook, long long timestamp) {
	
  // std::lock_guard<std::mutex> lk(mut);
  // std::lock_guard<std::mutex> lk1(otherBook->mut);
  switch (cmd) {
    case input_buy: {
      // Set sell price equal to buy price
      int buyPrice = price;
      // Track the index of the tuple for the seller with lowest price
      // Loop through the sell book vector and find the lowest seller
      int bestIndex = -1;
      {
      std::lock_guard<std::mutex> otherLock(otherBook->mut);
      wait();
      for(int i = (int)book.size()-1; i>=0; i--) {
        if (std::get<0>(book[(size_t)i]) <= buyPrice /* && std::get<4>(book[(size_t)i]) <= timestamp && std::get<1>(book[(size_t)i]) >0*/ ) {
          buyPrice = std::get<0>(book[(size_t)i]);
          bestIndex = i;
        }
      }
      }
       //
     
      // If we found a seller..
      if (bestIndex != -1) {
       //`std::lock_guard<std::mutex> lk1(otherBook->mut);
        incrementExId((size_t)bestIndex);
        // If we want to buy more than we're selling, lower our count and remove the sell order
        if (count >= std::get<1>(book[(size_t)bestIndex])) {
	        count -= std::get<1>(book[(size_t)bestIndex]);
          Output::OrderExecuted((uint32_t)std::get<2>(book[(size_t)bestIndex]), (uint32_t)activeId, (uint32_t)std::get<3>(book[(size_t)bestIndex]), (uint32_t)std::get<0>(book[(size_t)bestIndex]), (uint32_t)std::get<1>(book[(size_t)bestIndex]), timestamp);
	        remove(bestIndex);
          // Otherwise, set our count to 0 and lower the count of the sell order
        }  else {
          Output::OrderExecuted((uint32_t)std::get<2>(book[(size_t)bestIndex]), (uint32_t)activeId, (uint32_t)std::get<3>(book[(size_t)bestIndex]), (uint32_t)std::get<0>(book[(size_t)bestIndex]), (uint32_t)count, timestamp);
	  decrementCount((size_t)bestIndex, count);
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
      // Loop through the vector to find the highest seller
      
      //std::lock_guard<std::mutex> lk(mut);
      int bestIndex = -1;
       //
       // std::lock_guard<std::mutex> otherLock(otherBook->mut); //
       {
        std::lock_guard<std::mutex> lk(mut);
        wait();
        for(int i = (int)book.size()-1; i>=0; i--) {
          if (std::get<0>(book[(size_t)i]) >= buyPrice /* && std::get<4>(book[(size_t)i]) <= timestamp && std::get<1>(book[(size_t)i]) > 0 */) {
            buyPrice = std::get<0>(book[(size_t)i]);
            bestIndex = (int)i;
          }
        }
       }
       //
      // If we found a buyer...
      if (bestIndex != -1) {
       // std::lock_guard<std::mutex> lk1(otherBook->mut);
        incrementExId((size_t)bestIndex);
        // If we are selling more than they are buying, remove the buyer and lower our sell count
	if (count >= std::get<1>(book[(size_t)bestIndex])) {
          Output::OrderExecuted((uint32_t)std::get<2>(book[(size_t)bestIndex]), (uint32_t)activeId, (uint32_t)std::get<3>(book[(size_t)bestIndex]), (uint32_t)std::get<0>(book[(size_t)bestIndex]), (uint32_t)std::get<1>(book[(size_t)bestIndex]), timestamp);
          count -= std::get<1>(book[(size_t)bestIndex]);
	        remove(bestIndex);
        // Otherwise, set our count to 0 and subtract our count from the buyers order
        }  else {
          Output::OrderExecuted((uint32_t)std::get<2>(book[(size_t)bestIndex]), (uint32_t)activeId, (uint32_t)std::get<3>(book[(size_t)bestIndex]), (uint32_t)std::get<0>(book[(size_t)bestIndex]), (uint32_t)count, timestamp);
          decrementCount((size_t)bestIndex, count);
	        count = 0;
        }
      }
      return count;
      break;
    }
    default: {return -2;}
  }
}
