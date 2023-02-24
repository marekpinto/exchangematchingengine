#include <iostream>
#include <thread>
#include <string>
#include <mutex>
#include <tuple>
#include <unordered_map>
#include <memory>
#include <shared_mutex>

#include "io.hpp"
#include "engine.hpp"

#include "orderbook.hpp"

//makes a thread every time a connection comes in

Engine::Engine()
{  
	orderBookHash instrumentMap;
}

void Engine::accept(ClientConnection connection)
{
	auto thread = std::thread(&Engine::connection_thread, this, std::move(connection));
	thread.detach();
}

void Engine::updateBuyBook(std::string ticker, int price, int count, int id, long long timestamp)
{
	
	//std::lock_guard<std::mutex> lk(instrumentMut);
	//std::mutex bookMut  std::get<2>(instrumentMap.at(ticker));
	//std::lock_guard<std::mutex> bookLock(bookMut);
	//std::lock_guard<std::mutex> lk(bookMut);

	std::shared_ptr<Orderbook> book = std::get<0>(instrumentMap.at(ticker));
	
	book->add(price, count, id, timestamp);	
	
	}

void Engine::updateSellBook(std::string ticker, int price, int count, int id, long long timestamp)
{
	//std::lock_guard<std::mutex> lk(instrumentMut);
	//std::mutex bookMut std::get<2>(instrumentMap.at(ticker));
	//std::lock_guard<std::mutex> bookLock(bookMut);

	std::shared_ptr<Orderbook> book = std::get<1>(instrumentMap.at(ticker));
	
	book->add(price, count, id, timestamp);

}




void Engine::connection_thread(ClientConnection connection)
{
	std::unordered_map<int, std::pair<std::string, int>> orders;
	while(true)
	{
		ClientCommand input {};
		switch(connection.readInput(input))
		{
			case ReadResult::Error: SyncCerr {} << "Error reading input" << std::endl;
			case ReadResult::EndOfFile: return;
			case ReadResult::Success: break;
		}

		std::string ticker(input.instrument);
		long long time = getCurrentTimestamp();

		// Functions for printing output actions in the prescribed format are
		// provided in the Output class:
		switch(input.type)
		{

			case input_cancel: {
				SyncCerr {} << "Got cancel: ID: " << input.order_id << std::endl;

				// Remember to take timestamp at the appropriate time, or compute
				// an appropriate timestamp!
				auto output_time = getCurrentTimestamp();
				if (orders.contains((int)input.order_id)) {
					auto ordersPair = orders.at((int)input.order_id);
					std::lock_guard<std::mutex> lk(instrumentMut);
					auto correspondingTuple = instrumentMap.at(ordersPair.first);
					int type = ordersPair.second;
					//std::mutex bookMut = std::get<1>(orders.at((int)input.order_id));
					std::shared_lock<std::shared_mutex> bookLock(std::get<1>(orders.at((int)input.order_id)));
					bool result = orders.at((int)input.order_id).first -> removeById((int)input.order_id);
					if (result) {
						Output::OrderDeleted(input.order_id, true, output_time);
						orders.erase((int)input.order_id);
						break;
					}
				}
				Output::OrderDeleted(input.order_id, false, output_time); 
				break;
			}

			default: {
				SyncCerr {}
				    << "Got order: " << static_cast<char>(input.type) << " " << input.instrument << " x " << input.count << " @ "
				    << input.price << " ID: " << input.order_id << std::endl;

				// Remember to take timestamp at the appropriate time, or compute
				// an appropriate timestamp!
				// auto output_time = getCurrentTimestamp();
				bool result = Engine::handleOrder(ticker, input.type, (int)input.price, (int)input.count, (int)input.order_id, time);
				
				if (!result){
					if (input.type == input_buy) {
						//HASHMAP INSERTED
						std::lock_guard<std::mutex> lk(instrumentMut);
						auto orderbook_mutex_tuple = std::make_tuple(std::get<0>(instrumentMap.at(ticker)), std::get<2>(instrumentMap.at(ticker)));
						orders.emplace(input.order_id, orderbook_mutex_tuple);
						//orders.emplace(input.order_id, std::tuple<std::shared_ptr<Orderbook>, std::shared_mutex>(std::get<0>(instrumentMap.at(ticker)), std::get<2>(instrumentMap.at(ticker))));
					}
					else if (input.type == input_sell) {
						//HASHMAP INSERTED
						std::lock_guard<std::mutex> lk(instrumentMut);
						orders.emplace(input.order_id, std::tuple<std::shared_ptr<Orderbook>, std::shared_mutex>(std::get<0>(instrumentMap.at(ticker)), std::get<2>(instrumentMap.at(ticker))));
					}
				}
				break;
			}
		}

		// Additionally:

		// Remember to take timestamp at the appropriate time, or compute
		// an appropriate timestamp!
	}
}

bool Engine::handleOrder(std::string ticker, CommandType cmd, int price, int count, int id, long long timestamp) {
  // Retrieve otherBook param for findMatch
  {
	std::lock_guard<std::mutex> lk(instrumentMut);
	if (!instrumentMap.contains(ticker)){
			//HASHMAP INSERTED
			std::mutex mtx;
			instrumentMap.emplace(ticker, std::make_tuple(std::make_shared<Orderbook>(), std::make_shared<Orderbook>(), std::shared_mutex{}));
	}
  }
  std::lock_guard<std::mutex> lk(instrumentMut);
  //std::mutex bookMut std::get<2>(instrumentMap.at(ticker));
  std::shared_lock<std::shared_mutex> bookLock(std::get<2>(instrumentMap.at(ticker)));
  //Orderbook* thisBook;
  std::shared_ptr<Orderbook> otherBook;
  // Active orders are
  switch (cmd) {
  case input_buy: {
	
    		otherBook = std::get<1>(instrumentMap.at(ticker));
		//thisBook = std::get<0>(instrumentMap.at(ticker));
	
		updateBuyBook(ticker, price, count, id, timestamp);
    break;
  }
  case input_sell: {
	
    		otherBook = std::get<0>(instrumentMap.at(ticker));
		//thisBook = std::get<1>(instrumentMap.at(ticker));
	
		updateSellBook(ticker, price, count, id, timestamp);
    break;
  }
  default: {
	break;
  }
  // End switch
  }

  // Find a match such that shares are left
  while (count > 0) {
	int prevCount = count;
  	count = otherBook->findMatch(cmd, price, count, id, timestamp);
    if (count == prevCount) {
		break;
	}
  }
  // If count is 0, order is handled
  if (count == 0) {
	// Switch to remove active order if handled
	switch (cmd) {
	case input_buy: {
		std::get<0>(instrumentMap.at(ticker))->removeById(id);
	}
	case input_sell: {
		std::get<1>(instrumentMap.at(ticker))->removeById(id);
	}
	default: {}
	}
	// Return that active order was handled
    return true;
  }
  // Otherwise, update buy book if count is non-zero
  if (cmd == input_buy) {
    // updateBuyBook(ticker, price, count, id);
	  std::get<0>(instrumentMap.at(ticker))->decrementCountById(id, count);
	Output::OrderAdded((uint32_t)id, ticker.c_str(), (uint32_t)price, (uint32_t)count, cmd == input_sell, timestamp);
  // Update sell book if command is sell
  } else {
    // updateSellBook(ticker, price, count, id);
	std::get<1>(instrumentMap.at(ticker))->decrementCountById(id, count);
	Output::OrderAdded((uint32_t)id, ticker.c_str(), (uint32_t)price, (uint32_t)count, cmd == input_sell, timestamp);
  }
  return false;
}
