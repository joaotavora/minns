#ifndef TCP_SERVER_H
#define TCP_SERVER_H

#include "ConnectionManager.h"
#include "ConnectionHandler.h"

class TcpServer {
public:
    TcpServer(ConnectionManager& cm, ConnectionHandler&ch);

    void start();
    void end();
    void deamonize();

private:
    ConnectionManager& connectionManager;
    ConnectionHandler& connectionHandler;
};

#endif // TCP_SERVER_H




