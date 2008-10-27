#ifndef CONNECTION_HANDLER_H
#define CONNECTION_HANDLER_H

class ConnectionHandler {
public:
    virtual ~ConnectionHandler();

    virtual void handle(int connfd);
};


#endif // CONNECTION_HANDLER_H
