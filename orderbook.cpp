#include "orderbook.hpp"

int Orderbook::length() const {
  return book.size();
}

const std::vector<std::tuple<float, int>>& Orderbook::getBook() const {
  return book;
}

void Orderbook::add_new(float price, int size) {
  book.push_back(std::make_tuple(price, size));
}

void Orderbook::remove_from(int index) {
  book.erase(book.begin() + index);
}