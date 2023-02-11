#ifndef ORDERBOOK_HPP
#define ORDERBOOK_HPP

#include <vector>
#include <tuple>
#include "io.hpp"

struct Orderbook {

public:
  // Arrays of tuples
  std::vector<std::tuple<uint32_t, uint32_t>> book;

  // Accessors
  int length();
  std::vector<std::tuple<uint32_t, uint32_t>> getBook();

  // Mutators
  void add(uint32_t price, uint32_t size);
  void remove(int index);
  std::tuple<uint32_t, uint32_t> findMatch(CommandType cmd);
};

#endif