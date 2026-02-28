# Limit-order-book

Notes
- TCP server: Transmission Control Protocol. A connection oriented protocol between machines, with reliable ordered byte stream between them. Nothing gets lost, in the order that it was sent
    - create a socket (a file descriptor representing a network endpoint), bind it to a port, listen for incoming connections and accept them one at a time. 

Design decisions:
- store prices as ints representing cents. For example $100.23 is stored as `10023` - allows for flat array optimization
- `OrderMessage` struct - this is essentially how exchange protocols like NASDAQ ITCH work, just with more message types.

Improvements for later:
- implement a red-black tree, sorted flat array, or a skip list for price levels, replacing `std::map`