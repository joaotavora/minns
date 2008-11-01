// stdl includes

#include <iostream>
#include <string>

// Project includes

#include "TcpSocket.h"

using namespace std;

// Class members definition

TcpSocket::TcpSocket() throw (Socket::SocketException)
    :
    Socket(socket (AF_INET, SOCK_STREAM, 0)),
    max_receive(DEFAULT_MAX_MSG)
{
    // cout << "TcpSocket ctor\n";

    if (sockfd == -1){
        throw SocketException(errno, "Could not create socket");
    }

    int on;
    if (setsockopt (sockfd,
            SOL_SOCKET,
            SO_REUSEADDR,
            (const char*) &on,
            sizeof (on)) != 0)
    {
        throw SocketException(errno, "Could not setsockopt");
    }
}

TcpSocket::TcpSocket(int fd, SocketAddress& addr)
    :
    Socket(fd,addr),
    max_receive(DEFAULT_MAX_RECV) {
    // cerr << "  TcpSocket ctor\n";
    }

TcpSocket::~TcpSocket(){
    // cout << "TcpSocket dtor for: " << *this << endl;
    for (unsigned int i = 0; i < connected.size(); i++)
        delete connected[i];
    connected.clear();
}

void TcpSocket::connect(const string& host, const int port) throw (SocketException){
    SocketAddress toClient(host.c_str(), port);

    sockaddr_in clientaddress;
    clientaddress.sin_family         = AF_INET;
    clientaddress.sin_port           = htons(port);

    inet_pton (AF_INET, host.c_str(), &clientaddress.sin_addr);

    // if (::connect(sockfd, reinterpret_cast<struct sockaddr *>(&clientaddress), sizeof(sockaddr_in)) != 0)
    //     throw SocketException(errno, "Could not connect()");

    if (::connect(sockfd, reinterpret_cast<struct sockaddr *>(&toClient.sockaddr), sizeof(sockaddr_in)) != 0)
        throw SocketException(errno, "Could not connect()");
}

void TcpSocket::listen(const int max_connections) throw (SocketException){
    if (::listen (sockfd, max_connections) != 0)
        throw SocketException(errno, "Cound not accept()");
}

TcpSocket* TcpSocket::accept() throw (SocketException){
    int clifd;

    SocketAddress& fromClient = *new SocketAddress();
    if ((clifd = ::accept (sockfd,
                reinterpret_cast<struct sockaddr *>(&fromClient.sockaddr),
                &fromClient.socklen)) == -1)
        throw SocketException("Could not accept()");

    TcpSocket* client = new TcpSocket(clifd, fromClient);
    connected.push_back(client);
    return client;
}

void TcpSocket::writeline(const string s) const throw (SocketException){
    string::size_type remaining=s.size();
    string::size_type write_cnt;
    const char* buff = s.c_str();

again:
    if ((write_cnt = ::write(sockfd, buff, remaining)) != remaining)
        if (errno == EINTR){
            remaining -= write_cnt;
            goto again;
        } else throw SocketException(errno, "write() error");
}

// @returns
//      string::npos if EOF received
//
//
string::size_type TcpSocket::readline(std::string& retval, const char delimiter) throw (SocketException){
    int read_cnt = 0;
    int remaining=max_receive;
    char* temp= new char[max_receive];

again:
    read_cnt = ::read(sockfd,temp,remaining);
    remaining -= read_cnt;

    if (read_cnt < 0){
        // No data read, interruption or maybe more serious error
        if (errno == EINTR)
            goto again;
        else {
            delete []temp;
            throw new SocketException(errno, "read() error");
        }
    } else if (read_cnt == 0) {
        // EOF or no more characters remaining
        // append whatever is in read_buffer to retval, return npos
        retval.append(read_buffer);
        delete []temp;
        return std::string::npos;
    } else {
        // Some data read, null-terminate and add to read buffer
        temp[read_cnt] = '\0';
        read_buffer.append(temp);

        // Try to find newline and split accoringly
        std::string::size_type newline_idx = read_buffer.find_first_of(delimiter);
        if (newline_idx != std::string::npos){
            retval.append(read_buffer.substr(0, newline_idx + 1));
            read_buffer = read_buffer.substr(newline_idx+1);
            delete []temp;
            return max_receive-remaining-read_buffer.size();
        } else {
            retval.append(read_buffer);
            read_buffer.assign("");
            if (remaining == 0) {
                delete []temp;
                return read_cnt;
            }
        }
        // newline not found and remaining not exhausted, try another read system call
        goto again;
    }
}

void TcpSocket::setMaxReceive(const std::string::size_type howmany){max_receive=howmany;}

std::string::size_type TcpSocket::getMaxReceive() const{return max_receive;}

// Non-member operator redefinition

const TcpSocket& operator<<(const TcpSocket& ts, const std::string& s) throw (Socket::SocketException){
    ts.write(s);
    return ts;
}

bool operator>>(TcpSocket& ts, std::string& towriteto) throw (Socket::SocketException){
    string temp;
    std::string::size_type written = ts.readline(temp);
    towriteto.assign(temp);
    return (written != std::string::npos);
}

std::ostream& operator<<(std::ostream& os, const TcpSocket& sock){
    return os << "[TCP " << (const Socket&) sock << "]";
}


