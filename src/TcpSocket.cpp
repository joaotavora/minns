// stdl includes

#include <iostream>
#include <string>

// Project includes
#include "trace.h"
#include "TcpSocket.h"

using namespace std;

// Class members definition

TcpSocket::TcpSocket() throw (Socket::SocketException)
    :
    Socket(socket (AF_INET, SOCK_STREAM, 0)), maxreceive(DEFAULT_MAX_MSG){
    // ctrace << "TcpSocket ctor\n";

    if (sockfd == -1){
        throw SocketException(errno, TRACELINE("Could not socket()"));
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
        throw SocketException(errno, TRACELINE("Could not connect()"));
}

void TcpSocket::listen(const int max_connections) throw (SocketException){
    if (::listen (sockfd, max_connections) != 0)
        throw SocketException(errno, TRACELINE("Cound not listen()"));
}

TcpSocket* TcpSocket::accept() const throw (SocketException){
    int clifd;

    SocketAddress fromClient;
    if ((clifd = ::accept (sockfd,
                reinterpret_cast<struct sockaddr *>(&fromClient.sockaddr),
                &fromClient.socklen)) == -1)
        throw SocketException(errno, TRACELINE("Could not accept()"));

    TcpSocket* client = new TcpSocket(clifd, fromClient);
    return client;
}

// Data Transmission - raw byte functions
size_t TcpSocket::read(char* buff, const size_t howmany, const bool* stopflag) const throw (SocketException){
    size_t read_cnt = 0;

again:
    read_cnt = ::read(sockfd,buff,howmany);
    if (read_cnt < 0){
        // No data read, interruption or maybe more serious error
        if ((errno == EINTR) and ((stopflag == NULL) or (*stopflag==false))){
            ctrace << "TcpSocket::read caught EINTR. restarting..." << endl;
            goto again;
        } else {
            throw new SocketException(errno, TRACELINE("Could not read()"));
        }
    }
    return read_cnt;
}

size_t TcpSocket::write(const char* buff, const size_t howmany, const bool* stopflag) const throw (SocketException){
    size_t write_cnt;
again:
    if ((write_cnt = ::write(sockfd, buff, howmany)) < 0){
        if ((errno == EINTR) and ((stopflag == NULL) or (*stopflag==false))){
            ctrace << "TcpSocket::write caught EINTR. restarting..." << endl;
            goto again;
        } else throw SocketException(errno, TRACELINE("Could not write()"));
    }
    return write_cnt;
}

size_t TcpSocket::writeline(const string s, const bool* stopflag) const throw (SocketException){
    return write(s.c_str(), s.size(), stopflag);
}

size_t TcpSocket::readline(string& retval, const char delimiter, const size_t maxlen, const bool* stopflag) throw (SocketException){
    size_t read_cnt = 0;
    char* temp= new char[maxlen+1];

    retval.assign("");

again:
    try {
        read_cnt += read(temp,maxreceive, stopflag);
    } catch (SocketException& e) {
        delete []temp;
        throw e;
    }
    if (read_cnt == 0) {
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
                // ctrace << "     (already read " << maxlen << " chars)" << endl;
                return read_cnt;
            }
        }
        // newline not found and remaining not exhausted, try another read system call
        // ctrace << "     (going for another " << maxreceive << " read)" << endl;
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
    // ctrace << "     (operator>> reports " << read << " read line is " << towriteto << ")" << endl;
    return read != string::npos;
}

ostream& operator<<(ostream& os, const TcpSocket& sock){
    return os << "[TCP " << (const Socket&) sock << "]";
}




