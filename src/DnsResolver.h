#ifndef DNS_RESOLVER_H
#define DNS_RESOLVER_H

// stdl includes
#include <iostream>
#include <fstream>
#include <stdexcept>

#include <utility>
#include <list>
#include <vector>
#include <map>

// libc includes
#include <sys/types.h>
#include <arpa/inet.h>

class DnsResolver {
public:

    struct DnsEntry {
        in_addr_t ip;
        std::vector<std::string> aliases;
    };

    // Cache nested class
    class Cache {
        class map_value_t;

        unsigned int maxsize;

        typedef std::map<std::string, map_value_t> map_t;

        class map_value_t {
        public:
            map_value_t(
                in_addr_t i,
                std::list<map_t::iterator>::iterator li)
                : ip(i),listiter(li) {}
            in_addr_t ip;
            std::list<map_t::iterator>::iterator listiter;
        };
        typedef std::list<map_t::iterator> list_t;

    public:
        Cache(unsigned int maxsize);
        ~Cache();

        in_addr_t* lookup(const std::string& address);
        in_addr_t* insert(std::string& alias, in_addr_t ip);

    private:
        map_t local_map;
        list_t local_list;
    };

    // Resolve exception
    class ResolveException : public std::runtime_error {
    public:
        ResolveException(int i, const char* s);
        ResolveException(const char* s);
        int errno() const;
        friend std::ostream& operator<<(std::ostream& os, const ResolveException& e);
    private:
        int errno_number;
        static const ssize_t MAXERRNOMSG=200;
    };

    DnsResolver(const std::string& filename, int maxsize);
    ~DnsResolver();

// uses inet_ntop, delegates to resolve(const std::string&)
    std::string* resolve(const std::string& address, std::string& result) throw (ResolveException);
// uses inet_pton
    in_addr_t* resolve(const std::string& address) throw (ResolveException);

    static int parse_line(const std::string& line, DnsEntry& parsed);

private:
    static const unsigned int MAXIPSTRING = 20; // actually 15 + 1 should do it
    static const unsigned int DEFAULT_CACHE_SIZE = 20; // actually 15 + 1 should do it
    static const unsigned int MAX_ALIASES = 5;

    std::ifstream& file;
    Cache& cache;
};

#endif // DNS_RESOLVER_H
