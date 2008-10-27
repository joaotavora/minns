#include <iostream> // cout
#include <vector> // vector
#include <cmath> // fmax

// FIXME: remove this dependency. Make classes
#include "PrethreadedConnectionManager.h"

using namespace std;

const int listenq_length = 10;
const int default_nthreads = 5;

PrethreadedConnectionManager::PrethreadedConnectionManager(const char *p){
    host="";
    port=p;
    PrethreadedConnectionManager(default_nthreads,"",p);
}

PrethreadedConnectionManager::PrethreadedConnectionManager(const char *h, const char *p){
    PrethreadedConnectionManager(default_nthreads,h,p);
}

PrethreadedConnectionManager::PrethreadedConnectionManager(const int nt, const char *h, const char *p){
    host=h;
    port=p;
    nthreads=fmax(nt, 1);
    for (int i = 0; i < nthreads; i++)
        threads.push_back(new AcceptThread(i));
}

void PrethreadedConnectionManager::start(ConnectionHandler& ch){
    cout << "Starting PrethreadedConnectionManager\n";
    for (iterator iter = threads.begin(); iter != threads.end(); iter++)
        *iter.run();
}

void PrethreadedConnectionManager::stop(){
    cout << "Would be stoping PrethreadedConnectionManager\n";
}

/* Using the first solution described in http://www.tuxtips.org/?p=5

   See also:

   http://bytes.com/forum/thread141281.html
   http://www.mobydisk.com/softdev/techinfo/pthreads_tutorial/
   http://root.cern.ch/root/roottalk/roottalk99/2241.html */

PrethreadedConnectionManager::AcceptThread::AcceptThread(int ii, ConnectionHandler& ch, PrethreadedConnectionManager& pcm)
    : i(ii), handler(ch), manager(pcm){}

void* PrethreadedConnectionManager::AcceptThread::sThreadHelper (void* args){
    return ((AcceptThread*) args)->thread_main();
}

void PrethreadedConnectionManager::AcceptThread::run(){
    Pthread_create(&tid, NULL, &(PrethreadedConnectionManager::AcceptThread::sThreadHelper), (void *) this);
}

void *PrethreadedConnectionManager::AcceptThread::thread_main(){
    int connfd;
    struct sockaddr *cliaddr; // current client's address
    socklen_t clilen; // current client's address length

    if ((cliaddr = (struct sockaddr*) malloc(manager.addrlen))==NULL){
        err_sys("malloc() error creating cliaddr");
    }

    cout << "Thread " << i << "starting\n";
    while(1){
        clilen = manager.addrlen;
        Pthread_mutex_lock(&manager.mlock);
        connfd = Accept(manager.listenfd, cliaddr, &clilen);
        Pthread_mutex_unlock(&manager.mlock);
        handledConnections++;

        handler.handle(connfd);
        Close(connfd);
    }
}

// TEST CODE









