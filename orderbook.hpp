#ifndef ORDERBOOK_HPP
#define ORDERBOOK_HPP

#include <vector>
#include <tuple>
#include <mutex>
#include "io.hpp"

struct Orderbook {

public:
  // Arrays of tuples
  std::vector<std::tuple<int, int, int, int>> book;
  mutable std::mutex mut;

  // Accessors
  int length();
  std::vector<std::tuple<int, int, int, int>> getBook();
  void print_counts();
  // Mutators
  void incrementExId(int index);
  void decrementCount(int index, int numSubtracted);
  void add(int price, int size, int id);
  void remove(int index);
  bool removeById(int id);
  int  findMatch(CommandType cmd, int price, int count, int activeId);
};

#endif
