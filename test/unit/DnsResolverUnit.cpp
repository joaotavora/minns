// stdl includes
#include <iostream>
#include <fstream>

// project includes
#include "DnsResolver.h"

// usings
using namespace std;

string& tryResolve(DnsResolver& resolver, const string& what) throw (DnsResolver::ResolveException){
    static string result;

    if (resolver.resolve(what, result) != NULL)
        cout << "  Success! " << what << " resolved to " << result << ", " << resolver << "\n";
    else
        cout << "  Could not resolve " << what << ", " << resolver << "\n";
    return result;
}

int main(int argc, char* argv[]){
    try {
        if (argc < 2){
            cerr << "Usage: DnsResolver cachesize [names_to_resolve]\n";
            return -1;
        }
        int cachesize = strtol(argv[1], NULL, 0);
        if (cachesize == 0){
            cerr << "Invalid cache size\n";
            return -1;
        }

        DnsResolver a("testhosts.txt", cachesize);
        string result;

        for (int i=2; i < argc; i++){
            tryResolve(a, argv[i]);
        }
    } catch (std::exception& e) {
        cerr << "  Exception: " << e.what() << endl;
    }
}

