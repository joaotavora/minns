// libc includes
#include <arpa/inet.h>

// stdl includes
#include <string>

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
            throw ParseException("Unknown domain label type");
        size_t label_len = (*ptr & 0x3F);
        if (label_len > (buflen - total_len - 2))
            throw ParseException("One label mentions more characters than exist");

        memcpy(&resulting_thing[total_len], &ptr[1], label_len);

        total_len += (1 + label_len);
        if (total_len >= 255)
            throw ParseException("Domain name too long");

        resulting_thing[total_len - 1]='.';

        ptr += total_len;
    }
}
// len is the total size of data to consider
DnsMessage::DnsMessage(char *data, const size_t len) throw (ParseException){
    uint16_t
        question_count, // question count
        answer_count, // answer count
        authorities_count, // authorities count
        ar_count; // additional record count

    if (len < 12) throw ParseException("Buffer to small to hold DNS header");

    ID = (uint16_t) htons(*(uint16_t*)&data[0]);

    QR = data[2] & 0x80;

    if (QR != 0) throw ParseException("Uhhh. Client talking back to the server");

    OPCODE = (data[2] & 120) >> 3;

    if (OPCODE != 0) throw NotSupportedException("Only simple QUERY operation supported");

    AA = data[2] & 4;

    if (AA != 0) throw ParseException("Client thinks he's some kind of authority");

    TC = data[2] & 2;

    if (TC != 0) throw ParseException("Client sent a truncated message, why?");

    RD = data[2] & 1;

    RA = data[3] & 0x80;

    if (RA != 0) throw ParseException("Nice to know the client has recursion");

    Z = (data[3] & 0x70) >> 3;

    // RCODE in queries is ignored
    RCODE = data[3] & 0x0F;

    question_count    = (uint16_t) htons(*(uint16_t*)&data[4]);
    answer_count      = (uint16_t) htons(*(uint16_t*)&data[6]);
    authorities_count = (uint16_t) htons(*(uint16_t*)&data[8]);
    ar_count          = (uint16_t) htons(*(uint16_t*)&data[10]);

  /* read question section */

    size_t pos = 12;
    for (int i = 0; i < question_count; i++) {
        if (pos >= len)
            throw ParseException("So many questions, so little buffer space!");

        char *domainbuff = new char[len - pos - 4];
        int qname_len = parse_qname(&data[12], len - pos - 4, domainbuff);

        pos += qname_len;

        questions.push_back(
            DnsQuestion(
                domainbuff,
                (uint16_t) htons(*(uint16_t*)&data[pos]),
                (uint16_t) htons(*(uint16_t*)&data[pos+2])));

        pos += 4;

        delete []domainbuff;
    }

  /* all other sections ignored */
}


DnsMessage::DnsMessage(){
}

DnsMessage::~DnsMessage(){
}

size_t DnsMessage::serialize(char*, unsigned long){
    return 0;
}

size_t DnsMessage::serialize(char* buff, const size_t bufsize);

// DnsQuestion nested class

DnsMessage::DnsQuestion::DnsQuestion(const char* domainname, uint16_t _qtype, uint16_t _qclass)
    : QNAME(string(domainname)),
      QTYPE(_qtype),
      QCLASS(_qclass) {}

// ParseException exception

DnsMessage::ParseException::ParseException(const char* s) : std::runtime_error(s) {}

std::ostream& operator<<(std::ostream& os, const DnsMessage::ParseException& e){
    return os << "[ParseException: " << e.what() << "]";
}
