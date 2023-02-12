#include <iostream>
#include <thread>
#include <string>


#include "io.hpp"
#include "engine.hpp"

#include "orderbook.hpp"

//makes a thread every time a connection comes in

void Engine::accept(ClientConnection connection)
{
	auto thread = std::thread(&Engine::connection_thread, this, std::move(connection));
	thread.detach();
}

void Engine::updateBuyBook(std::string ticker, uint32_t price, uint32_t count)
{
	Orderbook book = get<0>(instrumentMap.at(ticker));
	book.add(price, count);
}

void Engine::updateSellBook(std::string ticker, uint32_t price, uint32_t count)
{
	Orderbook book = get<1>(instrumentMap.at(ticker));
	book.add(price, count);
}

Engine::Engine()
{  
	orderBookHash instrumentMap;
}

orderBookHash Engine::getOrderBookMap()
{
	return instrumentMap;
}

Orderbook Engine::createBook()
{
	Orderbook book;
	return book;
}


void Engine::connection_thread(ClientConnection connection)
{
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
			case input_buy: {
				Engine::updateBuyBook(ticker, input.price, input.count);
				break;
			}

			case input_sell: {
				Engine::updateSellBook(ticker, input.price, input.count);
				break;
			}


			case input_cancel: {
				SyncCerr {} << "Got cancel: ID: " << input.order_id << std::endl;

				// Remember to take timestamp at the appropriate time, or compute
				// an appropriate timestamp!
				auto output_time = getCurrentTimestamp();
				Output::OrderDeleted(input.order_id, true, output_time);
				break;
			}

			default: {
				SyncCerr {}
				    << "Got order: " << static_cast<char>(input.type) << " " << input.instrument << " x " << input.count << " @ "
				    << input.price << " ID: " << input.order_id << std::endl;

				// Remember to take timestamp at the appropriate time, or compute
				// an appropriate timestamp!
				auto output_time = getCurrentTimestamp();
				Output::OrderAdded(input.order_id, input.instrument, input.price, input.count, input.type == input_sell,
				    output_time);
				break;
			}
		}

		// Additionally:

		// Remember to take timestamp at the appropriate time, or compute
		// an appropriate timestamp!
		intmax_t output_time = getCurrentTimestamp();

		// Check the parameter names in `io.hpp`.
		Output::OrderExecuted(123, 124, 1, 2000, 10, output_time);
	}
}

bool Engine::handleOrder(std::string ticker, CommandType cmd, uint32_t price, uint32_t count) {
  // orderBookHash orderMap = f.getOrderBookMap();
  // Retrieve otherBook param for findMatch
  Orderbook otherBook;
  switch (cmd) {
  case input_buy: {
    otherBook = get<1>(instrumentMap.at(ticker));
    break;
  }
  case input_sell: {
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
    count = findMatch(cmd, otherBook, price, count);
  }
  // If count is 0, order is handled
  if (count == 0) {
    return true;
  }
  // Otherwise, update buy book if count is non-zero
  if (cmd == input_buy) {
    updateBuyBook(ticker, price, count);
  // Update sell book if command is sell
  } else {
    updateSellBook(ticker, price, count);
  }
  return false;
}
