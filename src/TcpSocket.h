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
    static const int DEFAULT_MAX_MSG = 512;
    static const int DEFAULT_MAX_CONNECTIONS = 5;

    // Client initialization
    void connect (const std::string& host, const int port) throw (SocketException);

    // Server initialization
    void listen(const int max_connections=TcpSocket::DEFAULT_MAX_CONNECTIONS) throw (SocketException);
    TcpSocket* accept() const throw (SocketException);

    // Data Transmission - raw byte functions
    size_t read(char* buff, const size_t howmany) throw (SocketException);
    size_t write(const char* buff, const size_t howmany) throw (SocketException);

    // Data Transmission - string functions
    size_t readline(std::string& result, const char delimiter = '\n', const size_t maxlen = DEFAULT_MAX_MSG) throw (SocketException);
    void writeline (const std::string) const throw (SocketException);

    void setMaxReceive(size_t howmany);
    size_t getMaxReceive() const;

    // Public constructor and destructor
    TcpSocket() throw (SocketException);
    ~TcpSocket();

private:
    // Ownership of connected sockets
    // std::vector<TcpSocket> connected;

    // Data Transmission
    std::string read_buffer;
    size_t maxreceive;

    // Private constructor for new accept() sockets
    TcpSocket(int fd, SocketAddress& addr);

    friend bool operator>>(TcpSocket& ts, std::string& towriteto) throw (Socket::SocketException);
    
};
const TcpSocket& operator<<(const TcpSocket& ts, const std::string& s) throw (Socket::SocketException);
bool operator>>(TcpSocket& ts, std::string& towriteto) throw (Socket::SocketException);

#endif // TCP_SOCKET_H
