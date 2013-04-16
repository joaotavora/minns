minns - minimal name server
==========================

Compiling and running
---------------------

To compile, unpack the tarball and call make:

    $ tar xvf minns.tar.gz
    $ make

This makes all the binaries (main executable and some unit tests) in
`./bin`. You should run **minns** for the first time by typing

    $ bin/minns -h

, which gives you a list of command line options. You'll probably want to try
minns on non-reserved UDP and TCP ports using the `-t` and `-u` options.

Features
--------

The main features of **minns** are:

* pre-threaded DNS server designed with robustness, simplicity, efficiency and
  capacity in mind.

* support of `QUERY_A` type DNS queries.

* reads entries of a file similar to `/etc/hosts`. caches frequently accessed
   entries for faster response.

* supports queries in both UDP and TCP transport

* uses no external libraries apart from libc and C++ stdl

* portable at least to MacOSX 10.5 Leopard (Darwin/BSD)

Limitations
----------

There are limitations:

* only supports IPV4 queries

* almost POSIX compliant, but not quite (`SO_RECVTIMEO` is not standard)

* the truncation (TC) bit in DNS UDP replies is not implemented. The server
   responds with a "Server failure" condition instead. However, a client can try
   a TCP request and will probably get a nicer answer.

* the server cannot run as a daemon yet.

Architecture
============

Overview
--------

**minns** can be seen as composed of three parts:

* the pre-threaded DNS Server, that depends on the other two parts, and is
   composed of base classes `DnsServer`, `DnsWorker` and `DnsMessage`.

* an independent `DnsResolver` base class, responsible for dealing with the
   hosts file and managing the cache, independent.

* three small libraries (`TcpSocket`, `UdpSocket`, `Thread`), independent
  between themselves, that wrap useful system calls and provide error handling
  through exceptions.

Pre-threaded DNS server
-----------------------

This is implemented in the files `DnsServer.cpp`, `DnsWorker.cpp` and
`DnsMessage.cpp`.

### DnsServer.cpp

A single instance of class `DnsServer` spawns a number of `DnsWorker`
threads. `DnsWorker` is an abstract base class to both the `UdpWorker` and
`TcpWorker` concrete classes, which handle UDP and TCP requests,
respectively. It owns the following objects, initialized in the constructor.

1. A single `UdpSocket` object for datagram communication, shared
   by all `UdpWorker`'s.

2. A single `TcpSocket` object (server socket) shared by all
   `TcpWorker`s's. This socket is put in the LISTEN state.

3. A variable number of `UdpWorker`s and `TcpWorker`s. On
   construction, these are given an instance of the DnsResolver
   class.

4. One mutex to protect access to the `DnsResolver` class and another to protect
   access to the server socket.

The entry point `DnsServer::start()` proceeds as follows:

1. Install signal handlers for the `SIGTERM` and `SIGINT` signals;

2. Creates worker threads using the `Thread(Runnable&)` constructor (`DnsWorker`
   objects are `Thread::Runnable` instances);

3. Runs all worker threads;

4. Waits on an exit semaphore, which is signalled when `SIGTERM` or `SIGINT` signals
   are received by the process;

5. Signals all workers to stop;

6. Closes both UDP and TCP sockets;

7. Signals possibly blocked threads with the `SIGALRM` signal
   (this causes any system calls to be interrupted);

8. Waits for all threads to finish (calling `pthread_join()`);

9. Reports on worker status on exit;


### DnsWorker.cpp

Each instance of the `DnsWorker` class runs in its own thread. This
abstract class tries to abstract as much similarity as possible
between the UDP and TCP cases.

The main entry point is `DnsWorker::work()`. It uses the following pure virtual
functions:

* `virtual void setup() = 0;`

  This method is called before trying to receive any queries
  and after each teardown() operation.

  `TcpWorker`'s implementation of this method calls `accept()`,
  after locking a mutex, thereby producing another `TcpSocket`
  object used to connect to the client.

  `UdpWorker`'s implementation does nothing.

* `virtual void teardown() = 0;`

  This method is called when an exceptional event happens after setup in one of
  the sockets.

  `TcpWorker`'s implementation of this method closes the client socket and
  deletes the `TcpSocket` object.

  `UdpWorker`'s implementation does nothing.

* `virtual size_t readQuery(char * buff, char * bufflen) = 0;`

  This method is called after setup() and produce a buffer of
  data for constructing a DnsMessage object.

  `UdpWorker`'s implementation of this method simply reads a
  datagram from the `UdpSocket` object.

  `TcpWorker`'s implementation reads 2 bytes containing the
  length of the message and then reads the rest of the
  message.

* `virtual size_t sendResponse(char* buff, char *bufflen) = 0;`

  This method is called when `DnsWorker` needs to send a
  serialized response (even if an error);

  Again, `TcpWorker`'s implementation of this method only
  differs from `UdpWorker`'s in the fact that two extra bytes
  are sent representing the message length.

### DnsMessage.cpp

An instance of `DnsResponse` (subclass of `DnsMessage` is built using a
`DnsResolver` object. This object is used to resolve the actual name being
queried, but only after locking a mutex.

Apart from that, these classes deal mostly with parsing and serialization of
messages. They could have been implemented using bit-fields which would be more
efficient but maybe less portable, so I haven't attempted it yet.

Exceptions are also thrown from some of the operations of these classes. They
are handled by the `DnsWorker` class.

DnsResolver.cpp and the cache
-----------------------------

The DnsResolver class is responsible for reading the hosts file and is always
delegated the resolution of a domain name. A size-limited cache is used to speed
up resolutions.

The method `addr_set_t* resolve (const string& name)` contains the main
algorithm and proceeds as follows:

1. If the file has been modified since the last time it was read, clear the
   cache. (this can be turned off with the `-n` option for efficiency)

2. Look-up the entry in the cache, if it is there return the set of
   corresponding addresses is returned.

3. Otherwise, start searching the file from the beginning. Parse a
   complete line. For each name entry in a valid line do:

   3.1 if the name matches the search insert the <name, ip>
       mapping it into the cache.

   3.2 else, if the cache is not full, insert the into the cache
       anyway,

   3.3 else don't do anything.

4. If a result has been fpound, return it, else go back to 3. and
   parse another line.

The cache itself is implemented by class `DnsResolver::Cache` and is composed of
the following data structures:

  a) A map (`std::map`), mapping names to sets of addresses;

  b) A list (`std::list`) instance, keeping track of the most recently
     searches;

and the following operations:

* `addr_set_t* lookup(const string& name);`

  Returns a matching set of ip addresses if these are found in
  a) or NULL otherwise.

  If a match is found in map a), the entry found also contains
  a hidden pointer to the position of the match in the list b).
  That entry in the list is moved up to the beginning of the
  list, and the pointer in map a)'s matching entry is updated.

  Implementation-wise, iterators are used instead of raw
  pointers.

* `addr_set_t* insert(const string& name, struct in_addr ip);`

   Inserts the `<name, ip>` mapping into the cache.

   If an entry for `name` is already ins map a) , add `ip` the
   set of IPs associated with `name`.

   Otherwise, create a new entry in map a). Also, insert into
   the beginning of the list b) a a pointer to this
   entry. Finally, make newly create entry point to the list as
   well.

   If after these steps the cache has exceeded its maximum size,
   remove the last element from the list b) and the
   corresponding map entry from map a).

TcpSocket, UdpSocket and Thread libraries
----------------------------------------------

These small libraries wrap POSIX system calls in nice C++ classes. They are not
intended to be fully fledged API implementations, other external libraries
already do that. But these do the job nicely. A handful of (quickly written and
possibly buggy) unit tests is available in `./test`.

Thanks
======

To the late W. Richard Stevens and the greatest book on Unix
Networking ever. This is the bible.yy

To Bruce Eckel and his fantastic (although sometimes annoying) way
of explaining C++ in the book "Thinking in C++"

To google, or better, to the people who kindly answer small
questions that are incredibly valuable.

To my girlfriend, for having let me work over the weekend :-)
