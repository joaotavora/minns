#ifndef TCP_SOCKET_H
#define TCP_SOCKET_H

// stdl includes
#include <vector>

// project includes
#include "Socket.h"

class TcpSocket : public Socket {
public:
    // Constants
    static const int MAXHOSTNAME = 200;
    static const int DEFAULT_MAX_RECV = 512;
    static const int DEFAULT_MAX_CONNECTIONS = 5;

    // Client initialization
    void connect (const std::string& host, const int port) throw (SocketException);

    // Server initialization
    void listen(const int max_connections=TcpSocket::DEFAULT_MAX_CONNECTIONS) throw (SocketException);
    TcpSocket* accept() throw (SocketException);

    // Data Transimission
    std::string::size_type readline(std::string& result, const char delimiter = '\n') throw (SocketException);
    void write (const std::string) const throw (SocketException);
    void setMaxReceive(std::string::size_type howmany);
    std::string::size_type getMaxReceive() const;

    // Public constructor and destructor
    TcpSocket() throw (SocketException);
    virtual ~TcpSocket();

private:
    // Connected sockets
    std::vector<TcpSocket*> connected;

    // Data Transmission
    std::string read_buffer;
    std::string::size_type max_receive;

    // Private constructor for new accept() sockets
    TcpSocket(int fd, SocketAddress& addr);
};
const TcpSocket& operator<<(const TcpSocket& ts, const std::string& s) throw (Socket::SocketException);
bool operator>>(TcpSocket& ts, std::string& towriteto) throw (Socket::SocketException);

#endif // TCP_SOCKET_H
