#ifndef UDP_SOCKET_H
#define UDP_SOCKET_H

// stdl includes
#include <string>

// Project includes
#include "Socket.h"

class UdpSocket : public Socket {
public:

    UdpSocket() throw ();

    // raw char data sendto and recvfrom
    size_t sendto(const char* msg, const SocketAddress& to, size_t len = DEFAULT_MAX_MSG) const throw (SocketException);
    size_t recvfrom(char* result, SocketAddress& from, size_t size = DEFAULT_MAX_MSG) const throw (SocketException);

    // string send and receive
    void sendto(const std::string& msg, const SocketAddress& to) const throw (SocketException);
    std::string& recvfrom(SocketAddress &from) const throw (SocketException);

    static const unsigned int DEFAULT_MAX_MSG=512;
private:
    UdpSocket(const UdpSocket& src);

};


#endif // UDP_SOCKET_H
