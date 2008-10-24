#include <iostream>
#include <string>

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>

#include "Exception.h"
#include "TcpSocket.h"

using namespace std;

TcpSocket::TcpSocket()
    : sockaddr(*new sockaddr_in),
      socklen(sizeof(socklen_t)),
      sockfd(socket (AF_INET, SOCK_STREAM, 0)),
      max_connections(DEFAULT_MAX_CONNECTIONS),
      read_buffer(*new std::string)
{

    if (sockfd == -1)
        throw *new SocketException("Could not create socket", errno);

    setStatus(FRESH);

    bzero(&sockaddr, sizeof(struct sockaddr_in));

    int on;
    if (setsockopt (sockfd,
            SOL_SOCKET,
            SO_REUSEADDR,
            (const char*) &on,
            sizeof (on)) != 0)
        throw *new SocketException("Could not setsockopt", errno);
}

TcpSocket::TcpSocket(int fd, sockaddr_in& addr, socklen_t& len)
    : sockaddr(sockaddr),
      socklen(len),
      sockfd(fd),
      max_connections(DEFAULT_MAX_CONNECTIONS),
      read_buffer(*new std::string) {}

TcpSocket::~TcpSocket(){
    if (::close(sockfd) != 0)
        throw *new SocketException("Could not close socket", errno);
}

void TcpSocket::connect(const string host, const int port){
    if (bound()) throw *new SocketException("Socket already bound");

    sockaddr.sin_family = AF_INET;
    sockaddr.sin_port   = htons(port);

    if (inet_pton(AF_INET, host.c_str(), &sockaddr.sin_addr) == -1)
        throw *new SocketException("Could not convert address",errno);

    if (::connect(sockfd, reinterpret_cast<struct sockaddr *>(&sockaddr), sizeof(sockaddr)) != 0)
        throw *new SocketException("Could not connect()", errno);

    setStatus(CONNECTED);
}

void TcpSocket::bind(const int port){
    if (bound()) throw *new SocketException("Socket already bound");

    sockaddr.sin_family         = AF_INET;
    sockaddr.sin_addr.s_addr    = INADDR_ANY;
    sockaddr.sin_port           = htons(port);

    if (::bind(sockfd, reinterpret_cast<struct sockaddr *>(&sockaddr), sizeof(sockaddr)) != 0)
        throw *new SocketException("Could not bind()", errno);

    setStatus(BOUND);
}

void TcpSocket::listen(){
    if (!bound()) throw *new SocketException("Socket isn't bound yet");

    if (::listen (sockfd, max_connections) != 0)
        throw *new SocketException("Cound not accept()", errno);

    setStatus(LISTENING);
}

TcpSocket& TcpSocket::accept() const {
    if (!listening()) throw *new SocketException("Socket isn't listening");

    int clifd;
    sockaddr_in& cliaddr  = *(new sockaddr_in);
    socklen_t& cliaddrlen = *(new socklen_t(sizeof(sockaddr_in)));

    bzero(&cliaddr, sizeof(sockaddr_in));

    if ((clifd = ::accept (sockfd,
                reinterpret_cast<struct sockaddr *>(&cliaddr),
                &cliaddrlen)) == -1)
        throw *new SocketException("Could not accept()");

    TcpSocket& clisocket = *new TcpSocket(clifd, cliaddr, cliaddrlen);
    clisocket.setStatus(CONNECTED);
    return clisocket;
}

void TcpSocket::send(const string s) const{
    cout << "Would be sending: " << s << " to socket\n";
}

std::string::size_type TcpSocket::readline(std::string& retval, const int howmany) const {
    if (!connected())
        throw *new SocketException("Socket not connected");

    int read_cnt = 0;
    int remaining=howmany;
    char* temp= new char[howmany];

        again:
    if ((read_cnt = read(sockfd,temp,remaining)) < 0){
        // No data read, interruption or maybe more serious error
        if (errno == EINTR)
            goto again;
        delete(temp);
        throw new SocketException("read() error", errno);
    } else if (read_cnt==0) {
        // EOF or no more characters remaining
        // return whatever is in retval, don't care about newline
        delete(temp);
        if (remaining != 0)
            return std::string::npos;
        else
            return remaining; // is 0;
    } else {
        // Some data read, null-terminate and add to read buffer, update remaining;
        temp[read_cnt] = '\0';
        read_buffer.append(temp);
        remaining -= read_cnt;

        // Try to find newline and split accoringly
        std::string::size_type newline_idx = read_buffer.find_first_of('\n');
        retval.append(read_buffer.substr(0,newline_idx+1));
        read_buffer = read_buffer.substr(newline_idx);

        // If newline found return retval immediately, even if remaining > 0
        if (newline_idx != std::string::npos){
            delete(temp);
            return howmany-remaining;
        }
        goto again;
    }
}

bool TcpSocket::fresh() const {return status==FRESH;}

bool TcpSocket::bound() const { return status==BOUND;}

bool TcpSocket::listening() const {return status==LISTENING;}

bool TcpSocket::connected() const {return status==CONNECTED;}

bool TcpSocket::closed() const {return status==CLOSED;}

void TcpSocket::setStatus(const enum status_t newStatus) {
    status=newStatus;
}

const string& TcpSocket::printStatus() const{
    switch (status){
    case BOUND:
        return *new string("Bound");
    case CONNECTED:
        return *new string("Connected");
    case LISTENING:
        return *new string("Listening");
    case CLOSED:
        return *new string("Closed");
    default:
        return *new string("New or unknown");
    }
}

std::ostream& operator<<(std::ostream& os, const TcpSocket& sock){
    string s = "[TcpSocket: status=\'" + sock.printStatus();
    if (!(sock.fresh() or sock.closed())){
        s += "\' address=\'";
        char buff[MAXHOSTNAME];
        if (inet_ntop(AF_INET, &sock.sockaddr, buff, MAXHOSTNAME)!=NULL)
            s += buff;
        else
            s += "<internal error>";
    } else { s += " address=irrevant!"; }
    s += "]";

    return os << s;
}

// Unit tests

TcpSocket& bindListenAccept(TcpSocket& serverSocket){
    serverSocket.bind(34343);
    serverSocket.listen();

    cout << "  Socket bound: " << serverSocket << endl
         << "  Waiting for accept() to return\n. ";
    // TODO: fork a child with connect here!
    TcpSocket& connected =  serverSocket.accept();
    cout << "  Connection accepted: " << connected << endl;
    return connected;
}

bool simpleAcceptConnectTest(){
    cout << "Starting simpleAcceptConnectTest() ...\n";
    try {
        TcpSocket& connectedSocket = bindListenAccept(*new TcpSocket());
        // TODO: fork a connecting client here
        cout << "  \n...done!\n";
        return true;
    } catch (SocketException& e) {
        cout << "  " << e << endl;
        cout << "  \n...failed!\n";
        return false;
    }
}

bool readOneLineFromClientTest(const int linesize=DEFAULT_MAX_RECV){
    cout << "Starting readOneLineFromClientTest() ...\n";
    try {
        TcpSocket& connectedSocket = bindListenAccept(*new TcpSocket());
        std::string clientmessage;
        cout << "  Reading one at most" << linesize << " chars long line from client\n";
        connectedSocket.readline(clientmessage, linesize);
        if (clientmessage.empty()){
            cout << "  \n...failed!\n";
            return false;
        }
        // std::string::size_type newline_idx=clientmessage.find_first_of('\n');
        // clientmessage.erase(newline_idx,1);
        cout << "  Read: \"" << clientmessage << "\"";
        cout << "  \n...done!\n";
        return true;
    } catch (SocketException& e) {
        cout << "  " << e << endl;
        return false;
    }
}

int main(int argc, char* argv[]){
    cout << "Starting TcpSocket unit tests\n";
    //simpleAcceptConnectTest();
    readOneLineFromClientTest(10);
    readOneLineFromClientTest(10);
    readOneLineFromClientTest(10);
    cout << "Done with TcpSocket unit tests\n";
}












