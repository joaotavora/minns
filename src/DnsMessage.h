#ifndef DNS_MESSAGE_H
#define DNS_MESSAGE_H

// stdl includes
#include <string>
#include <vector>
#include <stdexcept>

class DnsMessage {
public:
    class ParseException : public std::runtime_error {
    public:
        ParseException(const char* s);
        friend std::ostream& operator<<(std::ostream& os, const ParseException& e);
    private:
        static const ssize_t MAXERRNOMSG=200;
    };

    class QueryNotSupportedException : ParseException {
    };

    DnsMessage(const char *buff, const size_t size) throw (ParseException);
    DnsMessage();
    ~DnsMessage();

    size_t serialize(char* buff, const size_t bufsize);

    // friends
    friend std::ostream& operator<<(std::ostream& os, const DnsMessage& msg);

private:

        typedef uint16_t u_int16;
    typedef uint32_t u_int32;
    typedef char u_int4;
    typedef char u_int3;

    DnsMessage(DnsMessage& src);

    DnsMessage(DnsMessage& src);

    // ID(16): message ID, copied unchanged from query to response
    uint16_t ID;
    // QR(1): false for queries, true for answers
    bool QR;
    // OPCODE(4): operation type, only OP_QUERY supported
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
    std::vector<DnsQuestion> questions;
    // Variable length answers section, with ANCOUNT answers
    std::vector<ResourceRecord> answers;
    // Variable length authorities section, with NSCOUNT authorities
    std::vector<ResourceRecord> authorities;
    // Variable length additional section, with ARCOUNT additional resources
    std::vector<ResourceRecord> additional;
    class DomainName {

    };
    class DnsQuestion {
        // QNAME(variable, crazy structure): domain name asked for
        char* QNAME;
        // QTYPE(32 bits): query type - only DNS_TYPE_A supported
        uint16_t QTYPE;
        // QCLASS(16 bits): query class - only CLASS_IN supported
        uint16_t QCLASS;
    };
    class ResourceRecord {
        // NAME(variable, crazy structure): domain name the resource record is bound to. defaults to root
        // domain
        char* NAME;
        // TYPE(16 bits): type of this resource record, only DNS_TYPE_A(a host address) supported
        uint16_t TYPE;
        // CLASS(16 bits): class this resource record is in, only CLASS_IN supported
        uint16_t CLASS;
        // TTL(32 bits): time in seconds it may be stored in cache. always 0
        uint32_t TTL;
        // RDLENGTH: length in bytes of data pointed to by RDATA, only 4 is
        // supported
        uint16_t RDLENGTH;
        // RDATA: raw binary data for this resource record,
        char *RDATA;
    };
};

#endif // DNS_MESSAGE_H
