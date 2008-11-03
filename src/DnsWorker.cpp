// lib includes
#include <string.h> // memset

// stdl includes
#include <sstream>

// Project includes
#include "helper.h"
#include "DnsWorker.h"

using namespace std;

DnsWorker::DnsWorker(DnsResolver& _resolver, const size_t _maxmessage)
    : resolver(_resolver), maxmessage(_maxmessage){
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

void DnsWorker::work(){
    char* const temp = new char[maxmessage];
    memset(temp,0,maxmessage);
    cout << "DnsWorker " << this->what() << " starting..." << endl;

    for (int i=0; i<2 ; i++){
        setup(); // tcp performs accept here, udp does nothing
        while (!stop_flag){
            try {
                try {
                    size_t read = readQuery(temp, maxmessage);
                    if (read == 0)
                        throw Socket::SocketException("Read 0 bytes");
                    cerr << "\nRead " << read << " bytes: " << endl;
                    DnsMessage query(temp, read);
                    cout << "Query is " << query << endl;
                    DnsResponse response(query, resolver, maxmessage);
                    cout << "Response is " << response << endl;
                    try {
                        size_t towrite = response.serialize(temp, maxmessage);
                        size_t written = sendResponse(temp, towrite);
                        cout << "\nSent " << written << " bytes: " << endl;
                    } catch (DnsMessage::SerializeException& e) {
                        throw DnsMessage::DnsException(query.getID(), DnsErrorResponse::SERVER_FAILURE, e.what());
                    }
                } catch (DnsMessage::DnsException& e){
                    cerr << "Warning: message exception: " << e.what() << endl;
                    DnsErrorResponse error_response(e);
                    cerr << "Responding with " << error_response << endl;
                    size_t towrite = error_response.serialize(temp, maxmessage);
                    // hexdump(temp, towrite);
                    size_t written = sendResponse(temp, towrite);
                    cerr << "\nSent " << written << " bytes: " << endl;
                }
            } catch (DnsMessage::SerializeException& e) {
                cerr << "Warning: could not serialize error respose: " << e.what() << endl;
            } catch (Socket::SocketException& e) {
                cerr << "Warning: socket exception: " << e.what() << ". Continuing..." << endl;
                teardown();
                break;
            }
        }
    }
    delete []temp;
}

int DnsWorker::uniqueid = 0;

// UdpWorker

UdpWorker::UdpWorker(DnsResolver& resolver, const UdpSocket& s, const size_t maxmessage) throw (Socket::SocketException)
    : DnsWorker(resolver, maxmessage), socket(s) {}

void UdpWorker::setup(){
    cerr << "UdpWorker " << id << " setting up..." << endl;
} // Udp needs no special setup

void UdpWorker::teardown(){
    cerr << "UdpWorker " << id << " tearing down connection..." << endl;
} // Udp needs no special teardown

size_t UdpWorker::readQuery(char* buff, const size_t maxmessage) throw (Socket::SocketException){
    return socket.recvfrom(buff, clientAddress, maxmessage);
}

size_t UdpWorker::sendResponse(const char* buff, const size_t maxmessage) throw (Socket::SocketException){
    return socket.sendto(buff, clientAddress, maxmessage);
}

string UdpWorker::what() const{
    stringstream ss;
    ss << "[UdpWorker: id = \'" << id << "\']";
    return ss.str();
}

// TcpWorker

TcpWorker::TcpWorker(DnsResolver& resolver, const TcpSocket& socket, Thread::Mutex& mutex, const size_t maxmessage)
    throw ()
    : DnsWorker(resolver, maxmessage),
      serverSocket(socket),
      acceptMutex(mutex)
{}

TcpWorker::~TcpWorker() throw () {
    delete connectedSocket; // for safety
}

void TcpWorker::setup() throw (Socket::SocketException){
    cerr << "TcpWorker " << id << " setting up..." << endl;
    acceptMutex.lock();
    connectedSocket = serverSocket.accept();
    cerr << "TcpWorker " << id << " accepted connection..." << endl;
    acceptMutex.unlock();
} // Tcp needs no special setup

size_t TcpWorker::readQuery(char* buff, const size_t maxmessage) throw(Socket::SocketException){
    char temp[2]={0};
    // cerr << "      (Starting read of new length information)" << endl;
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

string TcpWorker::what() const{
    stringstream ss;
    ss << "[TcpWorker: id = \'" << id << "\']";
    return ss.str();
}


