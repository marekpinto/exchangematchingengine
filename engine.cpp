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
	        //return std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
		        std::unique_lock<std::mutex> timelk(timestampMut);
		                timestamp +=1;
					return timestamp;
		                }
		//

void Engine::accept(ClientConnection connection)
{
	auto thread = std::thread(&Engine::connection_thread, this, std::move(connection));
	thread.detach();
}

void Engine::updateBuyBook(std::string ticker, int price, int count, int id)
{
	
	//SyncCerr {} << "Updating Buy book" << std::endl;
	//std::lock_guard<std::mutex> lk(instrumentMut);
	//std::mutex bookMut  std::get<2>(instrumentMap.at(ticker));
	//std::lock_guard<std::mutex> bookLock(bookMut);
	//std::lock_guard<std::mutex> lk(bookMut);
	//std::shared_ptr<std::mutex> bookMut;
	std::shared_ptr<Orderbook> book;
	{
	std::unique_lock<std::mutex> lk(instrumentMut);
	
	//bookMut = std::get<2>(instrumentMap.at(ticker));
	book = std::get<0>(instrumentMap.at(ticker));
	}

	//std::unique_lock<std::mutex> bookLock(*bookMut);
	
	book->add(price, count, id);	
	
	}

void Engine::updateSellBook(std::string ticker, int price, int count, int id)
{
	//std::lock_guard<std::mutex> lk(instrumentMut);
	//std::mutex bookMut std::get<2>(instrumentMap.at(ticker));
	//std::lock_guard<std::mutex> bookLock(bookMut);
	//std::shared_ptr<std::mutex> bookMut;
	std::shared_ptr<Orderbook> book;
	{
	std::unique_lock<std::mutex> lk(instrumentMut);
	//bookMut = std::get<2>(instrumentMap.at(ticker));
	book = std::get<1>(instrumentMap.at(ticker));
	}

	//std::unique_lock<std::mutex> bookLock(*bookMut);
	
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
		//long time = (long)getCurrentTimestamp();
		// Functions for printing output actions in the prescribed format are
		// provided in the Output class:
		switch(input.type)
		{

			case input_cancel: {
				SyncCerr {} << "Got cancel: ID: " << input.order_id << std::endl;

				// Remember to take timestamp at the appropriate time, or compute
				// an appropriate timestamp!
				//auto output_time = getCurrentTimestamp();
				//potential error if orders hasn't been added yet - shouldn't be an issue tho since client commands go in order
				if (orders.contains((int)input.order_id)) {
					auto ordersPair = orders.at((int)input.order_id);
					//std::shared_lock<std::shared_mutex> lk(instrumentMut);
					//auto correspondingTuple = instrumentMap.at(ordersPair.first);
					//int type = ordersPair.second;
					//std::mutex bookMut = std::get<1>(orders.at((int)input.order_id));
					//std::shared_ptr<std::mutex> bookMut;
					//std::shared_ptr<Orderbook> book;
					
					//{
					//	std::mutex cancelMut;
					//	std::unique_lock<std::mutex> waitLock(cancelMut);
					//	cond.wait(waitLock);
					//}

				//	{
				//	std::unique_lock<std::mutex> lk(instrumentMut);
				//	if (!instrumentMap.contains(ticker)) {
				//		SyncCerr {} << "Cancel issue" << std::endl;
				//	}
				//	bookMut = std::get<2>(instrumentMap.at(ticker));
				//	if (type == 0) {
				//		book = std::get<0>(instrumentMap.at(ordersPair.first));
				//	} else if (type == 1) {
				//		book = std::get<1>(instrumentMap.at(ordersPair.first));
				//	}
				//	}
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

				// Remember to take timestamp at the appropriate time, or compute
				// an appropriate timestamp!
				// auto output_time = getCurrentTimestamp();
				bool result = Engine::handleOrder(ticker, input.type, (int)input.price, (int)input.count, (int)input.order_id);
				
				if (!result){
					if (input.type == input_buy) {
						//HASHMAP INSERTED
						std::unique_lock<std::mutex> lk(instrumentMut);
						auto orderbook_mutex_tuple = std::make_pair(std::get<0>(instrumentMap.at(ticker)),std::get<2>(instrumentMap.at(ticker)));
						orders.emplace(input.order_id, orderbook_mutex_tuple);
						//orders.emplace(input.order_id, std::tuple<std::shared_ptr<Orderbook>, std::shared_mutex>(std::get<0>(instrumentMap.at(ticker)), std::get<2>(instrumentMap.at(ticker))));
					}
					else if (input.type == input_sell) {
						//HASHMAP INSERTED
						std::unique_lock<std::mutex> lk(instrumentMut);
						auto orderbook_mutex_tuple = std::make_pair(std::get<1>(instrumentMap.at(ticker)), std::get<3>(instrumentMap.at(ticker)));
						orders.emplace(input.order_id, orderbook_mutex_tuple);
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
	std::unique_lock<std::mutex> lk(instrumentMut);
	if (!instrumentMap.contains(ticker)){
			//HASHMAP INSERTED
			//std::mutex mtx;
			//potential for race condition on making Orderbook???
			//std::unique_lock<std::mutex> bookLock(*bookMut);	
			instrumentMap.emplace(ticker, std::make_tuple(std::make_shared<Orderbook>(), std::make_shared<Orderbook>(), std::make_shared<std::mutex>(), std::make_shared<std::mutex>()));
	}
	}
 // std::unique_lock<std::mutex> lk(instrumentMut);
  //std::mutex bookMut std::get<2>(instrumentMap.at(ticker));
  //std::unique_lock<std::mutex> bookLock(*std::get<2>(instrumentMap.at(ticker)));
  //Orderbook* thisBook;
	std::shared_ptr<Orderbook> thisBook;
  std::shared_ptr<Orderbook> otherBook;
  std::shared_ptr<std::mutex> bookMut;
  std::shared_ptr<std::mutex> otherMut;
  // Active orders are
  switch (cmd) {
  case input_buy: {
		{
		 std::unique_lock<std::mutex> lk(instrumentMut);
		 thisBook = std::get<0>(instrumentMap.at(ticker));
		 otherBook = std::get<1>(instrumentMap.at(ticker));
		 bookMut = std::get<2>(instrumentMap.at(ticker));
		 otherMut = std::get<3>(instrumentMap.at(ticker));
		//thisBook = std::get<0>(instrumentMap.at(ticker));
		}
		//updateBuyBook(ticker, price, count, id);
    break;
  }
  case input_sell: {
			   {
			   std::unique_lock<std::mutex> lk(instrumentMut);
		  thisBook = std::get<1>(instrumentMap.at(ticker));
    			otherBook = std::get<0>(instrumentMap.at(ticker));
			bookMut = std::get<3>(instrumentMap.at(ticker));
			otherMut = std::get<2>(instrumentMap.at(ticker));
		//thisBook = std::get<1>(instrumentMap.at(ticker));
			   }
		//updateSellBook(ticker, price, count, id);
		break;
  }
  default: {
	break;
  }
  // End switch
  }
  {
	std::unique_lock<std::mutex> otherLock(*otherMut);
  // Find a match such that shares are left
  while (count > 0) {
	int prevCount = count;
	{
	//std::unique_lock<std::mutex> bookLock(*bookMut);
  	count = otherBook->findMatch(cmd, price, count, id, getCurrentTimestamp());
	}
	if (count == prevCount) {
		break;
	}
  }
  }
  
	  std::unique_lock<std::mutex> bookLock(*bookMut);
  // If count is 0, order is handled
  if (count == 0) {
	// Switch to remove active order if handled
	  {
		  //std::unique_lock<std::mutex> bookLock(*bookMut);
		  thisBook->removeById(id);
	  }
	
	// Return that active order was handled
    return true;
  }
  // Otherwise, update buy book if count is non-zero
	  {
		  //std::unique_lock<std::mutex> bookLock(*bookMut);
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
