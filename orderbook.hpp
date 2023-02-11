#ifndef ORDERBOOK_HPP
#define ORDERBOOK_HPP

#include <vector>
#include <tuple>

struct Orderbook {

public:
  // Arrays of tuples
  std::vector<std::tuple<float, int>> book;

  // Accessors
  int length() const;
  const std::vector<std::tuple<float, int>>& getBook() const;

  // Mutators
  void add_new(float price, int size);
  void remove_from(int index);
};

#endif