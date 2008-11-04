#ifndef DNS_RESOLVER_H
#define DNS_RESOLVER_H

// stdl includes
#include <iostream>
#include <fstream>
#include <stdexcept>

#include <utility>
#include <list>
#include <set>
#include <map>

// libc includes
#include <sys/types.h>
#include <arpa/inet.h>


// Probably could get this class to be nested somewhere inside DnsResolver, but
// that takes too much typedef-engineering...
class in_addr_cmp {
public:
    bool operator()(const struct in_addr& s1, const struct in_addr& s2 ) const {
        return s1.s_addr < s2.s_addr;
    }
};

typedef std::set<struct in_addr, in_addr_cmp> addr_set_t;

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
        const unsigned int maxialiases = DEFAULT_MAX_INVERSE_ALIASES[0],
        const bool nostatflag = DEFAULT_NOSTATFLAG) throw (ResolveException);
    ~DnsResolver();

    // public members
    std::string& resolve_to_string(const std::string& what) throw (ResolveException);
    const addr_set_t* resolve(const std::string& address) throw (ResolveException);

    // friends
    friend std::ostream& operator<<(std::ostream& os, const DnsResolver& dns);

    // constants
    static const unsigned int DEFAULT_CACHE_SIZE[3];
    static const unsigned int DEFAULT_MAX_ALIASES[3];
    static const unsigned int DEFAULT_MAX_INVERSE_ALIASES[3];
    static const bool DEFAULT_NOSTATFLAG;

private:

    // internal struct when parsing a line
    struct DnsEntry {
        struct in_addr ip;
        std::list<std::string> aliases;
    };

    // Cache nested class
    class Cache {
    public:
        Cache(unsigned int maxsize, unsigned int maxialiases, time_t file_mtime);
        ~Cache();

        // public members
        addr_set_t* lookup(const std::string& name);
        addr_set_t* insert(std::string& name, struct in_addr ip);
        bool full() const;
        size_t get_maxsize() const;
        size_t get_maxialiases() const;
        time_t get_file_mtime() const;

    private:
        // MapValue nested nested class and friends
        class MapValue;

        typedef std::map<std::string, MapValue> map_t;
        typedef std::list<map_t::iterator> list_t;

        class MapValue {
        public:
            MapValue(struct in_addr ip, std::list<map_t::iterator>::iterator li);
            addr_set_t ips;
            std::list<map_t::iterator>::iterator listiter;
        };

        unsigned int maxsize;
        unsigned int maxipaliases;
        time_t file_mtime;

        std::string print_head() const;
        std::string print_tail() const;

        friend std::ostream& operator<<(std::ostream& os, const DnsResolver& dns);

        map_t local_map;
        list_t local_list;
    };

    int parse_line(const std::string& line, DnsEntry& parsed) throw (ResolveException);

    unsigned int maxaliases;
    std::string filename;
    bool nostatflag;
    
    std::ifstream* file;
    Cache* cache;
};

#endif // DNS_RESOLVER_H                        
