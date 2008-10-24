#ifndef EXCEPTION_H
#define EXCEPTION_H

const int MAXERRNOMSG=200;

#include <iostream>
#include <string>

class Exception {
public:
    Exception(const char* s);
    Exception(const char* s, int err);
    virtual ~Exception();

private:
    const std::string& message;
    const int errno;

    friend std::ostream& operator<<(std::ostream& os, const Exception& e);
};

class SocketException : public Exception {
public:
    SocketException(const char* s);
    SocketException(const char* s, int err);
};

class ThreadException : public Exception {
public:
    ThreadException(const char* s);
    ThreadException(const char* s, int err);
};


#endif // EXCEPTION_H







