// lib includes
#include <string.h> // memset

// stdl includes
#include <sstream>

// Project includes
#include "helper.h"
#include "DnsWorker.h"

using namespace std;

DnsWorker::DnsWorker(DnsResolver& _resolver, Thread::Mutex &_resolve_mutex, const size_t _maxmessage)
    : resolver(_resolver), maxmessage(_maxmessage), resolve_mutex(_resolve_mutex){
    stop_flag = false;
    retval = -1;
    id = uniqueid++;
}

DnsWorker::~DnsWorker(){}

void DnsWorker::rest(){stop_flag = true;}

void* DnsWorker::main(){
    work();
    retval = 0;
    return &retval;
}

string DnsWorker::what() const{
    stringstream ss;
    ss << "["<< name() << ": id = \'" << id << "\' tid = x" << hex << Thread::self() << dec << "]";
    return ss.str();
}

void DnsWorker::work(){
    char* const temp = new char[maxmessage];
    memset(temp,0,maxmessage);
    cout << this->what() << ": starting to work..." << endl;

    while (!stop_flag){
        try {
            cerr << this->what() << ": setting up..." << endl;
            setup(); // tcp performs accept here, udp does nothing
        } catch (Socket::SocketException& e) {
            cerr << "Warning: socket exception waiting for setup: " << e.what() << ". Continuing..." << endl;
            // if (e.errno() == EINTR) continue; else break;
        }
        while (!stop_flag){
            try {
                try {
                    size_t read = readQuery(temp, maxmessage);
                    if (read == 0)
                        throw Socket::SocketException("Read 0 bytes");
                    // cerr << "\nRead " << read << " bytes: " << endl;
                    DnsMessage query(temp, read);
                    cout << this->what() << ": read " << read << " bute long query:" << query << endl;
                    try {
                        resolve_mutex.lock();
                        DnsResponse response(query, resolver, maxmessage);
                        resolve_mutex.unlock();
                        size_t towrite = response.serialize(temp, maxmessage);
                        size_t written = sendResponse(temp, towrite);
                        cout << this->what() << ": sent " << written << " byte long response: " << response << endl;
                    } catch (DnsMessage::DnsException& e){
                        resolve_mutex.unlock();
                        throw e;
                    }
                } catch (DnsMessage::DnsException& e){
                    cerr << "Warning: message exception: " << e.what() << endl;
                    DnsErrorResponse error_response(e);
                    cerr << "Responding with " << error_response << endl;
                    size_t towrite = error_response.serialize(temp, maxmessage);
                    // hexdump(temp, towrite);
                    size_t written = sendResponse(temp, towrite);
                    cout << this->what() << ": sent " << written << " byte long error response: " << error_response << endl;
                }
            } catch (DnsMessage::SerializeException& e) {
                cerr << "Warning: could not serialize error respose: " << e.what() << endl;
            } catch (Socket::SocketException& e) {
                cerr << "Warning: socket exception waiting for message: " << e.what() << ". Continuing..." << endl;
                cerr << this->what() << ": tearing down connection..." << endl;
                teardown();
                break;
            }
        }
    }
    delete []temp;
}

int DnsWorker::uniqueid = 0;

// UdpWorker

UdpWorker::UdpWorker(DnsResolver& resolver, const UdpSocket& s, Thread::Mutex& _resolvemutex, const size_t maxmessage) throw (Socket::SocketException)
    : DnsWorker(resolver, _resolvemutex, maxmessage), socket(s) {}

void UdpWorker::setup(){
} // Udp needs no special setup

void UdpWorker::teardown(){
} // Udp needs no special teardown

size_t UdpWorker::readQuery(char* buff, const size_t maxmessage) throw (Socket::SocketException){
    return socket.recvfrom(buff, clientAddress, maxmessage);
}

size_t UdpWorker::sendResponse(const char* buff, const size_t maxmessage) throw (Socket::SocketException){
    return socket.sendto(buff, clientAddress, maxmessage);
}

string UdpWorker::name() const {return string("UdpWorker");}


// TcpWorker

TcpWorker::TcpWorker(
    DnsResolver& resolver, const TcpSocket& socket, Thread::Mutex& acceptmutex,
    Thread::Mutex& _resolvemutex, unsigned int timeout, const size_t maxmessage)
    throw ()
    : DnsWorker(resolver, _resolvemutex, maxmessage),
      serverSocket(socket),
      acceptMutex(acceptmutex)
{
    timeout_tv.tv_sec = timeout;
    timeout_tv.tv_usec = 0;

}

TcpWorker::~TcpWorker() throw () {
    delete connectedSocket; // for safety
}

void TcpWorker::setup() throw (Socket::SocketException){
    try {
        acceptMutex.lock();
        connectedSocket = serverSocket.accept();
        cerr << this->what() << ": accepted connection..." << endl;
        acceptMutex.unlock();
        if (timeout_tv.tv_sec != 0)
            connectedSocket->setsockopt(SOL_SOCKET, SO_RCVTIMEO, &timeout_tv, sizeof(timeout_tv));
    } catch (Socket::SocketException& e) {
        acceptMutex.unlock();
        throw e;
    }
} 

size_t TcpWorker::readQuery(char* buff, const size_t maxmessage) throw(Socket::SocketException){
    char temp[2]={0};
    // cerr << "Worker in thread  " << hex << Thread::self() << "starting read of new length information)" << dec << endl;
    if (connectedSocket->read(temp,2) != 2)
        throw Socket::SocketException("No length information for new message (probably EOF)");
    uint16_t messagesize = ntohs(*(uint16_t*)&temp[0]);
    // hexdump(temp,2);
    if (messagesize < maxmessage){
        // cerr << "      (Advertised size was" << messagesize << ")" << endl;
        return connectedSocket->read(buff, maxmessage);
    } else {
        stringstream ss;
        ss << "Advertised size " << messagesize << " greater than max allowed size " << maxmessage;
        throw Socket::SocketException(ss.str().c_str());
    }
    
}

void TcpWorker::teardown() throw (Socket::SocketException){
    cerr << "TcpWorker " << id << " tearing down connection..." << endl;
    connectedSocket->close();
    delete connectedSocket;
    connectedSocket = NULL;
}

size_t TcpWorker::sendResponse(const char* buff, const size_t buflen) throw(Socket::SocketException){
    char temp[2]={0};
    *((uint16_t*)&temp[0]) = htons(buflen);
    
    if (connectedSocket->write(temp, 2) != 2)
        throw Socket::SocketException("Could not write length information for response");
    
    return connectedSocket->write(buff, buflen);
}

string TcpWorker::name() const {return string("TcpWorker");}



