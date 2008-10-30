// libc includes

#include <errno.h>

// stdl includes

#include <sstream>

// Project includes

#include "DnsResolver.h"

using namespace std;

DnsResolver::DnsResolver(
    const std::string& filename,
    const unsigned int maxsize,
    const unsigned int maxa) throw (ResolveException)
    : file(*new ifstream(filename.c_str(), ios::in)),
      maxaliases(maxa),
      cache(*new Cache(maxsize))
{
    if (file.fail())
        throw ResolveException(string("Could not open \'" + filename + "\'").c_str());
}


string* DnsResolver::resolve(const std::string& address, std::string& dst) throw (ResolveException) {
    static char buff[INET_ADDRSTRLEN];

    struct in_addr* result = resolve(address);

    if (result == NULL) return NULL;

    if (inet_ntop (
            AF_INET,
            reinterpret_cast<void *>(result),
            buff,
            sizeof(buff)) == NULL)
        throw ResolveException("Could not place network address in a string");
    dst.assign(buff);
    return &dst;
}

DnsResolver::~DnsResolver(){
    file.close();
    delete &file;
    delete &cache;
}


struct in_addr* DnsResolver::resolve(const std::string& address) throw (ResolveException) {
    // lookup `address' in the cache
    struct in_addr* result = cache.lookup(address);
    if (result != NULL) {
        // clog <<  __FILE__ << ":" << (int)__LINE__ << "resolve(): Cache HIT! for \'" << address << "\'\n";
        return result;
    }
    // clog <<  "        (Cache MISS for \'" << address << "\')\n";

    // lookup `address' in the file, starting from wherever it was
    // FIXME: should go round the file exactly once
    string line;

    streampos startpos = file.tellg();
    // clog << "      (startpos tellg() is " << file.tellg() << ")" << endl;
    while (1) {
        if (!getline(file, line)){
            file.clear();
            // clog << "      (Turning file around)" << endl;
            file.seekg(0, ios::beg); // Start of file
            // clog << "      (tellg() is " << file.tellg() << ")" << endl;
            if (startpos == file.tellg()){
                // clog << "        (Giving up after on whole trip)" << endl;
                return NULL;
            }
            if (!getline(file, line)){
                // clog << "       (getline() false after seeking to beginning)" << endl;
                return NULL;
            }
        }
        if (startpos == file.tellg()){
            // clog << "        (Giving up after on whole trip)" << endl;
            return NULL;
        }
        // clog << "    (Got line " << line << ")" << endl;

        DnsEntry parsed;
        if (parse_line(line, parsed) != -1){
            for (vector<string>::iterator iter=parsed.aliases.begin(); iter != parsed.aliases.end(); iter++){
                if (address.compare(*iter) == 0){
                    // clog << "        (File HIT for \'" << *iter << "\' inserted into cache )" << endl;
                    result = cache.insert(*iter, parsed.ip);
                } else if (!cache.full()) {
                    // clog << "        (Inserting \'" << *iter << "\' into cache anyway )" << endl;
                    cache.insert(*iter, parsed.ip);
                } else {
                    // clog << "        (Cache is full \'" << *iter << "\' not inserted)" << endl;
                }

            }
            if (result != NULL) return result;
        }
    }
    return NULL;
}

int DnsResolver::parse_line(const std::string& line, DnsEntry& parsed) throw (ResolveException){
    if (line[0] == '#') return -1; // a # denotes a comment

    string buf;
    stringstream ss(line);

    if (ss >> buf){
        int retval = inet_pton(AF_INET, buf.c_str(), reinterpret_cast<char *>(&parsed.ip));
        switch (retval){
        case 0:
            return -1;
        case -1:
            throw ResolveException(errno, "Error in inet_pton()");
        default:
            break;
        }
    }  else return -1;

    while ((ss >> buf) and (parsed.aliases.size() < maxaliases))
        parsed.aliases.push_back(buf);

    return parsed.aliases.size();
}

std::ostream& operator<<(std::ostream& os, const DnsResolver& dns){
    return os <<
        "[DnsResolver: " <<
        "msize=\'" << dns.cache.local_map.size() << "\' " <<
        "lsize=\'" << dns.cache.local_list.size() << "\' " <<
        "head=\'" << dns.cache.print_head()  << "\' " <<
        "tail=\'" << dns.cache.print_tail()  << "\' " <<
        "]";
}


// Cache nested class

DnsResolver::Cache::Cache(unsigned int ms) : maxsize(ms){}

DnsResolver::Cache::~Cache(){
    local_map.clear();
    local_list.clear();
}

struct in_addr *DnsResolver::Cache::lookup(const string& address){
    // lookup the key in the map
    map_t::iterator i = local_map.find(address);
    if (i != local_map.end() ){
        // remove the iterator from the list pointing to the recently found
        // element. Do this with listiter!
        local_list.erase((i->second).listiter);
        // add it again at the front of the list
        local_list.push_front(i);
        return &((i->second).ip);
    }
    return NULL;
}

struct in_addr* DnsResolver::Cache::insert(string& alias, struct in_addr ip){

    // add element to map and keep an iterator to it. point the newly
    // map_value_t to local_list.begin(), but that will be made invalid soon.
    pair<map_t::iterator,bool> temppair =
        local_map.insert(make_pair(alias, *new map_value_t(ip, local_list.begin())));

    if (temppair.second != true){
        // clog << "DnsResolver::Cache::insert(): Insertion of " << alias << " failed!" << endl;
        return NULL;
    }
    // insert into the beginning of the list the recently obtained iterator to
    // the map
    local_list.push_front(temppair.first);

    // now do the reverse, let the recently inserted element also point to its
    // place in the list
    temppair.first->second.listiter=local_list.begin();

    // if we exceeded the maximum allowed size, remove from both list and map
    if (local_list.size() > maxsize){
        // remove from map
        // clog << "        (removing \'" << print_tail() << "\' from cache)" << endl;
        local_map.erase(local_list.back());
        local_list.pop_back();
    }
    return &(temppair.first->second.ip);
}

bool DnsResolver::Cache::full() const {
    return local_list.size() == maxsize;
}

string DnsResolver::Cache::print_head() const {
    if (local_list.size() > 0)
        return (local_list.front())->first;
    else
        return "<none>";
}

string DnsResolver::Cache::print_tail() const {
    if (local_list.size() > 0)
        return (local_list.back())->first;
    else
        return "<none>";
}

// ResolveException nested class

DnsResolver::ResolveException::ResolveException(const char* s)
    : std::runtime_error(s) {}

DnsResolver::ResolveException::ResolveException(int i, const char* s)
    : std::runtime_error(s), errno_number(i) {}

int DnsResolver::ResolveException::what_errno() const {return errno_number;}

const char * DnsResolver::ResolveException::what() const throw(){
    string s = std::runtime_error::what();

    if (errno_number != 0){
        s += ": ";
        char buff[DnsResolver::ResolveException::MAXERRNOMSG];
        strerror_r(errno_number, buff, DnsResolver::ResolveException::MAXERRNOMSG);
        s += buff;
    };
    return (s.c_str());
}


std::ostream& operator<<(std::ostream& os, const DnsResolver::ResolveException& e){
    return os << "[ResolveException: " << e.what() << "]";
}
