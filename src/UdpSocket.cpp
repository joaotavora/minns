// project includes
#include "UdpSocket.h"

UdpSocket::UdpSocket() throw ()
    : Socket(socket (AF_INET, SOCK_DGRAM, 0)){
    if (sockfd == -1){
        throw SocketException(errno, "Could not create socket");
    }
}

std::ostream& operator<<(std::ostream& os, const UdpSocket& sock){
    return os << "[UDP " << (const Socket&) sock << "]";
}

size_t UdpSocket::sendto(const char* msg, size_t len, const SocketAddress& to) const throw (SocketException){
    if (len <= 0)
        throw SocketException("invalid amount of bytes to send: "+len);
    ssize_t sent = ::sendto(
        sockfd,
        msg,
        len,
        0,
        reinterpret_cast<struct sockaddr*>(&to.sockaddr),
        to.socklen);
    if (sent == -1)
        throw SocketException(errno, "send() error");
    return sent;
}

size_t UdpSocket::recvfrom(char* result, size_t size, SocketAddress& from) const throw (SocketException){
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
    if (sendto(msg.c_str(), msg.size(), to) != msg.size())
        throw SocketException("sendto(string&,...) error: not enough bytes sent");
}

std::string& UdpSocket::recvfrom(SocketAddress &from) const throw (SocketException){
    static char buff[UdpSocket::MAXLINE];

    recvfrom(buff,UdpSocket::MAXLINE,from);
    return *new std::string(buff);
}



