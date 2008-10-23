#include <string>

#include "Exception.h"

using namespace std;

Exception::Exception(const char* s)
    : string(s) , errno(0);

Exception::Exception(const char* s, int err)
    : string(s) , errno(err);

virtual Exception::~Exception(){}


