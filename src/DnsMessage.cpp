// libc includes
#include <string.h>
#include <arpa/inet.h>

// stdl includes
#include <string>
#include <sstream>

// Project includes
#include "DnsMessage.h"

// usings
using namespace std;

//  use this example for reference
//
//              8 m y d o m a i n 3 c o m 0
//              0 1 2 3 4 5 6 7 8 9 a b c d
//              m y d o m a i n . c o m 0
//
//
size_t DnsMessage::parse_qname(const char* buff, size_t buflen, char* resulting_thing) {
    // total_len keeps the number of character processed
    size_t total_len = 0;
    // ptr should always point to beginning of a label. A special label whose first
    // byte is 0 marks the end of this domain name
    size_t pos = 0;
    while (true) {
        // check if we reached the end label
        if (buff[pos]==0) {
            resulting_thing[total_len - 1] = '\0'; // null terminate the thing!
            return total_len + 1;
        }
        // check if the topmost two bits are 00 as required
        if ((buff[pos] & 0xC0) != 0)
            throw DnsException(getID(), DnsErrorResponse::FORMAT_ERROR, "Unknown domain label type");
        size_t label_len = (buff[pos] & 0x3F);
        if (label_len > (buflen - total_len - 2))
            throw DnsException(getID(), DnsErrorResponse::FORMAT_ERROR, "One label mentions more characters than exist");

        memcpy(&resulting_thing[total_len], &buff[pos + 1], label_len);

        total_len += (1 + label_len);
        if (total_len >= 255)
            throw DnsException(getID(), DnsErrorResponse::FORMAT_ERROR, "Domain name too long");

        resulting_thing[total_len - 1]='.';

        pos += label_len + 1;
    }
}
// len is the total size of data to consider
DnsMessage::DnsMessage(char *data, const size_t len) throw (DnsException){
    uint16_t
        question_count, // question count
        answer_count, // answer count
        authorities_count, // authorities count
        ar_count; // additional record count

    ID = ntohs(*(uint16_t*)&data[0]);

    if (len < 12) throw DnsException(getID(), DnsErrorResponse::FORMAT_ERROR, "Buffer to small to hold DNS header");

    QR = (data[2] & 0x80) >> 7;

    if (QR != QUERY) throw DnsException(getID(), DnsErrorResponse::FORMAT_ERROR, "Uhhh. Client talking back to the server");

    OPCODE = (data[2] & 120) >> 3;

    if (OPCODE != QUERY_A) throw DnsException(getID(), DnsErrorResponse::NOT_IMPLEMENTED, "Only simple QUERY operation supported");

    AA = (data[2] & 0x04) >> 2;

    if (AA != 0) throw DnsException(getID(), DnsErrorResponse::FORMAT_ERROR, "Client thinks he's some kind of authority");

    TC = (data[2] & 0x02) >> 1;

    if (TC != false) throw DnsException(getID(), DnsErrorResponse::FORMAT_ERROR, "Client sent a truncated message, why?");

    RD = (data[2] & 0x01);

    RA = (data[3] & 0x80) >> 7;

    if (RA != false) throw DnsException(getID(), DnsErrorResponse::FORMAT_ERROR, "Nice to know the client has recursion");

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
            throw DnsException(getID(), DnsErrorResponse::FORMAT_ERROR, "So many questions, so little buffer space!");

        char *domainbuff = new char[len - pos - 4];
        int qname_len = parse_qname(&data[12], len - pos - 4, domainbuff);

        pos += qname_len;

        DnsQuestion question(
            domainbuff,
            ntohs(*(uint16_t*)&data[pos]),
            ntohs(*(uint16_t*)&data[pos+2]));
        questions.push_back(question);

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

DnsMessage::~DnsMessage(){}

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
    
    for (list<DnsQuestion>::iterator i = questions.begin(); i != questions.end() ; i++)
        pos += i->serialize(&buff[pos], bufsize - pos);

    for (list<ResourceRecord>::iterator i = answers.begin(); i != answers.end() ; i++)
        pos += i->serialize(&buff[pos], bufsize - pos);

    for (list<ResourceRecord>::iterator i = authorities.begin(); i != authorities.end() ; i++)
        pos += i->serialize(&buff[pos], bufsize - pos);

    for (list<ResourceRecord>::iterator i = additional.begin(); i != additional.end() ; i++)
        pos += i->serialize(&buff[pos], bufsize - pos);
    
    

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
            delete []temp;
            return pos + 1;
        }
        ssize_t label_len = strlen(temp);

        if (pos + 1 + label_len > buflen)
        {
            delete []temp;
            throw SerializeException("qname label won't fit into buffer");
        }
        

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

    list<DnsMessage::DnsQuestion> tempquestions(m.questions);
    list<DnsMessage::ResourceRecord> tempanswers(m.answers);
    
    for (list<DnsMessage::DnsQuestion>::iterator i = tempquestions.begin(); i != tempquestions.end() ; i++)
        ss << "(q= \'" << i->QNAME << "\')";

    for (list<DnsMessage::ResourceRecord>::iterator i = tempanswers.begin(); i != tempanswers.end() ; i++)
        ss << "(r= \'" << i->NAME << "\' : \'" << inet_ntoa(*((in_addr *)i->RDATA)) << "\')";
    
    return os << ss.str() << "]";
}

// DnsQuestion nested class

DnsMessage::DnsQuestion::DnsQuestion(const char* domainname, uint16_t _qtype, uint16_t _qclass)
    : QNAME(domainname),
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
    memcpy(RDATA, (void *)&resolvedaddress, RDLENGTH);
}

DnsMessage::ResourceRecord::ResourceRecord(const ResourceRecord& src)
    : NAME(src.NAME),
      TYPE(src.TYPE),
      CLASS(src.CLASS),
      TTL(src.TTL),
      RDLENGTH(src.RDLENGTH)
{
    RDATA = new char [RDLENGTH];
    memcpy(RDATA, src.RDATA, RDLENGTH);
}


DnsMessage::ResourceRecord::~ResourceRecord(){ delete []RDATA; }


// DnsException exception nested class

DnsMessage::DnsException::DnsException(const uint16_t ID, char RCODE, const char* s) : std::runtime_error(s), queryID(ID),  errorRCODE(RCODE){}

DnsMessage::SerializeException::SerializeException(const char* s) : std::runtime_error(s){}

// DnsResponse subclass

DnsResponse::DnsResponse(const DnsMessage& q, DnsResolver& resolver, size_t maxmessage) throw (DnsException)
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

    // keep the same questions
    questions.assign(query.questions.begin(), query.questions.end());

    for (list<DnsQuestion>::iterator iter =  questions.begin(); iter != questions.end(); iter++){
        // provide answers using resolver
        try {
            resolve_mutex.lock();
            list<struct in_addr> result(*resolver.resolve(iter->QNAME));
            resolve_mutex.unlock();
            for (list<struct in_addr>::iterator jter = result.begin(); jter != result.end() ; jter++){
                ResourceRecord record(iter->QNAME,*jter);
                answers.push_back(record);
            }
        } catch (DnsResolver::ResolveException& e) {
            // WARNING (cerr << "Caught exception: " << e.what() << endl);
            resolve_mutex.unlock();
            break;
        }
    }
    if (answers.size() == 0){
        throw DnsException(ID, DnsErrorResponse::NAME_ERROR, "Could not resolve any of the questions");
    }
    TC = false;
}

DnsResponse::~DnsResponse(){}

// DnsErrprResponse subclass

DnsErrorResponse::DnsErrorResponse(uint16_t _ID, const char _RCODE) throw ()
    : DnsMessage() {
    cerr << "Creating error response with ID " << ID << " from ID " << hex << _ID << endl;
    ID = _ID;
    QR = true;
    RCODE = _RCODE;
}

DnsErrorResponse::DnsErrorResponse(const DnsException& e) throw ()
    : DnsMessage(){
    ID = e.queryID;
    QR = true;
    RCODE = e.errorRCODE;
}



