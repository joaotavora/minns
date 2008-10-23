#ifndef TCP_SOCKET_H
#define TCP_SOCKET_H

#include <string>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>

const int MAXHOSTNAME = 200;
const int MAXCONNECTIONS = 5;
const int MAXRECV = 500;

class TcpSocket{
public:
    TcpSocket();
    virtual ~TcpSocket();

    // Client initialization
    void connect (const std::string host, const int port);

    // Server initialization
    void bind (const int port);
    void listen() const;
    TcpSocket& accept() const;

    // Check status
    bool connected();
    bool bound();
    bool listening();


private:

    // Private constructor for new accept() sockets
    TcpSocket(int sockfd, int addr_length, TcpSocket& listening);

    // Data Transimission
    bool send ( const std::string ) const;
    int recv ( std::string& ) const;

    // File description and address (server or client)
    int          sockfd;
    sockaddr_in& sockaddr;

    // Status
    enum status_t {BOUND=1, LISTENING, CONNECTED} status;
    enum status_t getStatus();
    void setStatus (enum status_t status);
};

#endif // SOCKET_H
