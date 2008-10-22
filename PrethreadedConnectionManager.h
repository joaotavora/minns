#ifndef PRETHREADEDCONNECTIONMANAGER_H
#define PRETHREADEDCONNECTIONMANAGER_H

#include <sys/types.h>       // For data types
#include <sys/socket.h>      // For socket(), connect(), send(), and recv()
#include <netdb.h>           // For gethostbyname()
#include <arpa/inet.h>       // For inet_addr()
#include <unistd.h>          // For close()
#include <netinet/in.h>
#include <pthread.h>

// Based on W. Richard Stevens, "TCP Prethreaded Server, per-Thread accept"
// example of "Unix Network Programming, Volume 1"
class PrethreadedConnectionManager {
public:
    PrethreadedConnectionManager(const char *host, const char *port, const int);
    PrethreadedConnectionManager(const char *host, const char *port);
    PrethreadedConnectionManager(const char *port);

    void start();
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



};

#endif // PRETHREADEDCONNECTIONMANAGER_H

