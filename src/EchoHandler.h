#ifndef ECHO_HANDLER_H
#define ECHO_HANDLER_H

class EchoHandler {
public:
    void handle(TcpSocket& connected);
};

#endif // ECHO_HANDLER_H
