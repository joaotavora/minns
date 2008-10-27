#ifndef CONNECTION_MANAGER_H
#define CONNECTION_MANAGER_H

#include "ConnectionHandler.h"

class ConnectionManager {
public:

    virtual ~ConnectionManager();

    virtual void start(ConnectionHandler& ch); // start managing connections, handling them with ch
    virtual void stop(); // stop managing connections
};

#endif // CONNECTION_MANAGER_H

