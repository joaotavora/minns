#ifndef SOCKET_H
#define SOCKET_H

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

class Socket {
public:
    // Socket exception
    class SocketException : public std::runtime_error {
    public:
        SocketException(int i, const char* s);
        SocketException(const char* s);
        const char * what() const throw();
        friend std::ostream& operator<<(std::ostream& os, const SocketException& e);
    private:
        int errno_number;
        static const size_t MAXERRNOMSG=200;
    };

// Socket address
    class SocketAddress {
    public:
        SocketAddress() throw ();
        SocketAddress(sockaddr_in& address, socklen_t len) throw ();
        SocketAddress(const char* hostname, const int port) throw (SocketException);
        ~SocketAddress();

        sockaddr_in sockaddr;
        socklen_t socklen;

    private:
        friend std::ostream& operator<<(std::ostream& os, const SocketAddress& address);
    };

    // Common to Tcp and Udp Sockets
    void bind_any (const int port) throw (SocketException);
    void close() throw (SocketException);

protected:
    // File descriptior and address
    const int   sockfd;
    SocketAddress address;

    // protected virtual constructor and destructor
    Socket(int sock) throw ();
    Socket(int sock, SocketAddress& a) throw ();
    virtual ~Socket() = 0;

    // Printing
    friend std::ostream& operator<<(std::ostream& os, const Socket& sock);

private:
    // Private copy constructor defined to do nothing
    Socket(const Socket& src);
    // Status
    bool closed;

};

#endif // SOCKET_H
