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
    void sendto(const std::string& msg, const SocketAddress& to) const throw (SocketException);
    std::string& recvfrom(SocketAddress &from) const throw (SocketException);

    // std::string sendto and recvfrom
    size_t sendto(const char* msg, size_t len, const SocketAddress& to) const throw (SocketException);
    size_t recvfrom(char* result, size_t size, SocketAddress& from) const throw (SocketException);

    static const int MAXLINE=200;

};


#endif // UDP_SOCKET_H
