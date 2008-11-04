// libc includes

#include <errno.h>
#include <string.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

// stdl includes

#include <sstream>

// Project includes
#include "trace.h"
#include "DnsResolver.h"

// usings

using namespace std;

// non integral type constant setting
const unsigned int DnsResolver::DEFAULT_CACHE_SIZE[3] = {20, 1, LONG_MAX};
const unsigned int DnsResolver::DEFAULT_MAX_ALIASES[3] = {5, 1, 512};
const unsigned int DnsResolver::DEFAULT_MAX_INVERSE_ALIASES[3] = {5, 1, 512};
const bool DnsResolver::DEFAULT_NOSTATFLAG = false;

DnsResolver::DnsResolver(
    const std::string& _filename,
    const unsigned int maxsize,
    const unsigned int maxa,
    const unsigned int maxialiases,
    const bool _nostatflag) throw (ResolveException)
    : maxaliases(maxa), filename(_filename), nostatflag(_nostatflag)
{

    struct stat filestat;
    if ((stat(_filename.c_str(), &filestat) != 0))
        throw ResolveException(string(TRACELINE("Could not stat() \'") + filename + "\'").c_str());

    file = new ifstream(filename.c_str(), ios::in);
    if (file->fail()){
        delete file;
        throw ResolveException(string(TRACELINE("Could not open \'") + filename + "\'").c_str());
    }
    
    cache = new Cache(maxsize, maxialiases, filestat.st_mtime);
}


DnsResolver::~DnsResolver(){
    file->close();
    delete file;
    delete cache;
}

string& DnsResolver::resolve_to_string(const string& what) throw (ResolveException){
    static char buff[INET_ADDRSTRLEN];
    static string retval;
    stringstream ss;

    addr_set_t result(*resolve(what));

    for (addr_set_t::iterator iter = result.begin(); iter != result.end() ; iter++){
        struct in_addr temp = *iter;
        if (inet_ntop (
                AF_INET,
                reinterpret_cast<void *>(&temp),
                buff,
                sizeof(buff)) == NULL)
            throw ResolveException(TRACELINE("Could not inet_ntop()"));
        ss << " " << buff;
    }
    retval.assign(ss.str());
    return retval;
}

const addr_set_t* DnsResolver::resolve(const std::string& name) throw (ResolveException) {
    addr_set_t* result;

    if (!nostatflag) {
        struct stat filestat;
        if (stat(filename.c_str(), &filestat) != 0)
            throw ResolveException(string(TRACELINE("Could not stat() \'") + filename + "\'").c_str());
        if (filestat.st_mtime != cache->get_file_mtime()){
            cwarning << "File modification time has changed, clearing cache\n" << endl;
            size_t size = cache->get_maxsize();
            size_t ialias = cache->get_maxialiases();
            delete cache;
            cache = new Cache(size, ialias, filestat.st_mtime);
            goto search;
        }
    } 
    result = cache->lookup(name);
    if (result != NULL) {
        ctrace <<  "\t(Cache HIT! for \'" << name << "\')\n";
        return result;
    } else {
        cwarning <<  "\t(Cache MISS for \'" << name << "\')\n";
    }

search:
    // search the file
    file->clear();
    file->seekg(0, ios::beg);
    string line;
    while (getline(*file, line)){
        // cerr << " got line \'" << line << "\'" << endl;
        DnsEntry parsed;
        if (parse_line(line, parsed) != -1){
            for (list<string>::iterator iter=parsed.aliases.begin(); iter != parsed.aliases.end(); iter++){
                if (name.compare(*iter) == 0){
                    ctrace << "\t(File HIT for \'" << *iter << "\' inserted into cache )" << endl;
                    result = cache->insert(*iter, parsed.ip);
                } else if (!cache->full()) {
                    ctrace << "\t(Inserting \'" << *iter << "\' into cache anyway )" << endl;
                    cache->insert(*iter, parsed.ip);
                } else {
                    cwarning << "\t(Cache is full \'" << *iter << "\' not inserted)" << endl;
                }
            }
            if (result != NULL) break;
        }
    }
    if (result == NULL)
        throw ResolveException(string("Could not resolve \'" + name + "\'").c_str());
    return result;
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
            throw ResolveException(errno, TRACELINE("could not inet_pton()"));
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
        "msize=\'" << dns.cache->local_map.size() << "\' " <<
        "lsize=\'" << dns.cache->local_list.size() << "\' " <<
        "head=\'" << dns.cache->print_head()  << "\' " <<
        "tail=\'" << dns.cache->print_tail()  << "\' " <<
        "]";
}


// Cache nested class

// MapValue constructor
DnsResolver::Cache::MapValue::MapValue(struct in_addr ip, list<map_t::iterator>::iterator li)
    : listiter(li) {
    ips.insert(ip);
}

DnsResolver::Cache::Cache(size_t ms, unsigned int maxialiases, time_t _file_mtime) :
    maxsize(ms),
    maxipaliases(maxialiases),
    file_mtime(_file_mtime){}

DnsResolver::Cache::~Cache(){
    local_map.clear();
    local_list.clear();
}

addr_set_t *DnsResolver::Cache::lookup(const string& name){
    // lookup the key in the map
    map_t::iterator i = local_map.find(name);
    if (i != local_map.end() ){
        // remove the iterator from the list pointing to the recently found
        // element. Do this with listiter!
        MapValue *tempvalue = &(i->second);
        local_list.erase(tempvalue->listiter);
        // add it again at the front of the list
        local_list.push_front(i);
        // update the listiter pointer in the MapValue element
        tempvalue->listiter = local_list.begin();
        return &((i->second).ips);
    }
    return NULL;
}

addr_set_t* DnsResolver::Cache::insert(string& alias, struct in_addr ip){

    // add element to map and keep an iterator to it. point the newly
    // MapValue to local_list.begin(), but that will be made invalid soon.

    map_t::iterator retval_iter;
    if ((retval_iter = local_map.find(alias)) != local_map.end()){
        if (retval_iter->second.ips.size() < maxipaliases)
            retval_iter->second.ips.insert(ip);
    } else {
        MapValue value(ip, local_list.begin());
        pair<map_t::iterator,bool> temppair =
            local_map.insert(make_pair(alias, value));
        if (temppair.second != true){
            cerror << "DnsResolver::Cache::insert(): Insertion of " << alias << " failed!" << endl;
            return NULL;
        }
        // insert into the beginning of the list the recently obtained iterator to
        // the map
        local_list.push_front(temppair.first);

        // now do the reverse: let the MapValue listiter element also point to its
        // place in the list
        temppair.first->second.listiter=local_list.begin();

        retval_iter = temppair.first;

        // if we exceeded the maximum allowed size, remove from both list and map
        // the least important element, the back of the list
        if (local_list.size() > maxsize){
            // remove from map
            cwarning << "\t(removing \'" << print_tail() << "\' from cache)" << endl;
            local_map.erase(local_list.back());
            local_list.pop_back();
        }
    }
    return &(retval_iter->second.ips);
}

bool DnsResolver::Cache::full() const {
    return local_list.size() == maxsize;
}

inline size_t DnsResolver::Cache::get_maxsize() const {return maxsize;}

inline size_t DnsResolver::Cache::get_maxialiases() const {return maxipaliases;}

inline time_t DnsResolver::Cache::get_file_mtime() const { return file_mtime; }

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
    : std::runtime_error(s) {
    errno_number = 0;
}

DnsResolver::ResolveException::ResolveException(int i, const char* s)
    : std::runtime_error(s), errno_number(i) {}

int DnsResolver::ResolveException::what_errno() const {return errno_number;}

const char * DnsResolver::ResolveException::what() const throw(){
    static string s;

    s.assign(std::runtime_error::what());

    if (errno_number != 0){
        s += ": ";
        s.append(strerror(errno_number));
    }
    return s.c_str();
}

std::ostream& operator<<(std::ostream& os, const DnsResolver::ResolveException& e){
    return os << "[ResolveException: " << e.what() << "]";
}
