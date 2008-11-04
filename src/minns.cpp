// libc includes
#include <string.h>
#include <stdlib.h>

// stdl includes
#include <iostream>
#include <sstream>
#include <fstream>
#include <stdexcept>
#include <list>

// project includes
#include "helper.h"
#include "trace.h"
#include "DnsServer.h"

// usings
using namespace std;

void print_usage(char* program){
    cout << "Usage: " << program << " [options]" << endl;
    cout << endl;
    cout << " Resolver options" << endl;
    cout << "     -f FILENAME      use hosts file FILE (default is " << DnsServer::DEFAULT_HOSTS_FILE[0] << ")" << endl;
    cout << "     -c CACHESIZE     set resolver cache size to CACHESIZE (default is " << DnsResolver::DEFAULT_CACHE_SIZE[0] << ")" << endl;
    cout << "     -m MAXALIASES    maximum MAXALIASES aliases per entry (default is " << DnsResolver::DEFAULT_MAX_ALIASES[0] << ")" << endl;
    cout << "     -i MAXIALIASES   maximum MAXIALIASES addresses per alias (default is " << DnsResolver::DEFAULT_MAX_INVERSE_ALIASES[0] << ")" << endl;
    cout << endl;
    cout << " Network options" << endl;
    cout << "     -t TCPPORT       use TCP port TCPPORT (default is " << DnsServer::DEFAULT_TCP_PORT[0] << ")" << endl;
    cout << "     -u UDPPORT       use UDP port UDPPORT (default is " << DnsServer::DEFAULT_UDP_PORT[0] << ")" << endl;
    cout << "     -p TCPWORKERS    use TCPWORKERS threads for TCP connections (default is " << DnsServer::DEFAULT_TCP_WORKERS[0] << ")" << endl;
    cout << "     -d UDPWORKERS    use UDPWORKERS threads for UDP connections (default is " << DnsServer::DEFAULT_UDP_WORKERS[0] << ")" << endl;
    cout << "     -o TIMEOUT       timeout TCP connections in TIMEOUT seconds (default is " << DnsServer::DEFAULT_TCP_TIMEOUT[0] << ")" << endl;
    cout << endl;
    cout << "Read README file for some (not many) details" << endl;
}

int main(int argc, char* argv[]){
    try {
        // DnsResolver options
        char cachefile[DnsServer::MAX_FILE_NAME];
        strncpy (cachefile,DnsServer::DEFAULT_HOSTS_FILE, DnsServer::MAX_FILE_NAME);

        unsigned int cachesize = DnsResolver::DEFAULT_CACHE_SIZE[0]; // c
        unsigned int maxaliases = DnsResolver::DEFAULT_MAX_ALIASES[0]; //m
        unsigned int maxinversealiases = DnsResolver::DEFAULT_MAX_INVERSE_ALIASES[0]; //i

        // DnsServer options
        unsigned int udpthreads = DnsServer::DEFAULT_UDP_WORKERS[0]; // d
        unsigned int tcpthreads = DnsServer::DEFAULT_TCP_WORKERS[0]; // p

        // network options
        uint16_t tcpport = DnsServer::DEFAULT_TCP_PORT[0]; // t
        uint16_t udpport = DnsServer::DEFAULT_UDP_PORT[0];  // u
        unsigned int tcptimeout = DnsServer::DEFAULT_TCP_TIMEOUT[0]; // o

        char opt;
        while ((opt = getopt(argc, argv, "hf:c:m:i:d:p:t:u:")) != -1) {
            stringstream ss;
            try {
                switch (opt) {
                case 'h':
                    print_usage(argv[0]);
                    exit(0);
                case 'f':
                    if (strlen(optarg) < DnsServer::MAX_FILE_NAME)
                        strncpy(cachefile, optarg, DnsServer::MAX_FILE_NAME);
                    else
                        throw std::runtime_error("File name too long");
                    break;
                case 'c':
                    cachesize = strtol_helper('c',optarg,&DnsResolver::DEFAULT_CACHE_SIZE[1]);
                    break;
                case 'm':
                    maxaliases = strtol_helper('m',optarg,&DnsResolver::DEFAULT_MAX_ALIASES[1]);
                    break;
                case 'i':
                    maxinversealiases = strtol_helper('i',optarg,&DnsResolver::DEFAULT_MAX_INVERSE_ALIASES[1]);
                    break;
                case 'd':
                    udpthreads = strtol_helper('d',optarg, &DnsServer::DEFAULT_UDP_WORKERS[1]); break;
                case 'p':
                    tcpthreads = strtol_helper('p',optarg, &DnsServer::DEFAULT_TCP_WORKERS[1]); break;
                case 'u':
                    udpport = strtol_helper('u',optarg, &DnsServer::DEFAULT_UDP_PORT[1]); break;
                case 't':
                    tcpport = strtol_helper('t',optarg, &DnsServer::DEFAULT_TCP_PORT[1]); break;
                case 'o':
                    tcptimeout = strtol_helper('o',optarg, &DnsServer::DEFAULT_TCP_TIMEOUT[1]); break;
                default: // ?
                    ss << "Unknown option character \'" << (char)optopt << "\'. Ignoring...";
                    throw std::runtime_error(ss.str().c_str());
                    break;
                }
            } catch (std::runtime_error& e) {
                cwarning << argv[0]<< ": " << e.what() << endl;
            }
        }

        cout << "Printing options:" << endl;
        cout << endl;
        cout << " Resolver options" << endl;
        cout << "     -f FILENAME      use hosts file FILE (using " << cachefile << ")" << endl;
        cout << "     -c CACHESIZE     set resolver cache size to CACHESIZE (using " << cachesize << ")" << endl;
        cout << "     -m MAXALIASES    maximum MAXALIASES aliases per entry (using " << maxaliases << ")" << endl;
        cout << "     -i MAXIALIASES   maximum MAXIALIASES addresses per alias (using " << maxinversealiases << ")" << endl;
        cout << endl;
        cout << " Network options" << endl;
        cout << "     -t TCPPORT       use TCP port TCPPORT (using " << tcpport << ")" << endl;
        cout << "     -u UDPPORT       use UDP port UDPPORT (using " << udpport << ")" << endl;
        cout << "     -p TCPWORKERS    use TCPWORKERS threads for TCP connections (using " << tcpthreads << ")" << endl;
        cout << "     -d UDPWORKERS    use UDPWORKERS threads for UDP connections (using " << udpthreads << ")" << endl;
        cout << "     -o TIMEOUT       timeout TCP connections in TIMEOUT seconds (using " << tcptimeout << ")" << endl;
        cout << endl;


        DnsResolver r(cachefile, cachesize, maxaliases, maxinversealiases);
        DnsServer a(r, udpport, tcpport, udpthreads, tcpthreads, tcptimeout);
        a.start();
        return 0;
    } catch (std::exception& e) {
        cfatal << "Exception: " << e.what() << endl;
        return -1;
    }
}
