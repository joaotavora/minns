#include <pthread.h>

#include "Common.h"
#include "AcceptThread.h"



AcceptThread::run(){

    int n;

    if ((n = pthread_create(&tid, NULL, &handle_fn, (void *) i)) != 0){
        errno = n;
        err_sys("pthread_create error");
    } else return;
}


int connfd;
    struct sockaddr *cliaddr; // current client's address
    socklen_t clilen; // current client's address length
