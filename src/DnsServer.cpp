// stdl includes
#include <iostream>
#include <fstream>

// Project includes
#include "DnsServer.h"

//usings
using namespace std;

void initialize(){
    ofstream out("my_err");
    if ( out )
        clog.rdbuf(out.rdbuf());
    else
        cerr << "Error while opening the file" << endl;

    // FIXME: doesn't address the terminate() SIGSEGV problems, this needs flushing
}

int main(){



    return 0;
}
