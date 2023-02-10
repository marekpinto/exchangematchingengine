#include "orderbook.hpp"

int orderbook::length() const {
  return book.size();
}

const std::vector<std::tuple<float, int>>& orderbook::getBook() const {
  return book;
}

void orderbook::add(float price, int size) {
  book.push_back(std::make_tuple(price, size));
}

void orderbook::remove(int index) {
  book.erase(book.begin() + index);
}