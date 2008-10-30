// Project includes
#include "DnsMessage.h"

// DnsMessage nested class

DnsMessage::DnsMessage(const char *buff, const size_t size) throw (ParseException){
    throw ParseException("Constructor not implemented yet");
}

DnsMessage::DnsMessage(){
}

DnsMessage::~DnsMessage(){
}

size_t DnsMessage::serialize(char*, unsigned long){
    return 0;
}

size_t DnsMessage::serialize(char* buff, const size_t bufsize);

// DnsResolver exception

DnsMessage::ParseException::ParseException(const char* s) : std::runtime_error(s) {}

std::ostream& operator<<(std::ostream& os, const DnsMessage::ParseException& e){
    return os << "[ParseException: " << e.what() << "]";
}
