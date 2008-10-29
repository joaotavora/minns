#ifndef DNS_SERVER_H
#define DNS_SERVER_H

#include "Socket.h"
#include "UdpSocket.h"

class DnsServer {
public:
    void start();
    void handle(UdpSocket& connected) throw (Socket::SocketException);
};

#endif // DNS_SERVER_H
