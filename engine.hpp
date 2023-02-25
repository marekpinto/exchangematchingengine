// This file contains declarations for the main Engine class. You will
// need to add declarations to this file as you develop your Engine.

#ifndef ENGINE_HPP
#define ENGINE_HPP

#include <chrono>

#include <vector>
#include <tuple>
#include <string>
#include <unordered_map>
#include <mutex>
#include<unordered_map>

#include "io.hpp"
#include "orderbook.hpp"

// the tuple is of form (price, size)
// to sort: sort(orderBook.start(), orderBook.end());
typedef std::unordered_map< std::string, std::tuple<Orderbook *, Orderbook *> > orderBookHash;
typedef std::unordered_map<int, int> timeMap;


struct Engine
{
public:
	Engine();
	void accept(ClientConnection conn);
	void updateBuyBook(std::string ticker, int price, int count, int id);
	void updateSellBook(std::string ticker, int price, int count, int id);
	bool handleOrder(std::string ticker, CommandType cmd, int price, int count, int id);
	Orderbook createBook();
	orderBookHash instrumentMap;
	timeMap timestampMap;
	mutable std::mutex instrumentMut;
	mutable std::mutex timestampMut;
	int timestamp;
	int getCurrentTimestamp();
private:
	void connection_thread(ClientConnection conn);

	// tracks a mapping from instrument names to a buy orderBook and sell orderBook, respectively.
	//orderBookHash instrumentMap;

};




inline std::chrono::microseconds::rep getCurrentTimestamp() noexcept
{
	return std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
}

#endif
