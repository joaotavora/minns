// libc includes
#include <arpa/inet.h>

// stdl includes
#include <string>
#include <sstream>

// Project includes
#include "DnsMessage.h"

// usings
using namespace std;

// DnsMessage class, full of bits
//
// This is my beautiful binary, hex, decimal reference
//
// 0000   x0  0
// 0001   x1  1
// 0010   x2  2
// 0011   x3  3
// 0100   x4  4
// 0101   x5  5
// 0110   x6  6
// 0111   x7  7
// 1000   x8  8
// 1001   x9  9
// 1010   xA 10
// 1011   xB 11
// 1100   xC 12
// 1101   xD 13
// 1110   xE 14
// 1111   xF 15
// 1100 0000 0xC0 192


// bufflen is the total length of QNAME of buff to consider,
// resulting_thing can be at most 1 char smaller.
//
//  use this example for reference
//
//              8 m y d o m a i n 3 c o m 0
//              0 1 2 3 4 5 6 7 8 9 a b c d
//              m y d o m a i n . c o m 0
//
//
size_t DnsMessage::parse_qname(char* buff, size_t buflen, char* resulting_thing) {
    // total_len keeps the number of character processed
    size_t total_len = 0;
    // ptr should always point to beginning of a label. A special label whose first
    // byte is 0 marks the end of this domain name
    char* ptr = buff;
    while (true) {
        // check if we reached the end label
        if (*ptr == 0) {
            resulting_thing[total_len - 1] = '\0'; // null terminate the thing!
            return total_len + 1;
        }
        // check if the topmost two bits are 00 as required
        if ((*ptr & 0xC0) != 0)
            throw ParseException(getID(), "Unknown domain label type");
        size_t label_len = (*ptr & 0x3F);
        if (label_len > (buflen - total_len - 2))
            throw ParseException(getID(), "One label mentions more characters than exist");

        memcpy(&resulting_thing[total_len], &ptr[1], label_len);

        total_len += (1 + label_len);
        if (total_len >= 255)
            throw ParseException(getID(), "Domain name too long");

        resulting_thing[total_len - 1]='.';

        ptr += label_len + 1;
    }
}
// len is the total size of data to consider
DnsMessage::DnsMessage(char *data, const size_t len) throw (ParseException){
    uint16_t
        question_count, // question count
        answer_count, // answer count
        authorities_count, // authorities count
        ar_count; // additional record count

    ID = ntohs(*(uint16_t*)&data[0]);

    if (len < 12) throw ParseException(getID(), "Buffer to small to hold DNS header");

    QR = (data[2] & 0x80) >> 7;

    if (QR != QUERY) throw ParseException(getID(), "Uhhh. Client talking back to the server");

    OPCODE = (data[2] & 120) >> 3;

    if (OPCODE != QUERY_A) throw NotSupportedException(getID(), "Only simple QUERY operation supported");

    AA = (data[2] & 0x04) >> 2;

    if (AA != 0) throw ParseException(getID(), "Client thinks he's some kind of authority");

    TC = (data[2] & 0x02) >> 1;

    if (TC != false) throw ParseException(getID(), "Client sent a truncated message, why?");

    RD = (data[2] & 0x01);

    RA = (data[3] & 0x80) >> 7;

    if (RA != false) throw ParseException(getID(), "Nice to know the client has recursion");

    Z = (data[3] & 0x70) >> 3;

    // RCODE in queries is ignored
    RCODE = data[3] & 0x0F;

    question_count    = ntohs(*(uint16_t*)&data[4]);
    answer_count      = ntohs(*(uint16_t*)&data[6]);
    authorities_count = ntohs(*(uint16_t*)&data[8]);
    ar_count          = ntohs(*(uint16_t*)&data[10]);

  /* read question section */

    size_t pos = 12;
    for (int i = 0; i < question_count; i++) {
        if (pos >= len)
            throw ParseException(getID(), "So many questions, so little buffer space!");

        char *domainbuff = new char[len - pos - 4];
        int qname_len = parse_qname(&data[12], len - pos - 4, domainbuff);

        pos += qname_len;

        questions.push_back(
            DnsQuestion(
                domainbuff,
                ntohs(*(uint16_t*)&data[pos]),
                ntohs(*(uint16_t*)&data[pos+2])));

        pos += 4;

        delete []domainbuff;
    }

  /* all other sections ignored */
}

DnsMessage::DnsMessage(){
    ID = 0;
    QR = AA = TC = RD = RA = false;
    RCODE = OPCODE = 0;
}

// x x x x x x x x
// 1 0 0 0 1  1

DnsMessage::~DnsMessage(){
}

size_t DnsMessage::serialize(char* buff, const size_t bufsize) throw (SerializeException){
    uint16_t network_short;

    if (bufsize < 12)
        throw SerializeException("Not enough space for DNS header");

    network_short=htons(ID);
    memcpy(&buff[0], &network_short, 2);

    buff[2] = (QR << 7) | (OPCODE << 3) | (AA << 2) | (TC << 1) | RD;
    buff[3] = (RA << 7) | (Z << 3) | RCODE;

    network_short=htons(questions.size());
    memcpy(&buff[4], &network_short, 2);

    network_short=htons(answers.size());
    memcpy(&buff[6], &network_short, 2);

    network_short=htons(authorities.size());
    memcpy(&buff[8], &network_short, 2);

    network_short=htons(additional.size());
    memcpy(&buff[10], &network_short, 2);

    size_t pos = 12;
    for (unsigned int i = 0; i< questions.size() ; i++)

        pos += questions[i].serialize(&buff[pos], bufsize - pos);

    for (unsigned int i = 0; i< answers.size() ; i++)
        pos += answers[i].serialize(&buff[pos], bufsize - pos);
    for (unsigned int i = 0 ; i< authorities.size() ; i++)
        pos += authorities[i].serialize(&buff[pos], bufsize - pos);
    for (unsigned int i = 0 ; i< additional.size() ; i++)
        pos += additional[i].serialize(&buff[pos], bufsize - pos);

    return pos;
}

size_t DnsMessage::DnsQuestion::serialize(char* buff, const size_t bufsize) throw (SerializeException){
    uint16_t network_short;

    if (bufsize < 5)
        throw SerializeException("Not enough space for question");

    try {

        int pos = serialize_qname(QNAME, &buff[0], bufsize - 4);

        network_short=htons(QTYPE);
        memcpy(&buff[pos], &network_short, 2);

        network_short=htons(QCLASS);
        memcpy(&buff[pos + 2], &network_short, 2);
        return pos + 4;
    } catch (SerializeException& e) {
        string s("Not enough space for question: ");
        s += e.what();
        throw SerializeException(s.c_str());
    }
}

size_t DnsMessage::serialize_qname(const std::string& qname, char* buff, size_t buflen) throw (DnsMessage::SerializeException){
    stringstream ss(qname);
    char *temp = new char[buflen];
    size_t pos = 0;

    while (true){
        if (!ss.getline(temp, buflen, '.')){
            buff[pos] = 0;
            return pos + 1;
        }
        ssize_t label_len = strlen(temp);

        if (pos + 1 + label_len > buflen)
            throw SerializeException("qname label won't fit into buffer");

        buff[pos] = label_len & 0x3F;
        memcpy(&buff[pos+1],&temp[0],label_len);
        pos += label_len + 1;
    }
}


size_t DnsMessage::ResourceRecord::serialize(char* buff, const size_t bufsize) throw (SerializeException){
    uint16_t network_short;
    uint32_t network_long;

    if (bufsize < ((size_t)11 - RDLENGTH))
        throw SerializeException("Not enough space for Rrecord");

    try {
        int pos = serialize_qname(NAME, &buff[0], bufsize - 10 - RDLENGTH);

        network_short=htons(TYPE);
        memcpy(&buff[pos], &network_short, 2);

        network_short=htons(CLASS);
        memcpy(&buff[pos + 2], &network_short, 2);

        network_long=htonl(TTL);
        memcpy(&buff[pos + 4], &network_long, 2);

        network_short=htons(RDLENGTH);
        memcpy(&buff[pos + 8], &network_short, 2);

        // RDATA is already in network order
        memcpy(&buff[pos + 10], RDATA, RDLENGTH);
        return pos + 10 + RDLENGTH;
    } catch (SerializeException& e) {
        string s("Not enough space for record: ");
        s += e.what();
        throw SerializeException(s.c_str());
    }
}



std::ostream& operator<<(std::ostream& os, const DnsMessage& m){
    stringstream ss;

    ss << "[DnsMessage:" <<
        " ID= \'" << hex << m.ID <<
        "\' RCODE= \'" << hex << (uint16_t) m.RCODE <<
        "\' questions= \'" << m.questions.size() <<
        "\' answers= \'" << m.answers.size() << "\' ";

    for (unsigned int i=0; i < m.questions.size(); i++)
        ss << "(q" << i << "= \'" << m.questions[i].QNAME << "\')";
    for (unsigned int i=0; i < m.answers.size(); i++)
        ss << "(r" << i << "= \'" << m.answers[i].NAME << "\' : \'" << inet_ntoa(*((in_addr *)m.answers[i].RDATA)) << "\')";
    return os << ss.str() << "]";
}

// DnsQuestion nested class

DnsMessage::DnsQuestion::DnsQuestion(const char* domainname, uint16_t _qtype, uint16_t _qclass)
    : QNAME(string(domainname)),
      QTYPE(_qtype),
      QCLASS(_qclass) {}


DnsMessage::ResourceRecord::ResourceRecord(const std::string& name, const struct in_addr& resolvedaddress)
    : NAME(name),
      TYPE(TYPE_A),
      CLASS(CLASS_IN),
      TTL(0),
      RDLENGTH(4)
{
    RDATA = new char[RDLENGTH];
    memcpy(RDATA, &resolvedaddress, RDLENGTH);
}

// DnsException exception nested class

DnsMessage::DnsException::DnsException(const uint16_t ID, const char* s) : std::runtime_error(s), queryID(ID) {}

DnsErrorResponse* DnsMessage::DnsException::error_response() const {
    return new DnsErrorResponse(queryID, DnsErrorResponse::SERVER_FAILURE);
}

DnsErrorResponse* DnsMessage::ParseException::error_response() const {
    return new DnsErrorResponse(queryID, DnsErrorResponse::FORMAT_ERROR);
}

DnsErrorResponse* DnsMessage::CouldNotResolveException::error_response() const {
    return new DnsErrorResponse(queryID, DnsErrorResponse::NAME_ERROR);
}

DnsErrorResponse* DnsMessage::NotSupportedException::error_response() const {
    return new DnsErrorResponse(queryID, DnsErrorResponse::NOT_IMPLEMENTED);
}

std::ostream& operator<<(std::ostream& os, const DnsMessage::ParseException& e){
    return os << "[ParseException: " << e.what() << "]";
}

// DnsResponse subclass

DnsResponse::DnsResponse(const DnsMessage& q, DnsResolver& resolver, size_t maxmessage) throw (CouldNotResolveException)
    : DnsMessage(),query(q) {
    ID = q.ID;
    QR = true;
    OPCODE = q.OPCODE;
    AA = true; // We're an authority on this matter
    // TC: will be calculated later
    RCODE = DnsResponse::NO_ERROR;
    RD = q.RD;
    RA = false;
    Z = q.Z;
    for (unsigned int i=0; i < query.questions.size(); i++){
        // keep the same questions
        questions.push_back(query.questions[i]);
        // provide answers using resolver
        struct in_addr* result = resolver.resolve(query.questions[i].QNAME);
        if (result != NULL) answers.push_back(*new ResourceRecord(query.questions[i].QNAME,
                *result));
    }
    if (answers.size() == 0){
        questions.clear();
        answers.clear(); // for clarity, not necessary
        throw CouldNotResolveException(ID, "Could not resolve any of the questions");
    }


    TC = false;
}

DnsErrorResponse::DnsErrorResponse(uint16_t _ID, const char _RCODE) throw ()
    : DnsMessage() {
    cerr << "Creating error response with ID " << ID << " from ID " << hex << _ID << endl;
    ID = _ID;
    QR = true;
    RCODE = _RCODE;
}



