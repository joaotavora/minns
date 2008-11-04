// stdl includes
#include <sstream>

// project includes
#include "trace.h"
#include "UdpSocket.h"

UdpSocket::UdpSocket() throw ()
    :
    Socket(socket (AF_INET, SOCK_DGRAM, 0))
{
    if (sockfd == -1){
        throw SocketException(errno, TRACELINE("Could not socke()t"));
    }
}

std::ostream& operator<<(std::ostream& os, const UdpSocket& sock){
    return os << "[UDP " << (const Socket&) sock << "]";
}

size_t UdpSocket::sendto(const char* msg, const SocketAddress& to, size_t len) const throw (SocketException){
    if (len <= 0){
        std::stringstream ss;
        ss << TRACELINE("invalid amount of bytes to send: ") << len;
        throw SocketException(ss.str().c_str());
    }
    ssize_t sent = ::sendto(
        sockfd,
        msg,
        len,
        0,
        (struct sockaddr*)(&to.sockaddr),
        to.socklen);
    if (sent == -1)
        throw SocketException(errno, TRACELINE("Could not send()"));
    return sent;
}

size_t UdpSocket::recvfrom(char* result, SocketAddress& from, size_t size) const throw (SocketException){
    int recvd;
    if ((recvd = ::recvfrom(
                sockfd,
                result,
                size,
                0,
                reinterpret_cast<struct sockaddr*>(&from.sockaddr),
                &from.socklen)) < 0)
        throw SocketException(errno, TRACELINE("Could not recvfrom()"));
    else
        return recvd;
}

void UdpSocket::sendto(const std::string& msg, const SocketAddress& to) const throw (SocketException){
    if (sendto(msg.c_str(), to, msg.size() + 1) != msg.size() + 1)
        throw SocketException(TRACELINE("not enough bytes sent"));
}

std::string& UdpSocket::recvfrom(SocketAddress &from) const throw (SocketException){
    char buff[DEFAULT_MAX_MSG];

    size_t read = recvfrom(buff,from, DEFAULT_MAX_MSG);
    if (read < DEFAULT_MAX_MSG) buff[read]='\0'; // safe null terminate

    std::string& retval = *new std::string(buff);

    return retval;
}



