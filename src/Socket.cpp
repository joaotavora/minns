// TODO:
//
//  * figure out if the socklen member is really important...
//  * check why address on listening socket is always '0.2.134.39'
//  * check why address on either socket is always displayed as '0.0.0.0'

//  * verify automatic deletion and destructor call of temporary *new Object expressions. DONE THEY DONT GET DELETED!
//  * inline some functions, in the cpp file DONE
//  * add exception specifications !!!! DONE
//  * consider embedded member objects rather than reference member objects DONE
//  * don't throw exceptions in the constructor, or make sure all member objects are cleaned up DONE
//  * don't throw exceptions in the destructor, or make sure to catch them DONE
//  * find out about the standard c++ exception hiearachy. make SocketException nested in TcpSocket DONE

// stdl includes

#include <iostream>
#include <string>
#include <sstream>

// Project includes

#include "Socket.h"

using namespace std;

// Socket

Socket::Socket(int fd, SocketAddress& addr) throw ()
    : sockfd(fd), address(addr) {}

Socket::Socket(int fd) throw ()
    : sockfd(fd), address(*new SocketAddress()) {}

void Socket::bind_any(const int port) throw (SocketException){
    address.sockaddr.sin_family         = AF_INET;
    address.sockaddr.sin_addr.s_addr    = htonl(INADDR_ANY);
    address.sockaddr.sin_port           = htons(port);

    if (::bind(sockfd, reinterpret_cast<struct sockaddr *>(&address.sockaddr), sizeof(sockaddr_in)) != 0)
    {
        ostringstream ss;
        ss << "Could not bind() to port " << port;
        throw SocketException(errno, ss.str().c_str());
    }
}

void Socket::close() throw (SocketException){
    closed = true;
    if (::close(sockfd) != 0)
        throw SocketException(errno, "Could not close()");
}

Socket::~Socket(){
    // cout << "Socket dtor for: " << *this << endl;
    if (!closed){
        try {
            cerr << "Warning: closing socket" << *this << "in dtor!\n";
            close();
        } catch (SocketException& e) {
            cerr << "Socket: warning: dtor caught exception" << e;
        }
    }
    delete &address;
}

std::ostream& operator<<(std::ostream& os, const Socket& sock){
    return os << "[Socket: address=\'" << sock.address << "\']";
}

// Socket::SocketException nested class

Socket::SocketException::SocketException(const char* s) : std::runtime_error(s){}

Socket::SocketException::SocketException(int i,const char* s)
    : std::runtime_error(s), errno_number(i) {}

const char * Socket::SocketException::what() const throw(){
    string s = std::runtime_error::what();

    if (errno_number != 0){
        s += ": ";
        char buff[MAXERRNOMSG];
        strerror_r(errno_number, buff, MAXERRNOMSG);
        s += buff;
    };
    return (s.c_str());
}

std::ostream& operator<<(std::ostream& os, const Socket::SocketException& e){
    return os << "[SocketException: " << e.what() << "]";
}

// Socket::SocketAddress nested class

Socket::SocketAddress::SocketAddress() throw () :
    sockaddr(*new sockaddr_in), socklen(sizeof(sockaddr_in)) {
    bzero(&sockaddr,sizeof(sockaddr_in));
}

Socket::SocketAddress::SocketAddress(sockaddr_in& addr, socklen_t len) throw ()
    : sockaddr(addr), socklen(len) {
    bzero(&sockaddr,len);
}

Socket::SocketAddress::SocketAddress(const char* hostname, const int port) throw (Socket::SocketException) :
    sockaddr(*new sockaddr_in), socklen(sizeof(sockaddr_in)) {

    sockaddr.sin_family         = AF_INET;
    sockaddr.sin_port           = htons(port);

    int retval = inet_pton(AF_INET, hostname, &(sockaddr.sin_addr));
    switch (retval){
    case 0:
        throw SocketException("Address not parseable for AF_INET family");
    case -1:
        throw SocketException(errno, "inet_pton() error");
    default:
        break; // for clarity
    }
}

Socket::SocketAddress::~SocketAddress(){
    //cout << "SocketAddress dtor\n";
    delete &sockaddr;
}

std::ostream& operator<<(std::ostream& os, const Socket::SocketAddress& sock){
    string s;
    char buff[INET_ADDRSTRLEN];
    if (inet_ntop(AF_INET, &(sock.sockaddr.sin_addr), buff, INET_ADDRSTRLEN)!=NULL)
        s += buff;
    else
        s += "<internal error>";
    return os << s << ":" << ntohs(sock.sockaddr.sin_port);
}


