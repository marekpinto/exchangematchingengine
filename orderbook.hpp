#ifndef ORDERBOOK_HPP
#define ORDERBOOK_HPP

#include <vector>
#include <tuple>

struct orderbook {

public:
  // Arrays of tuples
  std::vector<std::tuple<float, int>> book;

  // Accessors
  int length() const;
  const std::vector<std::tuple<float, int>>& getBook() const;

  // Mutators
  void add(float price, int size);
  void remove(int index);
};

#endif