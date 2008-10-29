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
    static char buff[DnsResolver::MAXIPSTRING];

    in_addr_t* result = resolve(address);

    if (result == NULL) return NULL;

    in_addr temp= inet_makeaddr(*result,0);

    if (inet_ntop (
            AF_INET,
            reinterpret_cast<void *>(&temp),
            buff,
            sizeof(in_addr)) == NULL)
        throw ResolveException("Could not place network address in a string");
    dst.assign(buff);
    return &dst;
}

DnsResolver::~DnsResolver(){
    file.close();
    delete &file;
    delete &cache;
}


in_addr_t* DnsResolver::resolve(const std::string& address) throw (ResolveException) {
    // lookup `address' in the cache
    in_addr_t* result = cache.lookup(address);
    if (result != NULL) return result;

    // lookup `address' in the file, starting from wherever it was
    // FIXME: should go round the file exactly once
    string line;
    DnsEntry parsed;
    while (getline(file, line)){
        if (parse_line(line, parsed) != -1){
            for (vector<string>::iterator iter=parsed.aliases.begin(); iter != parsed.aliases.end(); iter++){
                in_addr_t* aux = cache.insert(*iter, parsed.ip);
                if (address.compare(*iter) == 0) result = aux;
            }
            if (result != NULL) return result;
        }
    }
    return NULL;
}

int DnsResolver::parse_line(const std::string& line, DnsEntry& parsed){
    if (line[0] == '#') return -1; // a # denotes a comment

    string buf;
    stringstream ss(line);

    if (ss >> buf){
        parsed.ip = inet_addr(buf.c_str());
        if (parsed.ip == INADDR_NONE) return -1;
    } else return -1;

    while ((ss >> buf) and (parsed.aliases.size() < MAX_ALIASES))
        parsed.aliases.push_back(buf);

    return parsed.aliases.size();
}

// Cache nested class

DnsResolver::Cache::Cache(unsigned int ms) : maxsize(ms){}

DnsResolver::Cache::~Cache(){
    local_map.clear();
    local_list.clear();
}

in_addr_t *DnsResolver::Cache::lookup(const string& address){
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

in_addr_t* DnsResolver::Cache::insert(string& alias, in_addr_t ip){

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

// ResolveException nested class

DnsResolver::ResolveException::ResolveException(const char* s)
    : std::runtime_error(s) {}

DnsResolver::ResolveException::ResolveException(int i, const char* s)
    : std::runtime_error(s), errno_number(i) {}

int DnsResolver::ResolveException::errno() const {return errno_number;}

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

int main(){
    try {
        DnsResolver a("testhosts.txt");
        string result;

        if (a.resolve("somehost", result) != NULL)
            cout << "  Sucess! resolved to " << result << "\n";
        else
            cout << "  Could not resolve!\n";

    } catch (std::exception& e) {
        cerr << "  Exception: " << e.what() << endl;
    }
}

