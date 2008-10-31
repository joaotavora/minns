#ifndef DNS_SERVER_H
#define DNS_SERVER_H

// stdl includes
#include <string>
#include <stdexcept>

// Project includes
#include "Socket.h"
#include "UdpSocket.h"
#include "DnsMessage.h"
#include "DnsResolver.h"

class DnsServer {
public:
    DnsServer(
        const std::string& filename,
        const unsigned int port,
        const unsigned int cachesize,
        const unsigned int maxaliases,
        const unsigned int maxmessage) throw (std::exception);
    ~DnsServer();

    void start();
    void stop();

private:

    DnsResponse* handle(const DnsMessage& query, size_t maxmessage) throw (DnsMessage::DnsException);
    DnsErrorResponse* handle(const DnsMessage::DnsException& e) throw ();

    UdpSocket& socket;
    DnsResolver& resolver;
    unsigned int maxmessage;

    bool stopFlag;

    static const unsigned int DEFAULT_DNS_PORT = 35;
};

#endif // DNS_SERVER_H
