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
    char* buff = new char[maxmessage];

    cout << "DnsServer starting..." << endl;
    while (!stopFlag){
        size_t read;
        try {
            read = socket.recvfrom(buff,client,maxmessage);
            cout << "Read " << read << " bytes long message: " << endl;
            hexdump(buff, read);
            // for (size_t i = 0; i < read; i++)
            //     putchar(buff[i]);

        } catch (Socket::SocketException& e) {
            cerr << "Warning: exception reading message: " << e.what() << ". Continuing..." << endl;
            continue;
        }
        if (read == 0){
            cerr << "Warning: read 0 bytes. Continuing..." << endl;
            continue;
        }
        try {
            DnsMessage query(buff, read);
            DnsMessage& response = *handle(query);

            size_t towrite = response.serialize(buff, maxmessage);
            socket.sendto(buff, client, towrite);
        } catch (DnsMessage::ParseException& e) {
            cerr << "Warning: exception parsing message: " << e.what() << ". Continuing..." << endl;
            continue;
        } catch (Socket::SocketException& e) {
            cerr << "Warning: exception sending reply: " << e.what() << ". Continuing..." << endl;
            continue;
        }
    }
}

DnsMessage *DnsServer::handle(DnsMessage& query) throw (){
    cerr << "Warning: returning a empty response" << endl;
    return new DnsMessage();
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
