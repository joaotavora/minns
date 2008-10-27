#include <iostream>

#include "EchoServer.h"
#include "PrethreadedConnectionManager.h"
#include "EchoHandler.h"

// TEST

using namespace std;

int main(int argc, char* argv[]){
    PrethreadedConnectionManager *manager=NULL;
    EchoHandler handler;

    if (argc == 3)
        manager=new PrethreadedConnectionManager(argv[1], argv[2]);
    else if (argc == 4)
        manager=new PrethreadedConnectionManager((int)strtol(argv[3], (char **)NULL, 10), argv[1], argv[2]);
    else{
        cerr << "usage: " << argv[0] <<" [ <host> ] <port#> <#threads>" << endl;
        exit(-1);
    }

    TcpServer server(*manager,handler);
    server.start();

    return 0;
}
