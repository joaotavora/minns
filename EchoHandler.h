#ifndef ECHO_HANDLER_H
#define ECHO_HANDLER_H

#include "ConnectionHandler.h"

class EchoHandler : public ConnectionHandler {
public:
    void handle(int connfd);
};

#endif // ECHO_HANDLER_H
