#ifndef TCP_SOCKET_H
#define TCP_SOCKET_H

#include <string>

const int MAXHOSTNAME = 200;
const int DEFAULT_MAX_RECV = 512;
const int DEFAULT_MAX_CONNECTIONS = 5;

class TcpSocket{
public:
    TcpSocket();
    TcpSocket(int fd, sockaddr_in& addr, socklen_t& len);
    virtual ~TcpSocket();

    // Client initialization
    void connect (const std::string host, const int port);

    // Server initialization
    void bind (const int port);
    void listen();
    TcpSocket& accept() const;

    // Common to client and server sockets
    void close();

    // Data Transimission
    std::string::size_type readline(std::string& result, const int howmany = DEFAULT_MAX_RECV) const;
    void write (const std::string) const;

    // Check status
    bool fresh() const;
    bool bound() const;
    bool listening() const;
    bool connected() const;
    bool closed() const;

private:

    // Private constructor for new accept() sockets
    TcpSocket(int sockfd, int addr_length, TcpSocket& listening);

    // File description and address (server or client)
    sockaddr_in& sockaddr;
    const socklen_t&   socklen;
    const int          sockfd;

    // Listen queue
    const int max_connections;

    // Status
    enum status_t {FRESH=0, BOUND, LISTENING, CONNECTED, CLOSED} status;
    enum status_t getStatus();
    void setStatus (enum status_t status);

    // Printing
    const std::string& printStatus() const;
    friend std::ostream& operator<<(std::ostream& os, const TcpSocket& sock);

    // Data Transmission
    std::string& read_buffer;
};

#endif // SOCKET_H
