// libc includes
#include <string.h>

// stdl includes
#include <iostream>
#include <fstream>
#include <stdexcept>
#include <vector>

// project includes
#include "DnsServer.h"

// usings
using namespace std;

// class members definition

DnsServer::DnsServer (
    DnsResolver& resolver,
    const unsigned int udpport = DEFAULT_UDP_PORT,
    const unsigned int tcpport = DEFAULT_TCP_PORT,
    const unsigned int udpworkers = DEFAULT_UDP_WORKERS,
    const unsigned int tcpworkers = DEFAULT_TCP_WORKERS )  throw(std::exception)
    {
        for (unsigned int i=0; i < udpworkers; i++)
            workers.push_back(new UdpWorker(udpport, resolver));

        for (unsigned int i=0; i < tcpworkers; i++)
            workers.push_back(new TcpWorker(tcpport, resolver));
    }

DnsServer::~DnsServer(){
}

void DnsServer::start(){
    
}

// unit tests

void initialize(){
    // primitive log
    ofstream out("my_err");
    if ( out )
        clog.rdbuf(out.rdbuf());
    else
        cerr << "Error while opening the file" << endl;

    // FIXME: doesn't address the terminate() SIGSEGV problems, this needs
    // flushing on atexit()
}

int main(){
    try {
        DnsResolver r("simplehosts.txt", DnsResolver::DEFAULT_CACHE_SIZE, DnsResolver::DEFAULT_MAX_ALIASES);
        DnsServer a(r, 43434, 53535, 1, 0);
        a.start();
        return 0;
    } catch (std::exception& e) {
        cerr << "Exception: " << e.what() << endl;
        return -1;
    }
}
