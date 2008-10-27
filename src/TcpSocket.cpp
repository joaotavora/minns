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

// Project includes

#include "TcpSocket.h"

using namespace std;

// Class members definition

TcpSocket::TcpSocket() throw (TcpSocket::SocketException)
    : socklen(sizeof(sockaddr_in)),
      sockfd(socket (AF_INET, SOCK_STREAM, 0)),
      max_connections(DEFAULT_MAX_CONNECTIONS),
      status(Status::FRESH),
      max_receive(DEFAULT_MAX_RECV)
{

    if (sockfd == -1){
        cleanup();
        throw SocketException(errno, "Could not create socket");
    }

    bzero(&sockaddr, sizeof(struct sockaddr_in));

    int on;
    if (setsockopt (sockfd,
            SOL_SOCKET,
            SO_REUSEADDR,
            (const char*) &on,
            sizeof (on)) != 0)
    {
        cleanup();
        throw SocketException(errno, "Could not setsockopt");
    }
}

TcpSocket::TcpSocket(int fd, sockaddr_in& addr, socklen_t& len, Status::status_t state)
    : sockaddr(sockaddr),
      socklen(len),
      sockfd(fd),
      max_connections(DEFAULT_MAX_CONNECTIONS),
      status(state),
      max_receive(DEFAULT_MAX_RECV) {}

TcpSocket::~TcpSocket(){
    // cout << "TcpSocket dtor for: " << this << endl;
    cleanup();
    if (!status.closed()){
        cerr << "TcpSocket: warning: closing socket in dtor\n";
        try {
            close();
        } catch (SocketException& e) {
            cerr << "TcpSocket: warning: dtor caught exception" << e;
        }
    }
}

void TcpSocket::cleanup(){}

void TcpSocket::connect(const string host, const int port) throw (SocketException){
    if (status.bound()) throw SocketException("Socket already bound");

    sockaddr.sin_family = AF_INET;
    sockaddr.sin_port   = htons(port);

    if (inet_pton(AF_INET, host.c_str(), &sockaddr.sin_addr) == -1)
        throw SocketException(errno, "Could not convert address");

    if (::connect(sockfd, reinterpret_cast<struct sockaddr *>(&sockaddr), sizeof(sockaddr)) != 0)
        throw SocketException(errno, "Could not connect()");

    status.setStatus(Status::CONNECTED);
}

void TcpSocket::bind(const int port) throw (SocketException){
    if (status.bound() or status.listening() or status.connected() ) throw SocketException("Socket already bound");

    sockaddr.sin_family         = AF_INET;
    sockaddr.sin_addr.s_addr    = INADDR_ANY;
    sockaddr.sin_port           = htons(port);

    if (::bind(sockfd, reinterpret_cast<struct sockaddr *>(&sockaddr), sizeof(sockaddr)) != 0)
        throw SocketException(errno, "Could not bind()");

    status.setStatus(Status::BOUND);
}

void TcpSocket::listen() throw (SocketException){
    if (!status.bound()) throw SocketException("Socket isn't bound yet");

    if (::listen (sockfd, max_connections) != 0)
        throw SocketException(errno, "Cound not accept()");

    status.setStatus(Status::LISTENING);
}

TcpSocket& TcpSocket::accept() const throw (SocketException){
    if (!status.listening()) throw SocketException("Socket isn't listening");

    int clifd;
    sockaddr_in cliaddr;
    socklen_t cliaddrlen = sizeof(sockaddr_in);

    bzero(&cliaddr, sizeof(sockaddr_in));

    if ((clifd = ::accept (sockfd,
                reinterpret_cast<struct sockaddr *>(&cliaddr),
                &cliaddrlen)) == -1)
        throw SocketException("Could not accept()");

    return *new TcpSocket(clifd, cliaddr, cliaddrlen, Status::CONNECTED);
}

void TcpSocket::close() throw (SocketException){
    status.setStatus(Status::CLOSED);
    if (::close(sockfd) != 0)
        throw SocketException(errno, "Could not close socket");
}

void TcpSocket::write(const string s) const throw (SocketException){
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
    if (!status.connected())
        throw SocketException("Socket not connected");

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

// TcpSocket::SocketException nested class

TcpSocket::SocketException::SocketException(const char* s)
    : std::runtime_error(s) {}

TcpSocket::SocketException::SocketException(int i, const char* s)
    : std::runtime_error(s), errno_number(i) {}

std::ostream& operator<<(std::ostream& os, const TcpSocket::SocketException& e){
    string s("[Exception: ");
    s += e.what();

    if (e.errno_number != 0){
        s += ": ";
        char buff[TcpSocket::SocketException::MAXERRNOMSG];
        strerror_r(e.errno_number, buff, TcpSocket::SocketException::MAXERRNOMSG);
        s += buff;
    };
    s += "]";

    return os << s;
}

// TcpSocket::Status nested class

inline bool TcpSocket::Status::fresh() const {return id==FRESH;}

inline bool TcpSocket::Status::bound() const { return id==BOUND;}

inline bool TcpSocket::Status::listening() const {return id==LISTENING;}

inline bool TcpSocket::Status::connected() const {return id==CONNECTED;}

inline bool TcpSocket::Status::closed() const {return id==CLOSED;}

inline void TcpSocket::Status::setStatus(const enum status_t newStatus) {
    id=newStatus;
}

inline const string& TcpSocket::Status::printStatus() const {
    static const std::string names[] =  {std::string("New or Unknown"),
                                         std::string("Bound"),
                                         std::string("Connected"),
                                         std::string("Listening"),
                                         std::string("Closed")};
    return names[id];
}

// Non-member operator redefinition

const TcpSocket& operator<<(const TcpSocket& ts, const std::string& s) throw (TcpSocket::SocketException){
    ts.write(s);
    return ts;
}

bool operator>>(TcpSocket& ts, std::string& towriteto) throw (TcpSocket::SocketException){
    string temp;
    std::string::size_type written = ts.readline(temp);
    towriteto.append(temp);
    return (written != std::string::npos);
}

std::ostream& operator<<(std::ostream& os, const TcpSocket& sock){
    string s = "[TcpSocket: status=\'" + sock.status.printStatus();
    if (!(sock.status.fresh() or sock.status.closed())){
        s += "\' address=\'";
        char buff[TcpSocket::MAXHOSTNAME];
        if (inet_ntop(AF_INET, &sock.sockaddr.sin_addr, buff, TcpSocket::MAXHOSTNAME)!=NULL)
            s += buff;
        else
            s += "<internal error>";
    } else { s += " address=irrevant!"; }
    s += "]";

    return os << s;
}
