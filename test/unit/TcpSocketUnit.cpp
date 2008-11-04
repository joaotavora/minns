// libc includes

#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <string.h>

// stdl includes

#include <iostream>
#include <string>

// Project includes

#include "TcpSocket.h"

// Usings

using namespace std;

const int LOCALPORT = 34343;

TcpSocket* bindListenAccept(int port, TcpSocket& serverSocket){
    int on;
    serverSocket.setsockopt (SOL_SOCKET, SO_REUSEADDR, (const char*) &on, sizeof (on));
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
    string message("Passaro verde abandona ninho. \nEscuto!\n");

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
        while ((*connectedSocket) >> clientmessage);
        if (clientmessage.empty())
            throw std::runtime_error("clinet message is empty");

        cout << "  Read: \"" << clientmessage << "\"\n";
        if (!(clientmessage.compare(message) == 0)){
            
            throw std::runtime_error("Messages dont match");
        }
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

class Object {
public:
    Object(char cc, int ii, bool bb, const char* sstring)
        : c(cc), i(ii), b(bb) {
        strncpy(string, sstring, 100);
    }
    Object(){
    }

    char c;
    int i;
    bool b;
    char string[100];

    int compare(Object& o){
        if ((o.c == c) and
            (o.i == i) and
            (o.b == b) and
            (strncmp(string, o.string, 100) == 0)) return 0; else return -1;
    }
};

std::ostream& operator<<(std::ostream& os, const Object& object){
    return os << "[Object: "
              << "c=\'" << object.c << "\' "
              << "i=\'" << object.i << "\' "
              << "b=\'" << object.b << "\' "
              << "string=\'" << object.string << "\']";
}

bool sendReceiveObjectTest(){
    cout << "sendReceiveObjectTest() ...\n";

    Object tosend('c', 34, true, "cinco");

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
            toServer.write(reinterpret_cast<char *>(&tosend), sizeof(Object));
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
        cout << "  Reading one object from client\n";

        // Read the object
        Object received;
        int len = connectedSocket->read(reinterpret_cast<char *>(&received), sizeof(Object));
        cout << "  Read: " << len << " bytes from " << *connectedSocket << endl;
        if (!(tosend.compare(received) == 0))
            throw std::runtime_error("objects dont match");
        cout << "  Server read " << received << endl;
        
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
    int retval = false;
    cout << "Starting TcpSocket unit tests\n";
    retval= simpleAcceptConnectTest()
        and sendReceiveObjectTest()
        and readLinesFromClientTest(1)
        and readLinesFromClientTest(5)
        and readLinesFromClientTest(38)
        and readLinesFromClientTest(39)
        and readLinesFromClientTest(512);
    cout << "Done with TcpSocket unit tests\n";
    if (retval)
        exit (0);
    else
        exit (-1);
}
