// libc includes
#include <string.h>

// stdl includes
#include <iostream>
#include <fstream>
#include <stdexcept>
#include <list>

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
    const unsigned int tcpworkers = DEFAULT_TCP_WORKERS )
    throw(std::exception)
    {

        if (udpworkers > 0){
            udp_serversocket.bind_any(udpport);
        }

        if (tcpworkers > 0){
            tcp_serversocket.bind_any(tcpport);
            tcp_serversocket.listen();
        }

        for (unsigned int i=0; i < udpworkers; i++)
            workers.push_back(new UdpWorker(resolver, udp_serversocket));

        for (unsigned int i=0; i < tcpworkers; i++)
            workers.push_back(new TcpWorker(resolver, tcp_serversocket, acceptMutex));
    }

DnsServer::~DnsServer(){
    for (list<DnsWorker*>::iterator iter = workers.begin(); iter != workers.end(); iter++)
        delete *iter;
}

void DnsServer::start(){
    list<Thread> threads;

    cerr << "DnsServer creating worker threads..." << endl;
    for (list<DnsWorker*>::iterator iter = workers.begin(); iter != workers.end(); iter++){
        Thread t(**iter);
        threads.push_back(t);
    }

    cerr << "DnsServer running worker threads..." << endl;
    for (list<Thread>::iterator iter = threads.begin(); iter != threads.end(); iter++)
        iter->run();

    cerr << "DnsServer waiting for worker threads to finish..." << endl;
    for (list<Thread>::iterator iter = threads.begin(); iter != threads.end(); iter++)
    {
        int retval;
        iter->join(&retval);
        cerr << "   DnsServer: Worker thread tid = " << iter->getTid() << " finished with retval " << retval << endl;
    }


    cerr << "DnsServer reports all threads joined" << endl;
}

// unit tests

void initialize(){
    // primitive log
    ofstream out("my_err");
    if ( out )
        clog.rdbuf(out.rdbuf());
    else
        cerr << "Error while opening the file" << endl;

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
