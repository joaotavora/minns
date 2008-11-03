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
    // Resolve exception
    class ResolveException : public std::runtime_error {
    public:
        ResolveException(int i, const char* s);
        ResolveException(const char* s);
        int what_errno() const;
        const char * what() const throw();
        friend std::ostream& operator<<(std::ostream& os, const ResolveException& e);
    private:
        int errno_number;
        static const ssize_t MAXERRNOMSG=200;
    };

    // public constructor/destructor
    DnsResolver(
        const std::string& filename,
        const unsigned int maxsize = DEFAULT_CACHE_SIZE[0],
        const unsigned int maxaliases = DEFAULT_MAX_ALIASES[0],
        const unsigned int maxialiases = DEFAULT_MAX_INVERSE_ALIASES[0]) throw (ResolveException);
    ~DnsResolver();

    // public members
    std::string& resolve_to_string(const std::string& what) throw (ResolveException);
    const std::list<struct in_addr>* resolve(const std::string& address) throw (ResolveException);

    // friends
    friend std::ostream& operator<<(std::ostream& os, const DnsResolver& dns);

    // constants
    static const unsigned int DEFAULT_CACHE_SIZE[3];
    static const unsigned int DEFAULT_MAX_ALIASES[3];
    static const unsigned int DEFAULT_MAX_INVERSE_ALIASES[3];

private:

    // internal struct when parsing a line
    struct DnsEntry {
        struct in_addr ip;
        std::list<std::string> aliases;
    };

    // Cache nested class
    class Cache {
    public:
        Cache(unsigned int maxsize, unsigned int maxialiases);
        ~Cache();

        // public members
        std::list<struct in_addr>* lookup(const std::string& name);
        std::list<struct in_addr>* insert(std::string& name, struct in_addr ip);
        bool full() const;

    private:
        // MapValue nested nested class and friends
        class MapValue;

        typedef std::map<std::string, MapValue> map_t;
        typedef std::list<map_t::iterator> list_t;

        class MapValue {
        public:
            MapValue(struct in_addr ip, std::list<map_t::iterator>::iterator li);
            std::list<struct in_addr> ips;
            std::list<map_t::iterator>::iterator listiter;
        };

        unsigned int maxsize;
        unsigned int maxipaliases;

        std::string print_head() const;
        std::string print_tail() const;

        friend std::ostream& operator<<(std::ostream& os, const DnsResolver& dns);

        map_t local_map;
        list_t local_list;
    };

    int parse_line(const std::string& line, DnsEntry& parsed) throw (ResolveException);

    std::ifstream& file;
    unsigned int maxaliases;
    Cache& cache;
};

#endif // DNS_RESOLVER_H
