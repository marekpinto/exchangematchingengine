#include <iostream>
#include <thread>
#include <string>
#include <mutex>


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

void Engine::updateBuyBook(std::string ticker, int price, int count, int id)
{
	Orderbook* book;
	{
		std::lock_guard<std::mutex> lk(instrumentMut);
		book = get<0>(instrumentMap.at(ticker));
	}
	book->add(price, count, id);	
}

void Engine::updateSellBook(std::string ticker, int price, int count, int id)
{
	Orderbook * book;
	{
		std::lock_guard<std::mutex> lk(instrumentMut);
		book = get<1>(instrumentMap.at(ticker));
	}
	book->add(price, count, id);
}




void Engine::connection_thread(ClientConnection connection)
{
	std::unordered_map<int, Orderbook *> orders;
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

		// Functions for printing output actions in the prescribed format are
		// provided in the Output class:
		switch(input.type)
		{

			case input_cancel: {
				SyncCerr {} << "Got cancel: ID: " << input.order_id << std::endl;

				// Remember to take timestamp at the appropriate time, or compute
				// an appropriate timestamp!
				auto output_time = getCurrentTimestamp();
				if (orders.contains(input.order_id)) {
					bool result = orders.at(input.order_id) -> removeById(input.order_id);
					if (result) {
						Output::OrderDeleted(input.order_id, true, output_time);
						orders.erase(input.order_id);
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
				bool result = Engine::handleOrder(ticker, input.type, input.price, input.count, input.order_id);
				if (!result){
					if (input.type == input_buy) {
						//HASHMAP INSERTED
						std::lock_guard<std::mutex> lk(instrumentMut);
						orders.emplace(input.order_id, get<0>(instrumentMap.at(ticker)));
					}
					else if (input.type == input_sell) {
						//HASHMAP INSERTED
						std::lock_guard<std::mutex> lk(instrumentMut);
						orders.emplace(input.order_id, get<1>(instrumentMap.at(ticker)));
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

bool Engine::handleOrder(std::string ticker, CommandType cmd, int price, int count, int id) {
  // Retrieve otherBook param for findMatch
  {
	std::lock_guard<std::mutex> lk(instrumentMut);
	if (!instrumentMap.contains(ticker)){
			//HASHMAP INSERTED
			instrumentMap.emplace(ticker, std::make_tuple(new Orderbook(), new Orderbook()));
	}
  }
  Orderbook* otherBook;
  switch (cmd) {
  case input_buy: {
	std::lock_guard<std::mutex> lk(instrumentMut);
    otherBook = get<1>(instrumentMap.at(ticker));
    break;
  }
  case input_sell: {
	std::lock_guard<std::mutex> lk(instrumentMut);
    otherBook = get<0>(instrumentMap.at(ticker));
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
  	count = otherBook->findMatch(cmd, price, count, id);
    if (count == prevCount) {
		break;
	}
  }
  // If count is 0, order is handled
  if (count == 0) {
    return true;
  }
  // Otherwise, update buy book if count is non-zero
  if (cmd == input_buy) {
    updateBuyBook(ticker, price, count, id);
	Output::OrderAdded(id, ticker.c_str(), price, count, cmd == input_sell, getCurrentTimestamp());
  // Update sell book if command is sell
  } else {
    updateSellBook(ticker, price, count, id);
	Output::OrderAdded(id, ticker.c_str(), price, count, cmd == input_sell, getCurrentTimestamp());
  }
  return false;
}
