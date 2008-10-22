#include <iostream> // cout
#include <cmath> // fmax

#include "wrappers.h"
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
}

void PrethreadedConnectionManager::start(ConnectionHandler& ch){
    cout << "Starting PrethreadedConnectionManager" << endl;
}

void PrethreadedConnectionManager::stop(){
    cout << "Would be stoping PrethreadedConnectionManager" << endl;
}

// TODO: get rid of any C-esque suff here, make this throw exception instead of calling err_quit...
int PrethreadedConnectionManager::tcp_listen(){
    int                 listenfd, n;
    const int           on = 1;
    struct addrinfo     hints, *res, *ressave;

    bzero(&hints, sizeof(struct addrinfo));
    hints.ai_flags = AI_PASSIVE;
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if ( (n = getaddrinfo(host, port, &hints, &res)) != 0)
        err_quit("tcp_listen error for %s:%s (%s)", host, port, gai_strerror(n));
    ressave = res;

    do {
        listenfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
        if (listenfd < 0)
            continue;           /* error, try next one */

        setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
        if (bind(listenfd, res->ai_addr, res->ai_addrlen) == 0)
            break;                      /* success */

        close(listenfd);        /* bind error, close and try next one */
    } while ( (res = res->ai_next) != NULL);

    if (res == NULL)   /* errno from final socket() or bind() */
        err_sys("tcp_listen error for %s:%s (%s)", host, port, gai_strerror(n));

    listen(listenfd, listenq_length);

    // save the size of protocol address before freeing memory allocated within
    // getaddrinfo().
    addrlen = res->ai_addrlen;    /* return size of protocol address */

    freeaddrinfo(ressave);

    return(listenfd);
}

PrethreadedConnectionManager::AcceptThread::AcceptThread(int ii, ConnectionHandler& ch) : i(ii), handler(ch){}

void PrethreadedConnectionManager::AcceptThread::run(){
    int n;

    if ((n = pthread_create_w(&tid, NULL, &thread_main, (void *) i)) != 0){
        errno = n;
        err_sys("pthread_create() error");
    } else return;
}

void *PrethreadedConnectionManager::AcceptThread::thread_main(void *arg){
    int connfd;
    struct sockaddr *cliaddr; // current client's address
    socklen_t clilen; // current client's address length

    if ((cliaddr = (struct sockaddr*) malloc(addrlen))==NULL){
        err_sys("malloc() error creating cliaddr");
    }

    cout << "Thread " << i << "starting" << endl;
    while(1){
        clilen = addrlen;
        pthread_mutex_lock_w(&mlock);
        connfd = accept_w(listenfd, cliaddr, &clilen);
        pthread_mutex_unlock_w(&mlock);
        handledConnections++;

        handler.handle(connfd);
        close_w(connfd);
    }
}

// TEST CODE

int main(int argc, char* argv[]){
    PrethreadedConnectionManager *manager=NULL;

    if (argc == 3)
        manager=new PrethreadedConnectionManager(argv[1], argv[2]);
    else if (argc == 4)
        manager=new PrethreadedConnectionManager(argv[1], argv[2], (int)strtol(argv[3], (char **)NULL, 10));
    else{
        cerr << "usage: serv07 [ <host> ] <port#> <#threads>" << endl;
        exit(-1);
    }
    manager->start();

    return 0;
}







