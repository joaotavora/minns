#ifndef DNS_MESSAGE_H
#define DNS_MESSAGE_H

// libc includes
#include <arpa/inet.h>

// stdl includes
#include <string>
#include <list>
#include <stdexcept>

// project includes
#include "DnsResolver.h"

class DnsErrorResponse;
class DnsMessage {
public:
    // DnsException - signals a problem
    class DnsException : public std::runtime_error {
    public:
        DnsException(const uint16_t ID, const char errorRCODE, const char* s);
    private:
        friend class DnsErrorResponse;
        uint16_t queryID;
        char errorRCODE;
    };

    class SerializeException : public std::runtime_error {
    public:
        SerializeException(const char* s);
    };
    
    // Constructors and destructors
    DnsMessage(char* buff, const size_t size) throw (DnsException);
    DnsMessage();
    virtual ~DnsMessage();
    void deleteRecords();

    const uint16_t getID() const {return ID;}

    // Serialize stuff
    size_t serialize(char* buff, const size_t len) throw (SerializeException);

    // friends - while DnsResponse inherits from DnsMessage, this is still
    // needed to have it modify private fields of another, pure DnsMessage
    // instance
    friend class DnsResponse;

private:
    DnsMessage(DnsMessage& src);

protected:
    size_t parse_qname(const char* buff, size_t buflen, char* resulting_thing);
    static size_t serialize_qname(const std::string& qname, char* resulting_thing, size_t buflen) throw (SerializeException);

    // friends
    friend std::ostream& operator<<(std::ostream& os, const DnsMessage& msg);

    typedef uint16_t u_int16;
    typedef uint32_t u_int32;
    typedef char u_int4;
    typedef char u_int3;

    // ID(2 bytes): message ID, copied unchanged from query to response
    uint16_t ID;
    // QR(1 bit): false for queries, true for answers
    bool QR;
    // OPCODE(4 bits): operation type, only OP_QUERY supported
    char OPCODE;
    // AA(1): true if authoritative answer,
    bool AA;
    // TC(1): "trucated" bit, true if answer is truncated and maybe client
    // should attemp TCP
    bool TC;
    // RD(1): "recursion desired", true if the client wants recursion, copied
    // unchanged from query to response
    bool RD;
    // RA(1): "recursion available", always 0 in our case
    bool RA;
    // Z(3): reserved for future use, yeah right
    char Z;
    // RCODE(4): "return code" for example RCODE_NOERROR, RCODE_NXDOMAIN,
    // RCODE_SERVFAIL, etc
    char RCODE;

    class DnsQuestion;
    class ResourceRecord;
    // Variable length question section, with QDCOUNT questions
    std::list<DnsQuestion> questions;
    // Variable length answers section, with ANCOUNT answers
    std::list<ResourceRecord> answers;
    // Variable length authorities section, with NSCOUNT authorities
    std::list<ResourceRecord> authorities;
    // Variable length additional section, with ARCOUNT additional resources
    std::list<ResourceRecord> additional;

    class DnsQuestion {
        friend class DnsResponse;
        // QNAME(variable, crazy structure): domain name asked for
        std::string QNAME;
        // QTYPE(4 bytes): query type - only DNS_TYPE_A supported
        uint16_t QTYPE;
        // QCLASS(2 bytes): query class - only CLASS_IN supported
        uint16_t QCLASS;

    public:
        DnsQuestion(const char* domainname, uint16_t _qtype, uint16_t _qclass);
        ~DnsQuestion(){};
        friend std::ostream& operator<<(std::ostream& os, const DnsMessage& msg);
        size_t serialize(char *buff, const size_t buflen) throw (SerializeException);

        DnsQuestion(const ResourceRecord& src){}
    };
    class ResourceRecord {
        // NAME(variable, crazy structure): domain name the resource record is bound to. defaults to root
        // domain
        std::string NAME;
        // TYPE(2 bytes): type of this resource record, only TYPE_A(a host address) supported
        uint16_t TYPE;
        // CLASS(2 bytes): class this resource record is in, only CLASS_IN supported
        uint16_t CLASS;
        // TTL(4 bytes): time in seconds it may be stored in cache. always 0
        uint32_t TTL;
        // RDLENGTH(2 bytes): length in bytes of data pointed to by RDATA, only 4 is
        // supported
        uint16_t RDLENGTH;
        // RDATA: raw binary data for this resource record, in network order
        // already.
        char *RDATA;
    public:
        size_t serialize(char *buff, const size_t buflen) throw (SerializeException);
        ResourceRecord(const std::string& name, const struct in_addr& resolvedaddress);
        ~ResourceRecord();
        friend std::ostream& operator<<(std::ostream& os, const DnsMessage& msg);
        ResourceRecord(const ResourceRecord& src);
        
    };

    // constants
    const static bool QUERY = 0;
    const static bool QUERY_A = 0;
    const static bool TYPE_A = 1;
    const static bool CLASS_IN = 1;
};

class DnsResponse : public DnsMessage {
    const DnsMessage& query;
public:
    DnsResponse(const DnsMessage& q, DnsResolver& resolver, const size_t maxresponse) throw (DnsException);
    ~DnsResponse();
    const static char NO_ERROR = 0;
};

class DnsErrorResponse : public DnsMessage {
public:
    DnsErrorResponse(const uint16_t ID, const char RCODE) throw ();
    DnsErrorResponse(const DnsException& e) throw ();

    // constants
    const static char FORMAT_ERROR = 1;
    const static char SERVER_FAILURE = 3;
    const static char NAME_ERROR = 3;
    const static char NOT_IMPLEMENTED = 4;
};

#endif // DNS_MESSAGE_H
