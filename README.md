# TCP Hashed Echo Server
A simple TCP ipv4 server that listens on a configured port (default: 2323) and responses with a hash calculated for each input line.

## Features
- Multithreaded read: each connected client is served with a read task that that runs in a software thread.
- Streaming hash calculation: the reader incrementaly updateds hash with the bytes read from the socket, thus it does not need to receive full line to calculate the hash

### brief class diagramm

![](./src/tserver_classes.svg)


## Building
> Prerequisites
- cmake 3.24+
- gnu make
- gcc/g++ capable of C++ 20
- openssl 3.0+ (sudo apt-get install libssl-dev)

> Dependencies

build brings:
- C++ Poco library 1.14.2, used for
   - Hash calculations
   - Configuration
   - Concurrent Task pool

- googletest 1.14.0 
  - unit testing

> Running build

```bash
[tnetserver]$ mkdir build
[tnetserver]$ cd build/
[build]$ cmake ..
[build]$ make -j10
[build]$ cd bin
[bin]$ ls -1
tnetserver
ut-tserver
```
## Running
```bash
[bin]$ ./tnetserver 
2025-06-04T21:47:21.530, 140090644629376, INFO, Configuration.cpp:17: Configuration file not found, using default values
2025-06-04T21:47:21.530, 140090626012736, INFO, TcpServer.cpp:119: starting HashEcho server on port: 2323, maxThreads: 24, epollEventsNum: 1
```
open second terminal
```bash
[~]$ echo "qqqqq"| nc -N localhost 2323 
437599f1ea3514f8969f161a6606ce18

[~]$ nc -N localhost 2323 < ttest.txt 
d05c5287c7e9a25cc23c7054f8ead102
d05c5287c7e9a25cc23c7054f8ead102
5eca9bd3eb07c006cd43ae48dfde7fd3
5eca9bd3eb07c006cd43ae48dfde7fd3
75c2ef9fc70164b810829ea12c1c9a27
8c5bcf4c601179479ac6fa15bb0f1dec
d41d8cd98f00b204e9800998ecf8427e
d41d8cd98f00b204e9800998ecf8427e
[~]
```
<b>note: some nc versions does not support -N option, then it should be omitted</b> 

## Configuration

The server looks for `tserver.cfg` in the current directory

file sample:
```ini
[server]
port=2323
hash_type=md5
# max_threads=16 # default is hardware capacity
epoll_events_num=1
read_buffer_size=1024

[log]
level = debug
```
## Unit tests
```bash
[bin]$ ls -1
tnetserver
ut-tserver
[bin]$ ./ut-tserver
```
<b>alternative method:</b>
in terminal one start `tnetserver`
```sh
[bin]$ ./tnetserver
```
in terminal two start ut app `ut-tserver`
```bash
[bin]$ export USE_EXTERNAL_TNETSERVER=1
[bin]$ ./ut-tserver 
```
<b>second UT option with valgrind</b>
```bash
[]$ valgrind --tool=memcheck --leak-check=yes --show-reachable=yes --num-callers=20 --track-fds=yes ./bin/tnetserver
```

```text
==33038== 
==33038== FILE DESCRIPTORS: 3 open (3 inherited) at exit.
==33038== 
==33038== HEAP SUMMARY:
==33038==     in use at exit: 304 bytes in 1 blocks
==33038==   total heap usage: 50,734 allocs, 50,733 frees, 8,350,668 bytes allocated
==33038== 
==33038== 304 bytes in 1 blocks are possibly lost in loss record 1 of 1
==33038==    at 0x48650DC: calloc (vg_replace_malloc.c:1675)
==33038==    by 0x40297D9: calloc (rtld-malloc.h:44)
==33038==    by 0x40297D9: allocate_dtv (dl-tls.c:375)
==33038==    by 0x40297D9: _dl_allocate_tls (dl-tls.c:634)
==33038==    by 0x4EFF7B4: allocate_stack (allocatestack.c:430)
==33038==    by 0x4EFF7B4: pthread_create@@GLIBC_2.34 (pthread_create.c:647)
==33038==    by 0x4CF8328: std::thread::_M_start_thread(std::unique_ptr<std::thread::_State, std::default_delete<std::thread::_State> >, void (*)()) (in /usr/lib/x86_64-linux-gnu/libstdc++.so.6.0.30)
==33038==    by 0x4006F31: thread<main()::<lambda()> > (std_thread.h:143)
==33038==    by 0x4006F31: main (main.cpp:58)
==33038== 
==33038== LEAK SUMMARY:
==33038==    definitely lost: 0 bytes in 0 blocks
==33038==    indirectly lost: 0 bytes in 0 blocks
==33038==      possibly lost: 304 bytes in 1 blocks
==33038==    still reachable: 0 bytes in 0 blocks
==33038==         suppressed: 0 bytes in 0 blocks
==33038== 
==33038== For lists of detected and suppressed errors, rerun with: -s
==33038== ERROR SUMMARY: 1 errors from 1 contexts (suppressed: 0 from 0)

```
