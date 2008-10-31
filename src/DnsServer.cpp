// stdl includes
#include <iostream>
#include <fstream>
#include <stdexcept>

// project includes
#include "DnsServer.h"

// usings
using namespace std;

// class members definition

DnsServer::DnsServer (
    const string& filename = "/etc/hosts",
    const unsigned int cachesize = DnsResolver::DEFAULT_CACHE_SIZE,
    const unsigned int port = DEFAULT_DNS_PORT,
    const unsigned int maxaliases = DnsResolver::DEFAULT_MAX_ALIASES,
    const unsigned int the_maxmessage = UdpSocket::DEFAULT_MAX_MSG)  throw(std::exception)
    :
    socket(*new UdpSocket()),
    resolver(*new DnsResolver(filename, cachesize, maxaliases)),
    maxmessage(the_maxmessage)
    {
        socket.bind_any(port);
    }

DnsServer::~DnsServer(){
    delete &socket;
    delete &resolver;
}

// FIXME: remove this forward declaration of pasted code
void hexdump(void *pAddressIn, long  lSize);

void DnsServer::start(){
    stopFlag = false;
    Socket::SocketAddress client;
    char* temp = new char[maxmessage];

    cout << "DnsServer starting..." << endl;
    while (!stopFlag){
        size_t read;
        try {
            try {
                read = socket.recvfrom(temp,client,maxmessage);
                if (read == 0)
                    throw Socket::SocketException("Read 0 bytes");

                cout << "Read " << read << " bytes long message: " << endl;
                hexdump(temp, read);

                DnsMessage query(temp, read);
                cout << "Query is " << query << endl;
                DnsResponse& response = *handle(query, maxmessage);
                cout << "Response is " << response << endl;

                size_t towrite = response.serialize(temp, maxmessage);
                if (towrite == 0)
                    throw Socket::SocketException("Serialized 0 bytes");

                socket.sendto(temp, client, towrite);
                delete &response;
            } catch (DnsMessage::DnsException& e){
                cerr << "Warning: exception parsing message: " << e.what() << ". Responding..." << endl;
                DnsErrorResponse& response = *handle(e);
                size_t towrite = response.serialize(temp, maxmessage);
                if (towrite == 0)
                    throw Socket::SocketException("Serialized 0 bytes of error response");
                socket.sendto(temp, client, towrite);
            }
        } catch (Socket::SocketException& e) {
            cerr << "Warning: socket exception: " << e.what() << ". Continuing..." << endl;
            continue;
        }
    }
}

DnsResponse *DnsServer::handle(const DnsMessage& query, const size_t maxmessage) throw (DnsMessage::DnsException){
    return new DnsResponse(query, resolver, maxmessage);
}

DnsErrorResponse *DnsServer::handle(const DnsMessage::DnsException& e) throw (){
    return e.error_response();
}

// unit tests

void hexdump(void *pAddressIn, long  lSize)
{
 char szBuf[100];
 long lIndent = 1;
 long lOutLen, lIndex, lIndex2, lOutLen2;
 long lRelPos;
 struct { char *pData; unsigned long lSize; } buf;
 unsigned char *pTmp,ucTmp;
 unsigned char *pAddress = (unsigned char *)pAddressIn;

   buf.pData   = (char *)pAddress;
   buf.lSize   = lSize;

   while (buf.lSize > 0)
   {
      pTmp     = (unsigned char *)buf.pData;
      lOutLen  = (int)buf.lSize;
      if (lOutLen > 16)
          lOutLen = 16;

      // create a 64-character formatted output line:
      sprintf(szBuf, " >                            "
                     "                      "
                     "    %08X", pTmp-pAddress);
      lOutLen2 = lOutLen;

      for(lIndex = 1+lIndent, lIndex2 = 53-15+lIndent, lRelPos = 0;
          lOutLen2;
          lOutLen2--, lIndex += 2, lIndex2++
         )
      {
         ucTmp = *pTmp++;

         sprintf(szBuf + lIndex, "%02X ", (unsigned short)ucTmp);
         if(!isprint(ucTmp))  ucTmp = '.'; // nonprintable char
         szBuf[lIndex2] = ucTmp;

         if (!(++lRelPos & 3))     // extra blank after 4 bytes
         {  lIndex++; szBuf[lIndex+2] = ' '; }
      }

      if (!(lRelPos & 3)) lIndex--;

      szBuf[lIndex  ]   = '<';
      szBuf[lIndex+1]   = ' ';

      printf("%s\n", szBuf);

      buf.pData   += lOutLen;
      buf.lSize   -= lOutLen;
   }
}

void initialize(){
    // primitive log
    ofstream out("my_err");
    if ( out )
        clog.rdbuf(out.rdbuf());
    else
        cerr << "Error while opening the file" << endl;

    // FIXME: doesn't address the terminate() SIGSEGV problems, this needs
    // flushing on atexit()
}

int main(){
    try {
        DnsServer a(
            "/etc/hosts",
            DnsResolver::DEFAULT_CACHE_SIZE,
            43434,
            DnsResolver::DEFAULT_MAX_ALIASES,
            UdpSocket::DEFAULT_MAX_MSG);
        a.start();
        return 0;
    } catch (std::exception& e) {
        cerr << "Exception: " << e.what() << endl;
        return -1;
    }
}
