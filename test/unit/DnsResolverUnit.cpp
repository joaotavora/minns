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
        if (argc < 3)
            throw DnsResolver::ResolveException("Usage: DnsResolver file cachesize [names_to_resolve]");

        int cachesize = strtol(argv[2], NULL, 0);
        if (cachesize == 0)
            throw DnsResolver::ResolveException("Usage: Invalid cache size");

        DnsResolver a(argv[1], cachesize);
        string result;

        for (int i=3; i < argc; i++){
            tryResolve(a, argv[i]);
        }
    } catch (std::exception& e) {
        cerr << "  Exception: " << e.what() << endl;
        return -1;
    }
}

