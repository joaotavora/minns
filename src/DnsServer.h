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
        const unsigned int tcpworkers,
        const unsigned int tcptimeout) throw (std::exception);
    ~DnsServer();

    void start();
    void stop();

    static const unsigned int MAX_FILE_NAME = 512;
    static const char DEFAULT_HOSTS_FILE[MAX_FILE_NAME];


    static const unsigned int DEFAULT_UDP_PORT[3];
    static const unsigned int DEFAULT_TCP_PORT[3];
    static const unsigned int DEFAULT_UDP_WORKERS[3];
    static const unsigned int DEFAULT_TCP_WORKERS[3];
    static const unsigned int DEFAULT_TCP_TIMEOUT[3];

private:
    std::list<DnsWorker*> workers;

    bool stopFlag;
    Thread::Mutex accept_mutex;
    Thread::Mutex resolve_mutex;
    
    TcpSocket tcp_serversocket;
    UdpSocket udp_serversocket;
};

#endif // DNS_SERVER_H
