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
    std::unique_lock<std::mutex> lk(instrumentMut);	
	orderBookHash instrumentMap;
	std::unique_lock<std::mutex> timelk(timestampMut);
	timestamp = 0;
}


int Engine::getCurrentTimestamp() 
{
	std::unique_lock<std::mutex> timelk(timestampMut);
	timestamp +=1;
	return timestamp;
}

void Engine::accept(ClientConnection connection)
{
	auto thread = std::thread(&Engine::connection_thread, this, std::move(connection));
	thread.detach();
}

void Engine::updateBuyBook(std::string ticker, int price, int count, int id)
{

	std::shared_ptr<Orderbook> book;
	{
		std::unique_lock<std::mutex> lk(instrumentMut);
		book = std::get<0>(instrumentMap.at(ticker));
	}	
	book->add(price, count, id);	
}

void Engine::updateSellBook(std::string ticker, int price, int count, int id)
{
	std::shared_ptr<Orderbook> book;
	{
		std::unique_lock<std::mutex> lk(instrumentMut);
		book = std::get<1>(instrumentMap.at(ticker));
	}	
	book->add(price, count, id);	
}




void Engine::connection_thread(ClientConnection connection)
{
	std::unordered_map<int, std::pair<std::shared_ptr<Orderbook>, std::shared_ptr<std::mutex>>> orders;
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
		switch(input.type)
		{
			case input_cancel: {
				SyncCerr {} << "Got cancel: ID: " << input.order_id << std::endl;
				if (orders.contains((int)input.order_id)) {
					auto ordersPair = orders.at((int)input.order_id);

					std::shared_ptr<Orderbook> book = ordersPair.first;
					bool result;
					{
						std::unique_lock<std::mutex> bookLock(*ordersPair.second);
						result = book->removeById((int)input.order_id);
					}

					if (result) {
						Output::OrderDeleted(input.order_id, true, getCurrentTimestamp());
						orders.erase((int)input.order_id);
						break;
					}
				}
				Output::OrderDeleted(input.order_id, false, getCurrentTimestamp()); 
				break;
			}

			default: {
				SyncCerr {}
				    << "Got order: " << static_cast<char>(input.type) << " " << input.instrument << " x " << input.count << " @ "
				    << input.price << " ID: " << input.order_id << std::endl;

				bool result = Engine::handleOrder(ticker, input.type, (int)input.price, (int)input.count, (int)input.order_id);
				
				if (!result){
					if (input.type == input_buy) {
						//HASHMAP INSERTED
						std::unique_lock<std::mutex> lk(instrumentMut);
						auto orderbook_mutex_tuple = std::make_pair(std::get<0>(instrumentMap.at(ticker)),std::get<2>(instrumentMap.at(ticker)));
						orders.emplace(input.order_id, orderbook_mutex_tuple);
					}
					else if (input.type == input_sell) {
						//HASHMAP INSERTED
						std::unique_lock<std::mutex> lk(instrumentMut);
						auto orderbook_mutex_tuple = std::make_pair(std::get<1>(instrumentMap.at(ticker)), std::get<2>(instrumentMap.at(ticker)));
						orders.emplace(input.order_id, orderbook_mutex_tuple);
					}
				}
				break;
			}
		}
	}
}

bool Engine::handleOrder(std::string ticker, CommandType cmd, int price, int count, int id) {
	// Retrieve otherBook param for findMatch
	{ 
	std::unique_lock<std::mutex> lk(instrumentMut);
	if (!instrumentMap.contains(ticker)){
		std::shared_ptr<std::mutex> bookMut = std::make_shared<std::mutex>();
		instrumentMap.emplace(ticker, std::make_tuple(std::make_shared<Orderbook>(), std::make_shared<Orderbook>(), bookMut));
	}
	// End of external scope
	}
	std::shared_ptr<Orderbook> thisBook;
	std::shared_ptr<Orderbook> otherBook;
	std::shared_ptr<std::mutex> bookMut;
	// Active orders are
	switch (cmd) {
  	case input_buy: {
		{
		 std::unique_lock<std::mutex> lk(instrumentMut);
		 thisBook = std::get<0>(instrumentMap.at(ticker));
		 otherBook = std::get<1>(instrumentMap.at(ticker));
		 bookMut = std::get<2>(instrumentMap.at(ticker));
		}
    break;
  }
  case input_sell: {
	{
		std::unique_lock<std::mutex> lk(instrumentMut);
		thisBook = std::get<1>(instrumentMap.at(ticker));
    	otherBook = std::get<0>(instrumentMap.at(ticker));
		bookMut = std::get<2>(instrumentMap.at(ticker));
	}
		break;
  }
  default: {
	break;
  }
  // End switch
  }
	std::unique_lock<std::mutex> bookLock(*bookMut);
  // Find a match such that shares are left
  while (count > 0) {
	int prevCount = count;
	{
  		count = otherBook->findMatch(cmd, price, count, id, getCurrentTimestamp());
	}
	if (count == prevCount) {
		break;
	}
  }
  // If count is 0, order is handled
  if (count == 0) {
	// Switch to remove active order if handled
	  {
		thisBook->removeById(id);
	  }
	
	// Return that active order was handled
    return true;
  }
  // Otherwise, update buy book if count is non-zero
	{
		thisBook->decrementCountById(id, count);
	}

	switch(cmd) {
		case input_buy: {
			updateBuyBook(ticker, price, count, id);
			break;
		}
		case input_sell: {
			updateSellBook(ticker, price, count, id);
			break;
		}
		default: {
			break;
		}
	}
	Output::OrderAdded((uint32_t)id, ticker.c_str(), (uint32_t)price, (uint32_t)count, cmd == input_sell, getCurrentTimestamp());
  
  return false;
}