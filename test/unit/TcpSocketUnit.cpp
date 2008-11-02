// libc includes

#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>

// stdl includes

#include <iostream>
#include <string>

// Project includes

#include "TcpSocket.h"

// Usings

using namespace std;

const int LOCALPORT = 34343;

TcpSocket* bindListenAccept(int port, TcpSocket& serverSocket){
    serverSocket.bind_any(port);
    serverSocket.listen();

    cout << "  Server socket bound: " << serverSocket << endl
         << "  Server accepting connections\n";
    TcpSocket* connected =  serverSocket.accept();
    cout << "  Connection accepted: " << *connected << endl;
    return connected;
}

void checkChild(pid_t child_pid, bool noerror = false){
    int status;

    if ((child_pid==waitpid(child_pid, &status, 0)) and
        WIFEXITED(status) and
        (WEXITSTATUS(status) == 0)){
        return;
    } else
        if (!noerror) throw std::runtime_error("Child process didn't terminate well");
}

// Unit tests

bool simpleAcceptConnectTest(){
    cout << "Starting simpleAcceptConnectTest() ...\n";
    pid_t child;
    // client code
    if ((child=fork())==0){
        try {
            TcpSocket toServer;
            cout << "  Forked child waiting a bit\n";
            sleep(1);
            cout << "  Forked child connecting to server\n";
            toServer.connect("127.0.0.1",LOCALPORT);
            cout << "  Forked child connected: " << toServer << endl;
            cout << "  Forked child closing connection to server\n";
            toServer.close();
            cout << "  Forked child exiting\n";
            exit(0);
        } catch (TcpSocket::SocketException& e) {
            cout << "  Forked child exception: " << e.what() << endl;
            exit(-1);
        }
        return false; // for clarity
    }
    // server code
    TcpSocket* toClient;
    try {
        TcpSocket serverSocket;
        toClient = bindListenAccept(LOCALPORT, serverSocket);
        cout << "  Server closing connection to client\n";
        toClient->close();
        cout << "  Server closing listening socket\n";
        serverSocket.close();
        checkChild(child);
        cout << "Done!\n";
        delete toClient;
        return true;
    } catch (std::exception& e) {
        cout << "  Server exception: " << e.what() << endl;
        cout << "Failed!\n";
        checkChild(child, true);
        delete toClient;
        return false;
    }
}

bool readLinesFromClientTest(const int linesize=TcpSocket::DEFAULT_MAX_MSG){
    cout << "Starting readLinesFromClientTest(" << linesize << ") ...\n";

    string subfirstline1("Passaro verde ");
    string subfirstline2("abandona ninho. \n");
    string line2("Escuto!\n");
    string message(subfirstline1 + subfirstline2 + line2);

    pid_t child;
    // client code
    if ((child=fork())==0){
        try {
            TcpSocket toServer;
            cout << "  Forked child waiting a bit\n";
            sleep(1);
            cout << "  Forked child connecting to server\n";
            toServer.connect("127.0.0.1",LOCALPORT);
            cout << "  Forked child connected: " << toServer << endl;
            cout << "  Forked child sending to server\n";
            toServer.writeline(message);
            cout << "  Forked child closing connection to server\n";
            toServer.close();
            cout << "  Forked child exiting\n";
            exit(0);
        } catch (std::exception& e) {
            cout << "  Forked child exception: " << e.what() << endl;
            exit(-1);
        }
        return false; // for clarity
    }

    // server code
    TcpSocket* connectedSocket;
    try {
        TcpSocket serverSocket;
        connectedSocket = bindListenAccept(LOCALPORT, serverSocket);
        std::string clientmessage;
        cout << "  Reading one (at most " << linesize << " chars long) line from client\n";

        // Read a line
        connectedSocket->setMaxReceive(linesize);
        while ((*connectedSocket) >> clientmessage){
            // cout << "  clientmessage= " << clientmessage << endl;
        }
        if (clientmessage.empty())
            throw std::runtime_error("clinet message is empty");

        cout << "  Read: \"" << clientmessage << "\"\n";
        if (!(clientmessage.compare(message) == 0))
            throw std::runtime_error("Messages dont match");
        checkChild(child);
        serverSocket.close();
        connectedSocket->close();
        cout << "Done!\n";
        delete connectedSocket;
        return true;
    } catch (std::exception& e) {
        cout << "  Server exception: " << e.what() << endl;
        cout << "Failed!\n";
        checkChild(child, true);
        delete connectedSocket;
        return false;
    }
}

int main(int argc, char* argv[]){
    cout << "Starting TcpSocket unit tests\n";
    simpleAcceptConnectTest();
    // readLinesFromClientTest(1);
    // readLinesFromClientTest(5);
    // readLinesFromClientTest(38);
    // readLinesFromClientTest(39);
    // readLinesFromClientTest(512);
    cout << "Done with TcpSocket unit tests\n";
}
