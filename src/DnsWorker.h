#ifndef DNS_WORKER_H
#define DNS_WORKER_H

// libc includes

// stdl includes
#include <iostream>

// Project includes
#include "Thread.h"
#include "UdpSocket.h"
#include "TcpSocket.h"
#include "DnsResolver.h"
#include "DnsMessage.h"

class DnsWorker : public Thread::Runnable {
public:
    virtual ~DnsWorker() = 0;
    void rest();

    virtual std::string what() const = 0;
    // time_t getPunch();


private:
    void work();
    // punch();

protected:
    virtual void   setup() = 0;
    virtual void   teardown() = 0;
    virtual size_t readQuery(char* buff, size_t maxmessage) throw(Socket::SocketException)= 0;
    virtual size_t sendResponse(const char* buff, size_t maxmessage) throw(Socket::SocketException)= 0;
    void*   main ();

protected:
    DnsWorker(DnsResolver& _resolver, Thread::Mutex &_resolve_mutex, const size_t _maxmessage);
    int id;

private:
    bool stop_flag;
    DnsResolver& resolver;
    const size_t maxmessage;
    int retval;
    static int uniqueid;

    Thread::Mutex& resolve_mutex;
    
    DnsWorker(const DnsWorker&);
};

class UdpWorker : public DnsWorker {
public:
    UdpWorker(DnsResolver& resolver, const UdpSocket& s, Thread::Mutex& resolvemutex, const size_t maxmessage=UdpSocket::DEFAULT_MAX_MSG)
        throw (Socket::SocketException);

    void   setup();
    void   teardown();
    size_t readQuery(char* buff, size_t maxmessage) throw(Socket::SocketException);
    size_t sendResponse(const char* buff, size_t maxmessage) throw(Socket::SocketException);
    std::string what() const;

private:
    UdpWorker(const UdpWorker& src);

    const UdpSocket& socket;
    Socket::SocketAddress clientAddress;
};


class TcpWorker : public DnsWorker{
public:
    TcpWorker(
        DnsResolver& resolver, const TcpSocket& socket, Thread::Mutex& acceptmutex,
        Thread::Mutex& resolvemutex, const size_t maxmessage=TcpSocket::DEFAULT_MAX_MSG)
        throw ();
    ~TcpWorker() throw ();

    void setup() throw (Socket::SocketException);
    void   teardown() throw (Socket::SocketException);
    size_t readQuery(char* buff, size_t maxmessage) throw(Socket::SocketException);
    size_t sendResponse(const char* buff, size_t maxmessage) throw(Socket::SocketException); /*  */
    std::string what() const;

private:
    TcpWorker(const TcpWorker& src);

    const TcpSocket& serverSocket;
    TcpSocket* connectedSocket;
    Thread::Mutex& acceptMutex;
};

#endif // DNS_WORKER
