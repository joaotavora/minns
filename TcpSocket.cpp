#include <iostream>
#include <string>

#include "Exception.h"
#include "TcpSocket.h"

using namespace std;

TcpSocket::TcpSocket() : sockaddr(*new sockaddr_in){

    // sockaddr = *new sockaddr_in(0);

    if ((sockfd = socket (AF_INET, SOCK_STREAM, 0)) == -1)
        throw new Exception("Could not create socket", errno);

    int on;
    if (setsockopt (sockfd,
            SOL_SOCKET,
            SO_REUSEADDR,
            (const char*) &on,
            sizeof (on)) != 0)
        throw new Exception("Could not setsockopt", errno);
}

TcpSocket::TcpSocket(int fd, int addr_length, TcpSocket& listening){
}

TcpSocket::~TcpSocket(){
    //TODO: review this
    cout << "TcpSocket dtor\n";
}

void TcpSocket::connect(const string host, const int port){
    if (bound()) throw new Exception("Socket already bound");

    sockaddr.sin_family = AF_INET;
    sockaddr.sin_port   = htons(port);

    if (inet_pton(AF_INET, host.c_str(), &sockaddr.sinaddr) == -1)
        throw new Exception("Could not convert address",errno);

    if (::connect(sockfd, (sockaddr *) &sockaddr, sizeof(sockaddr)) != 0)
        throw new Exception("Could not connect()", errno);

    setStatus(CONNECTED);
}

void TcpSocket::bind(const int port){
    if (bound()) throw new Exception("Socket already bound");

    sockaddr.sin_family         = AF_INET;
    sockaddr.sin_addr.s_addr    = INADDR_ANY;
    sockaddr.sin_port           = htons(port);

    if (::bind(sockfd, (struct sockaddr *)&sockaddr, sizeof(sockaddr)) != 0)
        throw new Exception("Could not bind()", errno);

    bound = true;
}

void TcpSocket::listen() const {
    if (!bound()) throw new Exception("Socket isn't bound yet");

    // TODO: review if MAXCONNECTIONS should be a parameter to the socket.
    if (::listen (sockfd, MAXCONNECTIONS) != 0)
        throw new Exception("Cound not accept()", errno);
}

TcpSocket& TcpSocket::accept() const {
    if (!listening()) throw new Exception("Socket isn't listening");

    int clifd;
    sockaddr_in* cliaddr = new sockaddr_in(0);
    socklen_t* cliaddrlen =new socklen_t;

    int cliaddrlen = sizeof (cliaddr);

    if ((clifd = ::accept (sockfd,
                (sockaddr *) cliaddr
                (socklen_t *) cliaddrlen)) == -1)
        throw new Exception("Could not accept()");

    return new TcpSocket(clifd, *cliaddr, *cliaddrlen,this);
}

void TcpSocket::send(const string s) const{
    cout << "Would be sending: " << s << " to socket\n";
}

void TcpSocket::recv(string& s) const{
    cout << "Would be sending: " << s << " to socket\n";
}


















