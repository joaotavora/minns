#ifndef EXCEPTION_H
#define EXCEPTION_H

#include <iostream>
#include <string>

class Exception {
public:
    Exception(const char* s);
    Exception(const char* s, int err);
    virtual ~Exception();

private:
    std::string& string;
    int errno;

    //TODO: falta aqui o metodo que converte isto em string
};

#endif // EXCEPTION_H







