#include <string>

#include "Exception.h"

using namespace std;

// base Exception

Exception::Exception(const char* s)
    : message(*new string(s)) , errno(0){}

Exception::Exception(const char* s, int err)
    : message(*new string(s)) , errno(err){}

Exception::~Exception(){}

// SocketException

SocketException::SocketException(const char* s)
    : Exception(s){}

SocketException::SocketException(const char* s, int err)
    : Exception(s,err){}

// ThreadException

ThreadException::ThreadException(const char* s)
    : Exception(s){}

ThreadException::ThreadException(const char* s, int err)
    : Exception(s,err){}

