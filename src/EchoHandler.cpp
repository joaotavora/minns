#include "EchoHandler.h"

void EchoHandler::handle(TcpSocket& connected){
    string in;
    cout << "  Starting EchoHandler::handle()..." << endl;
    while(1){
        cout << "  Reading one line from " << connected << endl;
        connected >> in;
        cout << "  Echoing back " << in << " to " << connected << endl;
        connected << in;
    }
}



