#ifndef ORDERBOOK_HPP
#define ORDERBOOK_HPP

#include <vector>
#include <tuple>
#include "io.hpp"

struct Orderbook {

public:
  // Arrays of tuples
  std::vector<std::tuple<uint32_t, uint32_t, uint32_t, uint32_t>> book;

  // Accessors
  int length();
  std::vector<std::tuple<uint32_t, uint32_t, uint32_t, uint32_t>> getBook();

  // Mutators
  void add(uint32_t price, uint32_t size, uint32_t id);
  void remove(int index);
  bool removeById(uint32_t id);
  int static findMatch(CommandType cmd, Orderbook otherBook, uint32_t price, uint32_t count, uint32_t activeId);
};

#endif
