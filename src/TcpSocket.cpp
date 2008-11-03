// stdl includes

#include <iostream>
#include <string>

// Project includes

#include "TcpSocket.h"

using namespace std;

// Class members definition

TcpSocket::TcpSocket() throw (Socket::SocketException)
    :
    Socket(socket (AF_INET, SOCK_STREAM, 0)), maxreceive(DEFAULT_MAX_MSG){
    // cout << "TcpSocket ctor\n";

    if (sockfd == -1){
        throw SocketException(errno, "Could not create socket");
    }
}

TcpSocket::TcpSocket(int fd, SocketAddress& addr)
    :
    Socket(fd), maxreceive(DEFAULT_MAX_MSG) {
    address = addr; // use SocketAddress's copy constructor
}

TcpSocket::~TcpSocket(){
}

void TcpSocket::connect(const string& host, const int port) throw (SocketException){
    SocketAddress toClient(host.c_str(), port);

    sockaddr_in clientaddress;
    clientaddress.sin_family         = AF_INET;
    clientaddress.sin_port           = htons(port);

    inet_pton (AF_INET, host.c_str(), &clientaddress.sin_addr);

    if (::connect(sockfd, reinterpret_cast<struct sockaddr *>(&toClient.sockaddr), sizeof(sockaddr_in)) != 0)
        throw SocketException(errno, "Could not connect()");
}

void TcpSocket::listen(const int max_connections) throw (SocketException){
    if (::listen (sockfd, max_connections) != 0)
        throw SocketException(errno, "Cound not accept()");
}

TcpSocket* TcpSocket::accept() const throw (SocketException){
    int clifd;

    SocketAddress fromClient;
    if ((clifd = ::accept (sockfd,
                reinterpret_cast<struct sockaddr *>(&fromClient.sockaddr),
                &fromClient.socklen)) == -1)
        throw SocketException("Could not accept()");

    TcpSocket* client = new TcpSocket(clifd, fromClient);
    return client;
}

// Data Transmission - raw byte functions
size_t TcpSocket::read(char* buff, const size_t howmany) throw (SocketException){
    size_t read_cnt = 0;

again:
    read_cnt = ::read(sockfd,buff,howmany);
    if (read_cnt < 0){
        // No data read, interruption or maybe more serious error
        if (errno == EINTR)
            goto again;
        else {
            throw new SocketException(errno, "read() error");
        }
    }
    return read_cnt;
}

size_t TcpSocket::write(const char* buff, const size_t howmany) throw (SocketException){
    size_t write_cnt;
again:
    if ((write_cnt = ::write(sockfd, buff, howmany)) < 0){
        if (errno == EINTR){
            goto again;
        } else throw SocketException(errno, "write() error");
    }
    return write_cnt;
}

void TcpSocket::writeline(const string s) const throw (SocketException){
    size_t remaining=s.size();
    size_t write_cnt;
    const char* buff = s.c_str();

again:
    if ((write_cnt = ::write(sockfd, buff, remaining)) != remaining){
        if (errno == EINTR){
            remaining -= write_cnt;
            goto again;
        } else throw SocketException(errno, "write() error");
    }
}


size_t TcpSocket::readline(string& retval, const char delimiter, const size_t maxlen) throw (SocketException){
    size_t read_cnt = 0;
    char* temp= new char[maxlen+1];

    retval.assign("");

again:
    read_cnt += ::read(sockfd,temp,maxreceive);
    // cout << "     (read " << read_cnt << " chars)" << endl;

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
        return string::npos;
    } else {
        // Some data read, null-terminate and add to read buffer
        temp[read_cnt] = '\0';
        read_buffer.append(temp);

        // Try to find newline and split accoringly
        size_t newline_idx = read_buffer.find_first_of(delimiter);
        if (newline_idx != string::npos){
            retval.append(read_buffer.substr(0, newline_idx + 1));
            read_buffer = read_buffer.substr(newline_idx+1);
            delete []temp;
            return retval.size();
        } else {
            retval.append(read_buffer);
            read_buffer.assign("");
            if (read_cnt == maxlen) {
                delete []temp;
                // cout << "     (already read " << maxlen << " chars)" << endl;
                return read_cnt;
            }
        }
        // newline not found and remaining not exhausted, try another read system call
        // cout << "     (going for another " << maxreceive << " read)" << endl;
        goto again;
    }
}

void TcpSocket::setMaxReceive(size_t howmany) {
    maxreceive=howmany;
}

size_t TcpSocket::getMaxReceive() const {
    return maxreceive;
}

// Non-member operator redefinition

const TcpSocket& operator<<(const TcpSocket& ts, const string& s) throw (Socket::SocketException){
    ts.writeline(s);
    return ts;
}

bool operator>>(TcpSocket& ts, string& towriteto) throw (Socket::SocketException){
    string temp;
    size_t read = ts.readline(temp);
    towriteto.append(temp);
    // cout << "     (operator>> reports " << read << " read line is " << towriteto << ")" << endl;
    return read != string::npos;
}

ostream& operator<<(ostream& os, const TcpSocket& sock){
    return os << "[TCP " << (const Socket&) sock << "]";
}




