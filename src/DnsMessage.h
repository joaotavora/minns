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

    // ID: message ID, copied unchanged from query to response
    u_int16 ID;
    // QR: false for queries, true for answers
    bool QR;
    // OPCODE: operation type, only OP_QUERY supported
    u_int4 OPCODE;
    // AA: true if authoritative answer,
    bool AA;
    // TC: "trucated" bit, true if answer is truncated and maybe client
    // should attemp TCP
    bool TC;
    // RD: "recursion desired", true if the client wants recursion, copied
    // unchanged from query to response
    bool RD;
    // RA: "recursion available", always 0 in our case
    bool RA;
    // Z: reserved for future use"
    u_int3 Z;
    // RCODE: "return code" for example RCODE_NOERROR, RCODE_NXDOMAIN,
    // RCODE_SERVFAIL, etc
    u_int4 RCODE;

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
        // QNAME: domain name asked for
        DomainName QNAME;
        // QTYPE: query type - only DNS_TYPE_A supported
        u_int16 QTYPE;
        // QCLASS: query class - only CLASS_IN supported
        u_int16 QCLASS;
    };
    class ResourceRecord {
    };

};

#endif // DNS_MESSAGE_H
