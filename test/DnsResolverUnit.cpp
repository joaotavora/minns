// libc includes
#include <stdlib.h>

// stdl includes
#include <iostream>
#include <fstream>
#include <sstream>

// project includes
#include "DnsResolver.h"

// usings
using namespace std;


bool tryToResolveTest(int cachesize, int maxaliases, int maxinverseliases, const char* what){
    try {
        cout << "Starting tryToResolveTest() for " << what << " and (" <<
        cachesize << ", " << maxaliases << ", " << maxinverseliases << ")" << endl;
        DnsResolver resolver("../test/simplehosts.txt", cachesize, maxaliases, maxinverseliases);
        stringstream ss(what);
        string tosearch;
        while (ss >> tosearch){
            try {
                string& result = resolver.resolve_to_string(tosearch);
                cout << "  Resolved " << tosearch << " to " << result << endl;
            } catch (DnsResolver::ResolveException& e) {
                cout << "  Failed to resolve :" << e.what() << endl;
            }
            cout << "  " << resolver << endl;
        }
        return true;
    } catch (std::exception& e) {
        cout << "  Exception: " << e.what() << endl;
        cout << "Failed: " << e.what() << endl;
        return false;
    }
}

int main(int argc, char* argv[]){
    cout << "Starting TcpSocket unit tests\n";
    tryToResolveTest(10,10,2, "bla ble");
    tryToResolveTest(2,10,2, "bla ble");
    tryToResolveTest(2,10,2, "bla ble");
    cout << "Done with TcpSocket unit tests\n";
}




