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
#include <memory>
#include <shared_mutex>
#include <condition_variable>

#include "io.hpp"
#include "orderbook.hpp"

typedef std::unordered_map< std::string, std::tuple<std::shared_ptr<Orderbook>, std::shared_ptr<Orderbook>, std::shared_ptr<std::mutex>> > orderBookHash;

struct Engine
{
public:
	Engine();
	void accept(ClientConnection conn);
	void updateBuyBook(std::string ticker, int price, int count, int id);
	void updateSellBook(std::string ticker, int price, int count, int id);
	bool handleOrder(std::string ticker, CommandType cmd, int price, int count, int id);
	orderBookHash instrumentMap;
	mutable std::mutex instrumentMut;
	mutable std::mutex timestampMut;
	int timestamp;
	int getCurrentTimestamp();

private:
	void connection_thread(ClientConnection conn);
};



#endif