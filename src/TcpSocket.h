#ifndef TCP_SOCKET_H
#define TCP_SOCKET_H

// stdl includes

#include <string>
#include <stdexcept>

// libc includes

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>

class TcpSocket{
public:
    // Constants
    static const int MAXHOSTNAME = 200;
    static const int DEFAULT_MAX_RECV = 512;
    static const int DEFAULT_MAX_CONNECTIONS = 5;

    // Socket exception
    class SocketException : public std::runtime_error {
    public:
        SocketException(int i, const char* s);
        SocketException(const char* s);
        friend std::ostream& operator<<(std::ostream& os, const SocketException& e);
    private:
        int errno_number;
        static const ssize_t MAXERRNOMSG=200;
    };

    // Client initialization
    void connect (const std::string host, const int port) throw (SocketException);

    // Server initialization
    void bind (const int port) throw (SocketException);
    void listen() throw (SocketException);
    TcpSocket& accept() const throw (SocketException);

    // Common to client and server sockets
    void close() throw (SocketException);

    // Data Transimission
    std::string::size_type readline(std::string& result, const char delimiter = '\n') throw (SocketException);
    void write (const std::string) const throw (SocketException);
    void setMaxReceive(std::string::size_type howmany);
    std::string::size_type getMaxReceive() const;

    // Constructor and destructor
    TcpSocket() throw (SocketException);
    virtual ~TcpSocket();

private:

    // File description and address (server or client)
    sockaddr_in sockaddr;
    const socklen_t   socklen;
    const int          sockfd;

    // Listen queue
    const int max_connections;

    // Status
    class Status {
    public:
        enum status_t {FRESH=0, BOUND, LISTENING, CONNECTED, CLOSED} id;

        Status(enum status_t s) : id(s) {}
        // Check status
        bool fresh() const;
        bool bound() const;
        bool listening() const;
        bool connected() const;
        bool closed() const;

        const std::string& printStatus() const;
        void setStatus (Status::status_t st);
    };
    Status status;

    // Printing

    friend std::ostream& operator<<(std::ostream& os, const TcpSocket& sock);

    // Data Transmission
    std::string read_buffer;
    std::string::size_type max_receive;

    // Cleanup wrapper
    void cleanup();

    // Private constructor for new accept() sockets
    TcpSocket(int fd, sockaddr_in& addr, socklen_t& len,Status::status_t status);

    // Private copy constructor defined to do nothing
    TcpSocket(const TcpSocket& src);

};

const TcpSocket& operator<<(const TcpSocket& ts, const std::string& s) throw (TcpSocket::SocketException);
bool operator>>(TcpSocket& ts, std::string& towriteto) throw (TcpSocket::SocketException);

#endif // SOCKET_H
