// libc includes

#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <string.h>

// stdl includes

#include <iostream>
#include <string>

// Project includes

#include "UdpSocket.h"

using namespace std;

const int LOCALPORT = 34343;

void checkChild(pid_t child_pid){
    int status;

    if ((child_pid==waitpid(child_pid, &status, 0)) and
        WIFEXITED(status) and
        (WEXITSTATUS(status) == 0)){
        return;
    } else
        throw std::runtime_error("Child process didn't terminate well");
}

// Unit tests

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
    if ((child = fork()) == 0){
        try {
            sleep(1); // HACK! give time for server to setup first
            cout << "  Forked child creating UdpSocket\n";
            UdpSocket clientSocket;
            cout << "  Forked child creating SocketAddress\n";
            Socket::SocketAddress toServer("127.0.0.1",LOCALPORT);
            cout << "  Forked child sending object" << tosend << endl;
            int len = clientSocket.sendto(reinterpret_cast<char *>(&tosend), toServer, sizeof(Object));
            cout << "  Forked child wrote " << len << " bytes to " << toServer << endl;
            cout << "  Forked child closing UdpSocket\n";
            clientSocket.close();
            cout << "  Forked child exiting\n";
            exit(0);
        } catch (std::exception& e) {
            cout << "  Forked child exception: " << e.what() << endl;
            exit(-1);
        }
        return false; // for clarity
    }
    // server code
    try {
        cout << "  Server creating UdpSocket\n";
        UdpSocket serverSocket;
        cout << "  Server creating SocketAddress\n";
        Socket::SocketAddress fromClient;
        cout << "  Server binding SocketAddress\n";
        serverSocket.bind_any(LOCALPORT);
        cout << "  Server recvfrom()\n";

        Object received;
        int len = serverSocket.recvfrom(reinterpret_cast<char *>(&received), fromClient, sizeof(Object));
        cout << "  Read: " << len << " bytes from " << fromClient << endl;
        if (!(tosend.compare(received) == 0))
            throw std::runtime_error("objects dont match");
        cout << "  Server read " << received << endl;
        cout << "  Server waiting for child to return\n";
        checkChild(child);
        cout << "  Server closing UdpSocket\n";
        serverSocket.close();
        cout << "Done!\n";
        return true;
    } catch (std::exception& e) {
        cout << "  Server exception" << e.what() << endl;
        cout << "Failed!\n";
        return false;
    }
}


bool sendReceiveStringTest(){
    cout << "sendReceiveStringTest() ...\n";
    string message("Passaro verde abandona ninho. Escuto\n");

    pid_t child;
    // client code
    if ((child = fork()) == 0){
        try {
            sleep(1); // HACK! give time for server to setup first
            cout << "  Forked child creating UdpSocket\n";
            UdpSocket clientSocket;
            cout << "  Forked child creating SocketAddress\n";
            Socket::SocketAddress toServer("127.0.0.1",LOCALPORT);
            cout << "  Forked child sending test message to" << toServer << endl;
            clientSocket.sendto(message, toServer);
            cout << "  Forked child closing UdpSocket\n";
            clientSocket.close();
            cout << "  Forked child exiting\n";
            exit(0);
        } catch (std::exception& e) {
            cout << "  Forked child exception: " << e.what() << endl;
            exit(-1);
        }
        return false; // for clarity
    }
    // server code
    try {
        cout << "  Server creating UdpSocket\n";
        UdpSocket serverSocket;
        cout << "  Server creating SocketAddress\n";
        Socket::SocketAddress fromClient;
        cout << "  Server binding SocketAddress\n";
        serverSocket.bind_any(LOCALPORT);
        cout << "  Server recvfrom()\n";
        string& received = serverSocket.recvfrom(fromClient);
        cout << "  Read: " << received << endl;
        cout << "  Addr: " << fromClient << endl;
        if (!(message.compare(received) == 0))
            throw std::runtime_error("Messages dont match");
        cout << "  Server waiting for child to return\n";
        checkChild(child);
        cout << "  Server closing UdpSocket\n";
        serverSocket.close();
        cout << "Done!\n";
        return true;
    } catch (std::exception& e) {
        cout << "  Server exception" << e.what() << endl;
        cout << "Failed!\n";
        return false;
    }
}

int main(int argc, char* argv[]){
    cout << "Starting UdpSocket unit tests\n";

    sendReceiveStringTest();
    sendReceiveObjectTest();

    cout << "Done with UdpSocket unit tests\n";
}
