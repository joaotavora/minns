#include <iostream> // cout
#include <cmath> // fmax

#include "Common.h"
#include "PrethreadedConnectionManager.h"

using namespace std;


const int listenq_length = 10;
const int default_nthreads = 5;

PrethreadedConnectionManager::PrethreadedConnectionManager(const char *h, const char *p){
    PrethreadedConnectionManager(h,p,default_nthreads);
}

PrethreadedConnectionManager::PrethreadedConnectionManager(const char *p){
    host="";
    port=p;
    PrethreadedConnectionManager("",p,default_nthreads);
}

PrethreadedConnectionManager::PrethreadedConnectionManager(const char *h, const char *p, const int nt){
    host=h;
    port=p;
    nthreads=fmax(nt, 1);
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

void PrethreadedConnectionManager::start(){
    cout << "Would be starting PrethreadedConnectionManager" << endl;
}


void PrethreadedConnectionManager::stop(){
    cout << "Would be stoping PrethreadedConnectionManager" << endl;
}

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







