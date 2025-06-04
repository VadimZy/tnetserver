# TCP Hashed Echo Server
A simple TCP ipv4 server that listens on a configured port (default: 2323) and responses with a hash calculated for each input line.

## Features
- Multithreaded read: each connected client is served with a read task that that runs in a software thread.
- Streaming hash calculation: the reader incrementaly updateds hash with the bytes read from the socket, thus it does not need to receive full line to calculate the hash

## Building
> Prerequisites
- cmake 3.24+
- gnu make
- gcc/g++ capable of C++ 20
- openssv 3.0+ (sudo apt-get install libssl-dev)

> Dependencies

The build brings:
- C++ Poco library 1.14.2, used for
   - Hash calculations
   - Configuration
   - Concurrent Task pool

- googletest 1.14.0 
  - unit testing
