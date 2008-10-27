// TODO:
//
//  * verify automatic deletion and destructor call of temporary *new Object expressions.
//  * add exception specifications !!!!
//  * inline some functions, in the cpp file
//  * figure out if the socklen member is really important...
//  * check why address on listening socket is always '0.2.134.39'
//  * check why address on either socket is always displayed as '0.0.0.0'

//  * consider embedded member objects rather than reference member objects DONE
//  * don't throw exceptions in the constructor, or make sure all member objects are cleaned up DONE
//  * don't throw exceptions in the destructor, or make sure to catch them DONE
//  * find out about the standard c++ exception hiearachy. make SocketException nested in TcpSocket DONE

// stdl includes

#include <iostream>
#include <string>

// libc includes

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>

// Project includes

#include "TcpSocket.h"


using namespace std;

// Class members definition

TcpSocket::TcpSocket()
    : socklen(sizeof(sockaddr_in)),
      sockfd(socket (AF_INET, SOCK_STREAM, 0)),
      max_connections(DEFAULT_MAX_CONNECTIONS),
      status(Status::FRESH)
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
      status(state) {}

TcpSocket::~TcpSocket(){
    cout << "TcpSocket dtor for: " << this << endl;
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

void TcpSocket::connect(const string host, const int port){
    if (status.bound()) throw SocketException("Socket already bound");

    sockaddr.sin_family = AF_INET;
    sockaddr.sin_port   = htons(port);

    if (inet_pton(AF_INET, host.c_str(), &sockaddr.sin_addr) == -1)
        throw SocketException(errno, "Could not convert address");

    if (::connect(sockfd, reinterpret_cast<struct sockaddr *>(&sockaddr), sizeof(sockaddr)) != 0)
        throw SocketException(errno, "Could not connect()");

    status.setStatus(Status::CONNECTED);
}

void TcpSocket::bind(const int port){
    if (status.bound() or status.listening() or status.connected() ) throw SocketException("Socket already bound");

    sockaddr.sin_family         = AF_INET;
    sockaddr.sin_addr.s_addr    = INADDR_ANY;
    sockaddr.sin_port           = htons(port);

    if (::bind(sockfd, reinterpret_cast<struct sockaddr *>(&sockaddr), sizeof(sockaddr)) != 0)
        throw SocketException(errno, "Could not bind()");

    status.setStatus(Status::BOUND);
}

void TcpSocket::listen(){
    if (!status.bound()) throw SocketException("Socket isn't bound yet");

    if (::listen (sockfd, max_connections) != 0)
        throw SocketException(errno, "Cound not accept()");

    status.setStatus(Status::LISTENING);
}

TcpSocket& TcpSocket::accept() const {
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

void TcpSocket::close() {
    status.setStatus(Status::CLOSED);
    if (::close(sockfd) != 0)
        throw SocketException(errno, "Could not close socket");
}

void TcpSocket::write(const string s) const {
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

string::size_type TcpSocket::readline(std::string& retval, const string::size_type howmany, const char delimiter) {
    if (!status.connected())
        throw SocketException("Socket not connected");

    int read_cnt = 0;
    int remaining=howmany;
    char* temp= new char[howmany];

        again:
    if ((read_cnt = ::read(sockfd,temp,remaining)) < 0){
        // No data read, interruption or maybe more serious error
        if (errno == EINTR)
            goto again;
        delete []temp;
        throw new SocketException(errno, "read() error");
    } else if (read_cnt==0) {
        // EOF or no more characters remaining
        // return whatever is in retval, don't care about newline
        delete []temp;
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
        std::string::size_type newline_idx = read_buffer.find_first_of(delimiter);
        retval.append(read_buffer.substr(0,newline_idx+1));
        read_buffer = read_buffer.substr(newline_idx);

        // If newline found return retval immediately, even if remaining > 0
        if (newline_idx != std::string::npos){
            delete []temp;
            return howmany-remaining;
        }
        goto again;
    }
}

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

inline const TcpSocket& operator<<(const TcpSocket& ts, const std::string& s){
    ts.write(s);
    return ts;
}

// Non-member operator redefinition

inline bool operator>>(TcpSocket& ts, std::string& towriteto){
    return (ts.readline(towriteto) != std::string::npos);
}

std::ostream& operator<<(std::ostream& os, const TcpSocket& sock){
    string s = "[TcpSocket: status=\'" + sock.status.printStatus();
    if (!(sock.status.fresh() or sock.status.closed())){
        s += "\' address=\'";
        char buff[MAXHOSTNAME];
        if (inet_ntop(AF_INET, &sock.sockaddr.sin_addr, buff, MAXHOSTNAME)!=NULL)
            s += buff;
        else
            s += "<internal error>";
    } else { s += " address=irrevant!"; }
    s += "]";

    return os << s;
}

// Unit test helpers

const int LOCALPORT = 34343;

TcpSocket& bindListenAccept(int port, TcpSocket& serverSocket){
    serverSocket.bind(port);
    serverSocket.listen();

    cout << "  Server socket bound: " << serverSocket << endl
         << "  Server accepting connections\n";
    // TODO: fork a child with connect here!
    TcpSocket& connected =  serverSocket.accept();
    cout << "  Connection accepted: " << connected << endl;
    return connected;
}

void checkChild(pid_t child_pid){
    int status;

    if ((child_pid==waitpid(child_pid, &status, 0)) and
        WIFEXITED(status) and
        (WEXITSTATUS(status) == 0)){
        return;
    } else
        throw std::runtime_error("Child process didn't terminate well");
}

// Unit tests

bool simpleAcceptConnectTest(){
    cout << "Starting simpleAcceptConnectTest() ...\n";
    pid_t child;
    // client code
    if ((child=fork())==0){
        try {
            TcpSocket toServer;
            cout << "  Forked child connecting to server\n";
            toServer.connect("localhost",LOCALPORT);
            cout << "  Forked child connected: " << toServer << endl;
            cout << "  Forked child closing connection to server\n";
            toServer.close();
            exit(0);
        } catch (TcpSocket::SocketException& e) {
            cout << "  Forked child exception: " << e << endl;
            exit(-1);
        }
        return false; // for clarity
    }
    // server code
    try {
        TcpSocket serverSocket;
        TcpSocket& toClient = bindListenAccept(LOCALPORT, serverSocket);
        cout << "  Server closing connection to client\n";
        toClient.close();
        cout << "  Server closing listening socket\n";
        serverSocket.close();
        checkChild(child);
        cout << "Done!\n";
        return true;
    } catch (TcpSocket::SocketException& e) {
        cout << "  Server exception: " << e << endl;
        cout << "Failed!\n";
        return false;
    }
}

bool readOneLineFromClientTest(const int linesize=DEFAULT_MAX_RECV){
    cout << "Starting readOneLineFromClientTest() ...\n";

    string message("Passaro verde abandona ninho. Escuto\n");
    pid_t child;
    // client code
    if ((child=fork())==0){
        try {
            TcpSocket toServer;
            cout << "  Forked child connecting to server\n";
            toServer.connect("localhost",LOCALPORT);
            cout << "  Forked child connected: " << toServer << endl;
            cout << "  Forked child sending to server\n";
            // toServer.write(message);
            toServer << message;
            cout << "  Forked child closing connection to server\n";
            toServer.close();
            exit(0);
        } catch (TcpSocket::SocketException& e) {
            cout << "  Forked child exception: " << e << endl;
            exit(-1);
        }
        return false; // for clarity
    }

    // server code
    try {
        TcpSocket& connectedSocket = bindListenAccept(LOCALPORT, *new TcpSocket());
        std::string clientmessage;
        cout << "  Reading one (at most " << linesize << " chars long) line from client\n";
        // connectedSocket.readline(clientmessage, linesize,'\n');
        connectedSocket >> clientmessage;
        if (clientmessage.empty()){
            cout << "  \n...failed!\n";
            return false;
        }
        cout << "  Read: \"" << clientmessage << "\"";
        if (!(clientmessage.compare(message) == 0))
            throw std::runtime_error("Messages dont match");
        checkChild(child);
        cout << "Done!\n";
        return true;
    } catch (TcpSocket::SocketException& e) {
        cout << "  " << e << endl;
        cout << "Failed!\n";
        return false;
    }
}

int main(int argc, char* argv[]){
    cout << "Starting TcpSocket unit tests\n";
    simpleAcceptConnectTest();
    readOneLineFromClientTest();
    cout << "Done with TcpSocket unit tests\n";
}











