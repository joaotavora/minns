#include <iostream>

#include "TcpServer.h"

using namespace std;

TcpServer::TcpServer(ConnectionManager& cm, ConnectionHandler&ch)
    : connectionManager(cm), connectionHandler (ch) {}

void TcpServer::start() {
    cout << "Starting TcpServer\n";
    connectionManager.start(connectionHandler);
}

void TcpServer::stop() {
    cout << "Would be stopping TcpServer\n";
}

void TcpServer::daemonize(){
    cout << "Would be daemonizing TcpServer\n";
}




