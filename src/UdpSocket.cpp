// stdl includes
#include <sstream>

// project includes
#include "UdpSocket.h"

UdpSocket::UdpSocket() throw ()
    :
    Socket(socket (AF_INET, SOCK_DGRAM, 0))
{
    if (sockfd == -1){
        throw SocketException(errno, "Could not create socket");
    }
}

std::ostream& operator<<(std::ostream& os, const UdpSocket& sock){
    return os << "[UDP " << (const Socket&) sock << "]";
}

size_t UdpSocket::sendto(const char* msg, const SocketAddress& to, size_t len) const throw (SocketException){
    if (len <= 0){
        std::stringstream ss;
        ss << "invalid amount of bytes to send: " << len;
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
        throw SocketException(errno, "send() error");
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
        throw SocketException(errno, "recvfrom() error");
    else
        return recvd;
}

void UdpSocket::sendto(const std::string& msg, const SocketAddress& to) const throw (SocketException){
    if (sendto(msg.c_str(), to, msg.size()) != msg.size())
        throw SocketException("sendto(string&,...) error: not enough bytes sent");
}

std::string& UdpSocket::recvfrom(SocketAddress &from) const throw (SocketException){
    char buff[DEFAULT_MAX_MSG];

    recvfrom(buff,from, DEFAULT_MAX_MSG);

    std::string& retval = *new std::string(buff);

    return retval;
}



