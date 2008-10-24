#ifndef EXCEPTION_H
#define EXCEPTION_H

#include <iostream>
#include <string>

class Exception {
public:
    Exception(const char* s);
    Exception(const char* s, int err);
    virtual ~Exception();

protected:
    const std::string& message;
    const int errno;

    //TODO: falta aqui o metodo que converte isto em string
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







