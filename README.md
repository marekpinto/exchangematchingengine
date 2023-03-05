# Exchange Matching Engine

This C++ program uses techniques of concurrent programming to efficiently match buy and sell orders for a stock exchange. As active orders are placed, they will 
be compared to all resting orders to find a potential match. If there is no match, the active order is placed into a resting order book and can be matched at any point
in the future. The engine matches orders using the price-time priority rule. This rule for matching two orders on an exchange is expressed using the following conditions – which must all be true for the matching to happen:

- The side of the two orders must be different (i.e. a buy order must match against a sell orders or vice versa).
- The instrument of the two orders must be the same (i.e. an order for “GOOG” must match against another order for “GOOG”).
- The size of the two orders must be greater than zero.
- The price of the buy order must be greater or equal to the price of the sell order.
- In case multiple orders can be matched, the order with the best price is matched first. For sell orders, the best price is the lowest; for buy orders, the best price is the highest.
- If there are still multiple matchable orders, the order that was added to the order book the earliest (ie. the order whose “added to order book” log has the earliest timestamp) will be matched first. 

# How to Use

Clone the repository and run "make", which will compile the latest code into an engine executable and client executable. You can start the engine using `./engine <path_to_socket>`.
This will start a server and listen on the socket at `<path_to_socket>`. To start a client, run `./client <path_to_socket>`. You can start as many clients as you'd like, and each can send
orders in parallel that will be processed concurrently.

## Inputs

There are three input commands that can be sent by the client:

#### New Buy Order
Arguments: Order ID, Instrument, Price, Count
Example: `B 123 GOOG 2700 10`

#### New Sell Order
Arguments: Order ID, Instrument, Price, Count
Example: `S 124 GOOG 1800 8`

#### Cancel Order
Arguments: Order ID
Note: Cancels may only be sent by the same client that placed the order.
Example: `C 123`

## Outputs

These outputs will be sent to stdout by the engine executable.

#### Order Successfully Added
Format: `<B/S> <Order ID> <Instrument> <Price> <Count> <Timestamp completed>`
Example: `B 123 GOOG 2700 10 1`
Example: `S 124 GOOG 1800 8 2`

#### Order Executed
Format: `E <Resting order ID> <New order ID> <Execution ID> <Price> <Count> <Timestamp completed>`
Example: `E 123 124 1 2700 8 3`

#### Order Deleted
Format: `X <Order ID> <A/R> <Timestamp completed>`
A represents an accepted cancel, and R represents a rejected cancel.
Example: `X 123 A 4`

# Explanation of Concurrency

We enable the concurrent execution of orders from multiple clients by compartmentalizing data based on instrument and status as a buy or sell order. Since *instrumentMap* maps each instrument to its own tuple of *Orderbooks*, threads can concurrently access and execute orders with different instruments. 

We used mutexes to isolate reads and writes to the *Orderbooks* and to the *instrumentMap* under certain circumstances. We decided that all synchronization would be done from the *Engine* class for simplicity. This is possible because of the splitting of *Orderbooks* by instrument and type, so that each *Orderbook* only needs to be accessed by one thread at a time. We use a *unique_lock* within *engine.cpp* to limit access to *instrumentMap*. However, *instrumentMap* is only modified when a new instrument is encountered, and only accessed to update orders, access a specific *Orderbook*, or obtaining the pointer to the *Orderbook* mutex. These actions only happen around once per input cycle, and we use limited scopes to ensure that the *instrumentMutex* does not significantly reduce concurrency. The process of finding a match for a given buy or sell order is done through the *handleOrder* function, which sets a *unique_lock* on the mutex which is stored in the *instrumentMap* function and corresponds to that mutex. Therefore, orders on different instruments can happen concurrently, but orders on the same instrument are serialized. 

Our engine achieves Instrument-Level Concurrency. Orders for different instruments can execute concurrently because the pertinent data is stored in separate tuples accessed via hash map. 

## Testing

We began by testing basic functionality against simple one-thread test cases, which highlighted several pointer and logical errors that were patched. We passed the basic cases and then moved on to manual testing with multiple threads. In an environment with 4 threads, our engine was able to perform cross-thread full and partial matching. The engine also maintained correct ordering of matching based on pricing and timestamp. The engine also only allowed cancellations for orders produced within the same thread.

We then moved on to creating complex test cases using the Python script generate_test_cases.py. We generated test files to mimic complex testing cases. We also created two more categories of tests: medium (up to 4 clients) and mediumHard (up to 20 clients). Below are parameters for our complex test cases.

- Random stock instrument chosen from a group of length 428
- 40 clients (and therefore 40 concurrent threads)
- Random number of orders in range [1000, 50000]
- Random order type, both buy, sell, and cancel with a probability of ⅓ each
- Random assignment of client with an equal probability for all clients
- Random price in range [100, 2000]
- Random count between [10, 1000]

We debugged concurrency issues using ThreadSanitizer, as well as printing to std:cerr to identify the order in which different parts of our program were executed. We also created our own timestamp system (with a counter starting at 0 and incrementing whenever a timestamp was printed), so that we could more easily track the execution order and fix issues with orders being added to the book out of order.

We then rewrote our code to its new form, putting all synchronization within the engine class. We also used a single mutex for both buy and sell orderbooks to avoid the issue of matching buys and sells that came in at the same time. By imposing these concurrency restraints, we were able to achieve 100% accuracy across all generated concurrent test cases (medium, mediumHard, and complex).
