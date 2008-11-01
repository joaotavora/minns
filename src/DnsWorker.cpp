// stdl includes

// Project includes
#include "DnsWorker.h"

using namespace std;

DnsWorker::DnsWorker(DnsResolver& _resolver, const size_t _maxmessage)
    : resolver(_resolver), maxmessage(_maxmessage){}

DnsWorker::~DnsWorker(){}

void DnsWorker::rest(){stop_flag = true;}
    

void DnsWorker::work(){
    stop_flag = false;

    int queries=0;
    
    char* const temp = new char[maxmessage];

    cout << "DnsServer starting..." << endl;
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
        }
        queries++;
        if (queries==2) break;
    }
    delete []temp;
}

// UdpWorker

UdpWorker::UdpWorker(uint16_t _port, DnsResolver& resolver, const size_t maxmessage) throw (Socket::SocketException) 
    : DnsWorker(resolver, maxmessage) {
    socket.bind_any(_port);
}

void UdpWorker::setup(){} // Udp needs no special setup

size_t UdpWorker::readQuery(char* buff, const size_t maxmessage){
    return socket.recvfrom(buff, clientAddress, maxmessage);
}

size_t UdpWorker::sendResponse(const char* buff, const size_t maxmessage){
    return socket.sendto(buff, clientAddress, maxmessage);
}

// TcpWorker

TcpWorker::TcpWorker(uint16_t _port, DnsResolver& resolver, TcpSocket& socket, Thread::Mutex& mutex, const size_t maxmessage)
    throw (Socket::SocketException) 
    : DnsWorker(resolver, maxmessage),
      serverSocket(socket),
      acceptMutex(mutex)
{}

void TcpWorker::setup(){
    acceptMutex.lock();
    connectedSocket = socket.accept();
    acceptMutex.unlock();
} // Tcp needs no special setup

size_t TcpWorker::readQuery(char* buff, const size_t maxmessage){
    return connectedSocket->read(buff, maxmessage);
}

size_t TcpWorker::sendResponse(const char* buff, const size_t maxmessage){
    return connectedSocket->write(buff, maxmessage);
}



