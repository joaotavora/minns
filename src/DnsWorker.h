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
    static bool stop_flag;

    std::string report() const;
    
private:
    void work();

protected:
    virtual void   setup() = 0;
    virtual void   teardown() = 0;
    virtual size_t readQuery(char* buff, size_t maxmessage) throw(Socket::SocketException)= 0;
    virtual size_t sendResponse(const char* buff, size_t maxmessage) throw(Socket::SocketException)= 0;
    virtual std::string name() const = 0;

    std::string what() const;

    void*   main ();

    DnsWorker(DnsResolver& _resolver, Thread::Mutex &_resolve_mutex, const size_t _maxmessage);
    int id;

private:
    static void sig_alrm_handler(int signo);
    
    DnsResolver& resolver;
    const size_t maxmessage;
    int retval;
    static int uniqueid;

    unsigned int served;
    unsigned int served_error;

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
    
private:
    std::string name() const;
    UdpWorker(const UdpWorker& src);

    const UdpSocket& socket;
    Socket::SocketAddress clientAddress;
};


class TcpWorker : public DnsWorker{
public:
    TcpWorker(
        DnsResolver& resolver, const TcpSocket& socket, Thread::Mutex& acceptmutex,
        Thread::Mutex& resolvemutex, unsigned int timeout, const size_t maxmessage=TcpSocket::DEFAULT_MAX_MSG)
        throw ();
    ~TcpWorker() throw ();

    void setup() throw (Socket::SocketException);
    void   teardown() throw (Socket::SocketException);
    size_t readQuery(char* buff, size_t maxmessage) throw(Socket::SocketException);
    size_t sendResponse(const char* buff, size_t maxmessage) throw(Socket::SocketException); /*  */

private:
    std::string name() const;
    TcpWorker(const TcpWorker& src);

    const TcpSocket& serverSocket;
    TcpSocket* connectedSocket;
    Thread::Mutex& acceptMutex;

    struct timeval timeout_tv;
};

#endif // DNS_WORKER
