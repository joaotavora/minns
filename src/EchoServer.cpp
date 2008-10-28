#include "EchoServer.h"

using namespace std;

void EchoServer::handle(TcpSocket& connected) throw (Socket::SocketException) {
    string in;
    cout << "  Starting EchoHandler::handle()..." << endl;
    while(1){
        cout << "  Reading one line from " << connected << endl;
        connected >> in;
        cout << "  Echoing back " << in << " to " << connected << endl;
        connected << in;
    }
}

void EchoServer::start(){
    TcpSocket serv;
    cout << "Server starting..." << endl;
    try {
        serv.bind_any(34343);
        serv.listen();
    } catch (Socket::SocketException& e) {
        cerr << "Server caught serious exception: " << e.what() << endl;
        exit(-1);
    }
    while (1){
        TcpSocket* connected;
        try {
            connected = serv.accept();
            handle(*connected);
        } catch (Socket::SocketException& e) {
            cerr << "  Caught exception: " << e.what() << endl;
            cout << "  Trying another accept." << endl;
            connected->close();
            delete connected;
        }
    }
}

int main(){
    EchoServer e;

    e.start();
}
