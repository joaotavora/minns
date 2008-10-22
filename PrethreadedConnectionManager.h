#ifndef PRETHREADED_CONNECTION_MANAGER_H
#define PRETHREADED_CONNECTION_MANAGER_H

#include <sys/types.h>       // For data types
#include <sys/socket.h>      // For socket(), connect(), send(), and recv()
#include <netdb.h>           // For gethostbyname()
#include <arpa/inet.h>       // For inet_addr()
#include <unistd.h>          // For close()
#include <netinet/in.h>
#include <pthread.h>

#include "ConnectionManager.h"

// Based on W. Richard Stevens, "TCP Prethreaded Server, per-Thread accept"
// example of "Unix Network Programming, Volume 1"
class PrethreadedConnectionManager :: ConnectionManager {
public:
    PrethreadedConnectionManager(const int nthreads, const char *host, const char *port);
    PrethreadedConnectionManager(const char *host, const char *port);
    PrethreadedConnectionManager(const char *port);

    void start(ConnectionHandler& ch);
    void stop();

private:
    int nthreads;
    const char *host;
    const char *port;

    int listenfd;
    socklen_t addrlen;

    pthread_mutex_t mlock;
    vector<AcceptThread> threads;

    int tcp_listen();

    class AcceptThread {
    public:
        Thread(int i, ConnectionHandler& handler);

        void run(); // creates the thread
        void stop(); // signals the thread to stop when it sees fit

    private:
        int i;    // thread index
        int tid;  // thread id, different from index, assigned by pthread_create()

        ConnectionHandler& handler;

        void *thread_main(void *arg);
    };
};

#endif // PRETHREADED_CONNECTION_MANAGER_H

