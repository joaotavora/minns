// TODO: FATAL, WARNING, MESSAGE macros

// libc includes
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

// stdl includes
#include <iostream>
#include <sstream>
#include <fstream>
#include <stdexcept>
#include <list>

// project includes
#include "trace.h"
#include "helper.h"
#include "DnsServer.h"

// usings
using namespace std;

// statics
Thread::Semaphore DnsServer::stop_sem;

// init of static consts
const char DnsServer::DEFAULT_HOSTS_FILE[MAX_FILE_NAME] = "/etc/hosts";
const unsigned int DnsServer::DEFAULT_UDP_PORT[3] = {53, 1, 65000};
const unsigned int DnsServer::DEFAULT_TCP_PORT[3] = {53, 1, 65000};
const unsigned int DnsServer::DEFAULT_UDP_WORKERS[3] = {1, 0, 50};
const unsigned int DnsServer::DEFAULT_TCP_WORKERS[3] = {5, 0, 50};
const unsigned int DnsServer::DEFAULT_TCP_TIMEOUT[3] = {2, 0, 300};

// class members definition

DnsServer::DnsServer (
    DnsResolver& resolver,
    const unsigned int udpport = DEFAULT_UDP_PORT[0],
    const unsigned int tcpport = DEFAULT_TCP_PORT[0],
    const unsigned int udpworkers = DEFAULT_UDP_WORKERS[0],
    const unsigned int tcpworkers = DEFAULT_TCP_WORKERS[0],
    const unsigned int tcptimeout = DEFAULT_TCP_TIMEOUT[0])
    throw(std::exception)
    {

        if (udpworkers > 0){
            int on = 1;
            udp_serversocket.setsockopt (SOL_SOCKET, SO_REUSEADDR, (const char*) &on, sizeof (on));
            udp_serversocket.bind_any(udpport);
        }

        if (tcpworkers > 0){
            int on = 1;
            tcp_serversocket.setsockopt (SOL_SOCKET, SO_REUSEADDR, (const char*) &on, sizeof (on));
            tcp_serversocket.bind_any(tcpport);
            tcp_serversocket.listen();
        }

        for (unsigned int i=0; i < udpworkers; i++)
            workers.push_back(new UdpWorker(resolver, udp_serversocket, resolve_mutex));

        for (unsigned int i=0; i < tcpworkers; i++)
            workers.push_back(new TcpWorker(resolver, tcp_serversocket, accept_mutex, resolve_mutex, tcptimeout));
    }

DnsServer::~DnsServer(){
    for (list<DnsWorker*>::iterator iter = workers.begin(); iter != workers.end(); iter++)
        delete *iter;
}

void DnsServer::start() throw (std::runtime_error){
    cwarning << "installing signal handlers..." << endl;
    signal_helper(SIGTERM, DnsServer::sig_term_handler);
    signal_helper(SIGINT, DnsServer::sig_term_handler);

    list<Thread> threads;
    ctrace << "creating worker threads..." << endl;
    for (list<DnsWorker*>::iterator iter = workers.begin(); iter != workers.end(); iter++){
        Thread t(**iter);
        threads.push_back(t);
    }

    ctrace << "running worker threads..." << endl;
    for (list<Thread>::iterator iter = threads.begin(); iter != threads.end(); iter++)
        iter->run();

    try {
        ctrace << "waiting on exit semaphore..." << endl;
        DnsServer::stop_sem.wait();
    } catch (Thread::ThreadException& e){
        throw std::runtime_error(e.what());
    }
    
        
    ctrace << "signalling stop to all workers" << endl;
    for (list<DnsWorker*>::iterator iter = workers.begin(); iter != workers.end(); iter++){
        (*iter)->stop();
    }

    ctrace << "closing all serversockets" << endl;
    udp_serversocket.close();
    tcp_serversocket.close();

    ctrace << "signalling all remaining workers with SIGALRM" << endl;
    for (list<Thread>::iterator iter = threads.begin(); iter != threads.end(); iter++){
        try {
            iter->kill(SIGALRM);
        } catch (Thread::ThreadException& e) {
            if (e.what_errno() ==  Thread::ThreadException::ESRCH_error)
                cwarning << "\t(tid = 0x" << hex << iter->getTid() << " had already died anyway)" << endl;
            else throw e;
        }
    }

    ctrace << "waiting for worker threads to finish..." << endl;
    for (list<Thread>::iterator iter = threads.begin(); iter != threads.end(); iter++)
    {
        int retval = 0;
        iter->join(&retval);
        ctrace << "\t(thread tid =0x" << iter->getTid() << " finished with retval " << retval << ")" << endl;
    }

    ctrace << "all threads joined" << endl;
    
    for (list<DnsWorker*>::iterator iter = workers.begin(); iter != workers.end(); iter++){
        cout << "\t\t" << (*iter)->report() << endl;
    }
}

// SIGTERM and SIGINT signal handlers
void DnsServer::sig_term_handler(int signo){
    DnsServer::stop_sem.post();
    return;
}

    




