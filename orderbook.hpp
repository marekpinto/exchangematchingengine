#ifndef ORDERBOOK_HPP
#define ORDERBOOK_HPP

#include <vector>
#include <tuple>
#include "io.hpp"

struct Orderbook {

public:
  // Arrays of tuples
  std::vector<std::tuple<int, int, int, int>> book;

  // Accessors
  int length();
  std::vector<std::tuple<int, int, int, int>> getBook();
  void print_counts();
  // Mutators
  void add(int price, int size, int id);
  void remove(int index);
  bool removeById(int id);
  int static findMatch(CommandType cmd, Orderbook* otherBookRef, int price, int count, int activeId);
};

#endif
