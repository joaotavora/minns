// libc includes

#include <errno.h>

// stdl includes

#include <sstream>

// Project includes

#include "DnsResolver.h"

using namespace std;

DnsResolver::DnsResolver(const std::string& filename, int maxsize = DEFAULT_CACHE_SIZE)
    : file(*new ifstream(filename.c_str())),
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
        cerr <<  "        (Cache HIT! for \'" << address << "\')\n";
        return result;
    }
    cerr <<  "        (Cache MISS for \'" << address << "\')\n";

    // lookup `address' in the file, starting from wherever it was
    // FIXME: should go round the file exactly once
    string line;
    DnsEntry parsed;
    while (getline(file, line)){
        if (parse_line(line, parsed) != -1){
            for (vector<string>::iterator iter=parsed.aliases.begin(); iter != parsed.aliases.end(); iter++){
                struct in_addr* aux = cache.insert(*iter, parsed.ip);
                if (address.compare(*iter) == 0) result = aux;
            }
            if (result != NULL) {
                cerr <<  "        (File  HIT! for \'" << address << "\')\n";
                return result;
            }
        }
    }
    return NULL;
}

int DnsResolver::parse_line(const std::string& line, DnsEntry& parsed){
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

    while ((ss >> buf) and (parsed.aliases.size() < MAX_ALIASES))
        parsed.aliases.push_back(buf);

    return parsed.aliases.size();
}

std::ostream& operator<<(std::ostream& os, const DnsResolver& dns){
    return os <<
        "[DnsResolver: " <<
        "msize=\'" << dns.cache.local_map.size() << "\' " <<
        "lsize=\'" << dns.cache.local_list.size() << "\' " <<
        "head=\'" << dns.cache.local_list_head()  << "\' " <<
        "tail=\'" << dns.cache.local_list_tail()  << "\' " <<
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
        cerr << "DnsResolver::Cache::insert(): Insertion of " << alias << " failed!";
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
        local_map.erase(local_list.back());
        local_list.pop_back();
    }
    return &(temppair.first->second.ip);
}

string DnsResolver::Cache::local_list_head() const {
    if (local_list.size() > 0)
        return (local_list.front())->first;
    else
        return "<none>";
}

string DnsResolver::Cache::local_list_tail() const {
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

std::ostream& operator<<(std::ostream& os, const DnsResolver::ResolveException& e){
    std::string s("[Exception: ");
    s += e.what();

    if (e.errno_number != 0){
        s += ": ";
        char buff[DnsResolver::ResolveException::MAXERRNOMSG];
        strerror_r(e.errno_number, buff, DnsResolver::ResolveException::MAXERRNOMSG);
        s += buff;
    };
    s += "]";

    return os << s;
}

// Unit tests

string& tryResolve(DnsResolver& resolver, const string& what) throw (DnsResolver::ResolveException){
    static string result;

    if (resolver.resolve(what, result) != NULL)
        cout << "  Success! " << what << " resolved to " << result << "\n";
    else
        cout << "  Could not resolve " << what << "\n";
    return result;
}


int main(int argc, char* argv[]){
    try {
        DnsResolver a("testhosts.txt");
        string result;

        for (int i=1; i < argc; i++){
            tryResolve(a, argv[i]);
            cout << "  a = " << a << endl;
        }
    } catch (std::exception& e) {
        cerr << "  Exception: " << e.what() << endl;
    }
}

