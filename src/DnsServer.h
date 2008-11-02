#ifndef DNS_SERVER_H
#define DNS_SERVER_H

// stdl includes
#include <string>
#include <list>
#include <stdexcept>
#include <memory> // auto_ptr

// Project includes
#include "Socket.h"
#include "UdpSocket.h"
#include "DnsMessage.h"
#include "DnsResolver.h"
#include "DnsWorker.h"

class DnsServer {
public:
    DnsServer(
        DnsResolver& resolver,
        const unsigned int udpport,
        const unsigned int tcpport,
        const unsigned int udpworkers,
        const unsigned int tcpworkers) throw (std::exception);
    ~DnsServer();

    void start();
    void stop();

private:
    std::list<DnsWorker*> workers;

    bool stopFlag;
    Thread::Mutex acceptMutex;
    TcpSocket tcp_serversocket;
    UdpSocket udp_serversocket;

    static const unsigned int DEFAULT_UDP_PORT = 53;
    static const unsigned int DEFAULT_TCP_PORT = 53;
    static const unsigned int DEFAULT_UDP_WORKERS = 1;
    static const unsigned int DEFAULT_TCP_WORKERS = 5;
};

#endif // DNS_SERVER_H
