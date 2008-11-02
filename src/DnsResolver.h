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

// Project includes
#include "Thread.h"

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
        const unsigned int maxsize = DEFAULT_CACHE_SIZE,
        const unsigned int maxaliases = DEFAULT_MAX_ALIASES) throw (ResolveException);
    ~DnsResolver();

    // public members
    std::string* resolve(const std::string& address, std::string& result) throw (ResolveException);
    struct in_addr* resolve(const std::string& address) throw (ResolveException);

    // friends
    friend std::ostream& operator<<(std::ostream& os, const DnsResolver& dns);

    // constants
    static const unsigned int DEFAULT_CACHE_SIZE = 20; // actually 15 + 1 should do it
    static const unsigned int DEFAULT_MAX_ALIASES = 5;

private:

    struct in_addr* resolve_helper(const std::string& address) throw (ResolveException);

    struct DnsEntry {
        struct in_addr ip;
        std::vector<std::string> aliases;
    };

    // Cache nested class
    class Cache {
    public:
        Cache(unsigned int maxsize);
        ~Cache();

        // public members
        struct in_addr* lookup(const std::string& address);
        struct in_addr* insert(std::string& alias, struct in_addr ip);
        bool full() const;

    private:
        // map_value_t nested nested class and friends
        class map_value_t;

        typedef std::map<std::string, map_value_t> map_t;
        typedef std::list<map_t::iterator> list_t;

        class map_value_t {
        public:
            map_value_t(
                struct in_addr i,
                std::list<map_t::iterator>::iterator li)
                : ip(i),listiter(li) {}
            struct in_addr ip;
            std::list<map_t::iterator>::iterator listiter;
        };

        unsigned int maxsize;

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
    Thread::Mutex mutex;
};

#endif // DNS_RESOLVER_H
