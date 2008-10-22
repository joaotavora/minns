#ifndef ACCEPTTHREAD_H
#define ACCEPTTHREAD_H

class AcceptThread {
public:
    Thread(int i);
    Thread(int i, ConnectionHandler ch);

    void run(); // creates the thread
    void stop(); // signals the thread to stop when it sees fir

private:
    int i;    // thread index
    int tid;  // thread id, different from index, assigned by pthread_create()

    ConnectionHandler& handler;

    // FIXME: web_child() for preliminary testing only!!!
    void web_child(int connfd);
}

#endif // ACCEPTTHREAD_H



