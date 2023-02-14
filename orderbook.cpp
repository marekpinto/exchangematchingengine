#include "orderbook.hpp"
#include "engine.hpp"
#include <tuple>
#include "io.hpp"

int Orderbook::length() {
  return book.size();
}

std::vector<std::tuple<int, int, int, int>> Orderbook::getBook() {
  return book;
}

void Orderbook::print_counts() {
	for (int i = 0; i < (int)book.size(); i++) {
		std::cerr << i << " count: " << get<1>(book[i]) << std::endl;
	}
}

void Orderbook::add(int price, int size, int id) {
  book.push_back(std::make_tuple(price, size, id, 1));
}

void Orderbook::remove(int index) {

  book.erase(book.begin() + index);

}

bool Orderbook::removeById(int id) {
  for(int i = 0; i<(int)book.size(); i++){
    if (get<2>(book[i]) == id) {
  	std::cerr << "i: " << i << std::endl;
	book.erase(book.begin() + i);
      return true;
    }
  }
  return false;
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
int  Orderbook::findMatch(CommandType cmd, Orderbook* otherBookRef, int price, int count, int activeId) {
Orderbook otherBook = *otherBookRef;  
switch (cmd) {
    case input_buy: {
      // Set sell price equal to buy price
      int sellPrice = price;
      // Track the index of the tuple for the seller with lowest price
      int bestIndex = -1;
      // Loop through the sell book vector and find the lowest seller
      for(int i = otherBook.length()-1; i>=0; i--) {
        if (get<0>(otherBook.getBook()[i]) <= sellPrice) {
          sellPrice = get<0>(otherBook.getBook()[i]);
          bestIndex = i;
        }
      }
      // If we found a seller...
      if (bestIndex != -1) {
        get<3>(otherBookRef->getBook()[bestIndex]) += 1;
        // If we want to buy more than we're selling, lower our count and remove the sell order
        if (count >= get<1>(otherBook.getBook()[bestIndex])) {
	        count -= get<1>(otherBook.getBook()[bestIndex]);
          Output::OrderExecuted(get<2>(otherBook.getBook()[bestIndex]), activeId, get<3>(otherBook.getBook()[bestIndex]), get<0>(otherBook.getBook()[bestIndex]), get<1>(otherBook.getBook()[bestIndex]), getCurrentTimestamp());
	        otherBookRef->remove(bestIndex);
          // Otherwise, set our count to 0 and lower the count of the sell order
        }  else {
          get<1>(otherBook.getBook()[bestIndex]) -= count;
          Output::OrderExecuted(get<2>(otherBook.getBook()[bestIndex]), activeId, get<3>(otherBook.getBook()[bestIndex]), get<0>(otherBook.getBook()[bestIndex]), count, getCurrentTimestamp());
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
      int buyPrice = price;
      int bestIndex = -1;
      // Loop through the vector to find the highest seller
      for(int i = otherBook.length()-1; i>=0; i--) {
        if (get<0>(otherBook.getBook()[i]) >= buyPrice) {
          buyPrice = get<0>(otherBook.getBook()[i]);
          bestIndex = i;
        }
      }
      // If we found a buyer...
      if (bestIndex != -1) {
        get<3>(otherBookRef->getBook()[bestIndex]) += 1;
        // If we are selling more than they are buying, remove the buyer and lower our sell count
        if (count >= get<1>(otherBook.getBook()[bestIndex])) {
          Output::OrderExecuted(get<2>(otherBook.getBook()[bestIndex]), activeId, get<3>(otherBook.getBook()[bestIndex]), get<0>(otherBook.getBook()[bestIndex]), get<1>(otherBook.getBook()[bestIndex]), getCurrentTimestamp());
          count -= get<1>(otherBook.getBook()[bestIndex]);
	        otherBookRef->remove(bestIndex);
        // Otherwise, set our count to 0 and subtract our count from the buyers order
        }  else {
          Output::OrderExecuted(get<2>(otherBook.getBook()[bestIndex]), activeId, get<3>(otherBook.getBook()[bestIndex]), get<0>(otherBook.getBook()[bestIndex]), count, getCurrentTimestamp());
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

