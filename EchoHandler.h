#ifndef ECHO_HANDLER_H
#define ECHO_HANDLER_H

class EchoHandler : public ConnectionHandler {
public:
    void handle(int connfd);
};

#endif ECHO_HANDLER_H
