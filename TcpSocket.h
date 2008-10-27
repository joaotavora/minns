#ifndef TCP_SOCKET_H
#define TCP_SOCKET_H

#include <string>
#include <stdexcept>

const int MAXHOSTNAME = 200;
const int DEFAULT_MAX_RECV = 512;
const int DEFAULT_MAX_CONNECTIONS = 5;

class TcpSocket{
public:
    TcpSocket();
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
    std::string::size_type readline(
        std::string& result,
        const std::string::size_type howmany = DEFAULT_MAX_RECV,
        const char delimiter = '\n');
    void write (const std::string) const;

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

    // Cleanup wrapper
    void cleanup();

    // Private constructor for new accept() sockets
    TcpSocket(int fd, sockaddr_in& addr, socklen_t& len,Status::status_t status);

    // Private copy constructor defined to do nothing
    TcpSocket(const TcpSocket& src);

};

const TcpSocket& operator<<(const TcpSocket& ts, const std::string& s);
bool operator>>(TcpSocket& ts, std::string& towriteto);

#endif // SOCKET_H
