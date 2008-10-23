#ifndef PRETHREADED_CONNECTION_MANAGER_H
#define PRETHREADED_CONNECTION_MANAGER_H

#include <sys/types.h>       // For data types
#include <sys/socket.h>      // For socket(), connect(), send(), and recv()
#include <netdb.h>           // For gethostbyname()
#include <arpa/inet.h>       // For inet_addr()
#include <unistd.h>          // For close()
#include <netinet/in.h>
#include <pthread.h>

#include <vector>

#include "ConnectionManager.h"

// Based on W. Richard Stevens, "TCP Prethreaded Server, per-Thread accept"
// example of "Unix Network Programming, Volume 1"
class PrethreadedConnectionManager : public ConnectionManager {
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

    class AcceptThread;
    friend class AcceptThread;

    std::vector<AcceptThread> threads;

    class AcceptThread {
    public:
        AcceptThread(int i, ConnectionHandler& handler, PrethreadedConnectionManager& pcm);

        void run(); // creates the thread
        void stop(); // signals the thread to stop when it sees fit

    private:
        int i;    // thread index
        pthread_t tid;  // thread id, different from index, assigned by pthread_create()
        int handledConnections;

        ConnectionHandler& handler;
        PrethreadedConnectionManager& manager;

        static void* sThreadHelper (void* args);
        void *thread_main();
    };
};

#endif // PRETHREADED_CONNECTION_MANAGER_H

