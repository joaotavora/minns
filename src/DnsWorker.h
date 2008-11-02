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
    virtual size_t readQuery(char* buff, size_t maxmessage) throw(Socket::SocketException)= 0;
    virtual size_t sendResponse(const char* buff, size_t maxmessage) throw(Socket::SocketException)= 0;
    void*   main ();

protected:
    DnsWorker(DnsResolver& resolver, const size_t maxmessage);
    int id;

private:
    bool stop_flag;
    // time_t last_punch;
    DnsResolver& resolver;
    const size_t maxmessage;
    int retval;
    static int uniqueid;
};

class UdpWorker : public DnsWorker {
public:
    UdpWorker(DnsResolver& resolver, const UdpSocket& s, const size_t maxmessage=UdpSocket::DEFAULT_MAX_MSG)
        throw (Socket::SocketException);

    void   setup();
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
    TcpWorker(DnsResolver& resolver, const TcpSocket& socket, Thread::Mutex& mutex, const size_t maxmessage=TcpSocket::DEFAULT_MAX_MSG) throw ();
    ~TcpWorker() throw ();

    void setup() throw (Socket::SocketException);
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
