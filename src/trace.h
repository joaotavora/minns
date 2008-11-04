#ifndef TRACE_H
#define TRACE_H

#include <iostream>

#define cmessage cout << __FILE__ << ":" << __LINE__ << ": "

#define ctrace clog << __FILE__ << ":" << __LINE__ << ": Info: "
#define cwarning clog << __FILE__ << ":" << __LINE__ << ": Warning: "
#define cerror clog << __FILE__ << ":" << __LINE__ << ": Error: "
#define cfatal cout << "Fatal error\n"; clog << __FILE__ << ":" << __LINE__ << ": Fatal: "

#define TRACELINE(string) __FILE__ "@" XSTR(__LINE__) ": " string

#define STR(x)   #x
#define XSTR(x)  STR(x)

#endif // TRACE_H 
