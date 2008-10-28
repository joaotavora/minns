#ifndef ECHO_SERVER_H
#define ECHO_SERVER_H

#include "Socket.h"
#include "TcpSocket.h"
#include "Thread.h"

class EchoServer {
public:
    void start();
    void handle(TcpSocket& connected) throw (Socket::SocketException);
};

#endif // ECHO_SERVER_H
