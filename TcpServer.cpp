#include <iostream>

#include "TcpServer.h"

TcpServer::TcpServer(ConnectionManager& cm, ConnectionHandler&ch)
    : connectionManager(cm), connectionHandler (ch) {};

TcpServer::start() {
    cout << "Starting TcpServer\n";
    connectionManager.start(connectionHandler); }

TcpServer::stop() {
    cout << "Would be stopping TcpServer\n";
}

TcpServer::daemonize(){
    cout << "Would be daemonizing TcpServer\n";
}




