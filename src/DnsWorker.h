#ifndef DNS_WORKER_H
#define DNS_WORKER_H

// libc includes

// stdl includes

// Project includes
#include "Thread.h"
#include "UdpSocket.h"
#include "TcpSocket.h"
#include "DnsResolver.h"
#include "DnsMessage.h"

class DnsWorker : public Thread::Runnable {
public:
    virtual ~DnsWorker();
    void rest();

private:
    void work();

protected:
    virtual void   setup();
    virtual size_t readQuery(char* buff, size_t maxmessage);
    virtual size_t sendResponse(const char* buff, size_t maxmessage);

protected:
    DnsWorker(DnsResolver& resolver, const size_t maxmessage);

private:
    bool stop_flag;
    DnsResolver& resolver;
    const size_t maxmessage;
};

class UdpWorker : public DnsWorker{
public:
    UdpWorker(DnsResolver& resolver, const size_t maxmessage=UdpSocket::DEFAULT_MAX_MSG)
        throw (Socket::SocketException);

    void   setup();
    size_t readQuery(char* buff, size_t maxmessage);    
    size_t sendResponse(const char* buff, size_t maxmessage);
private:
    UdpSocket& socket;
    Socket::SocketAddress clientAddress;
};


class TcpWorker : public DnsWorker{
public:
    TcpWorker(DnsResolver& resolver, TcpSocket& socket, Thread::Mutex& mutex, const size_t maxmessage);
    size_t readQuery(char* buff, size_t maxmessage);    
    size_t sendResponse(char* buff, size_t maxmessage);
private:
    TcpSocket& socket;
    TcpSocket* connectedSocket;
    Thread::Mutex& mutex;
};

#endif // DNS_WORKER
