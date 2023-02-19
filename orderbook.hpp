#ifndef ORDERBOOK_HPP
#define ORDERBOOK_HPP

#include <vector>
#include <tuple>
#include <mutex>
#include "io.hpp"

struct Orderbook {

public:
  // Arrays of tuples
  std::vector<std::tuple<int, int, int, int, long long>> book;
  mutable std::mutex mut;

  // Accessors
  size_t length();
  std::vector<std::tuple<int, int, int, int, long long>> getBook();
  void print_counts();
  // Mutators
  void incrementExId(size_t index);
  void decrementCount(size_t index, int numSubtracted);
  void add(int price, int size, int id, long long timestamp);
  void remove(int index);
  bool removeById(int id);
  void decrementCountById(int id, int numSubtracted);
  bool contains(int id);
  int  findMatch(CommandType cmd, int price, int count, int activeId, long long timestamp);
};

#endif
