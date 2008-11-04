// lib includes
#include <string.h> // memset

// stdl includes
#include <sstream>

// Project includes
#include "trace.h"
#include "helper.h"
#include "DnsWorker.h"

using namespace std;

DnsWorker::DnsWorker(DnsResolver& _resolver, Thread::Mutex &_resolve_mutex, const size_t _maxmessage)
    : resolver(_resolver), maxmessage(_maxmessage), resolve_mutex(_resolve_mutex){

    retval = -1;
    id = uniqueid++;
    served_error = 0;
    served = 0;
    stop_flag = false;
}

// signal handler related to thread shutdown
void DnsWorker::sig_alrm_handler(int signo){} // does nothing, but should interrupt all ongoing system calls

DnsWorker::~DnsWorker(){}

void* DnsWorker::main(){
    signal_helper(SIGALRM, DnsWorker::sig_alrm_handler);
    try {
        work();
        retval = 0;
    } catch (...) {
        retval = -1;
    }
    return &retval;
}

void DnsWorker::stop(){ stop_flag = true; }

string DnsWorker::what() const{
    stringstream ss;
    ss << "["<< name() << ": id = \'" << id << "\' tid = x" << hex << Thread::self() << dec << "]";
    return ss.str();
}

string DnsWorker::report() const{
    stringstream ss;
    ss << "["<< name() << ": id = " << id << " served = " << served << " served_error = " << served_error << "]";
    return ss.str();
}

void DnsWorker::work(){
    char* const temp = new char[maxmessage];
    memset(temp,0,maxmessage);
    ctrace << this->what() << ": starting to work..." << endl;

    //
    //  A. outer setup/teardown cycle
    //  
    while (!stop_flag){
        try {
            ctrace << this->what() << ": setting up..." << endl;
            setup(); // tcp performs accept here, udp does nothing
        } catch (Socket::SocketException& e) {
            cwarning << "SocketException during setup/teardown: " << e.what() << ". Resuming..." << endl;
            // if (e.errno() == EINTR) continue; else break;
        }
        //
        // B. inner read/write cycle
        // 
        while (!stop_flag){
            try {
                try {
                    // B.1 Read query
                    //
                    size_t read = readQuery(temp, maxmessage);
                    if (read == 0)
                        throw Socket::SocketException(TRACELINE("Read 0 bytes"));
                    DnsMessage query(temp, read);
                    ctrace << this->what() << ": read " << read << " byte long query:" << query << endl;

                    // B.2 Answer query, serialize and send response
                    //
                    //     handle any exceptions while answering, unlock mutex
                    //     and rethrow simply
                    //
                    //     handle rethrow any exceptions while serializing,
                    //     rethrow as SERVER_FAILURE. This should produce error
                    //     message to client
                    //
                    try {
                        resolve_mutex.lock();
                        DnsResponse response (query, resolver, maxmessage);
                        resolve_mutex.unlock();
                        size_t towrite = response.serialize(temp, maxmessage);
                        size_t written = sendResponse(temp, towrite);
                        ctrace << this->what() << ": sent " << written << " byte long response: " << response << endl;
                        served++;
                    } catch (DnsMessage::DnsException& e){
                        resolve_mutex.unlock();
                        throw e;
                    } catch (DnsMessage::SerializeException& e){
                        // TODO: Handle TC (Truncated bit) here.
                        cerror << "response serialization failed" << endl;
                        ctrace << "throwing another exception to indicate server failure " << endl;
                        throw DnsMessage::DnsException(query.getID(), DnsErrorResponse::SERVER_FAILURE, TRACELINE("TC not implemented yet"));
                    }
                    // B.3 Handle errors gracefully, responsing to client
                    // 
                } catch (DnsMessage::DnsException& e){
                    cwarning << " handling " << e.what() << endl;
                    DnsErrorResponse error_response(e);
                    size_t towrite = error_response.serialize(temp, maxmessage);
                    // hexdump(temp, towrite);
                    size_t written = sendResponse(temp, towrite);
                    cwarning << this->what() << ": sent " << written << " byte long error response: " << error_response << endl;
                    served_error++;
                }
                // B.4 Problem serializing error message, abort, keep on B cycle
            } catch (DnsMessage::SerializeException& e) {
                cerror << "could not serialize error respose: " << e.what() << endl;
                // B.5 Socket exception during B cycle, call polymorphic
                //     teardown and escape to A cycle. 
            } catch (Socket::SocketException& e) {
                cwarning << "Socket Exception during read/write cycle: " << e.what() << ". Resuming..." << endl;
                ctrace << this->what() << ": tearing down connection..." << endl;
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
        ctrace << this->what() << ": accepted connection..." << endl;
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
    if (connectedSocket->read(temp, 2, &stop_flag) != 2)
        throw Socket::SocketException("No length information for new message (probably EOF)");
    uint16_t messagesize = ntohs(*(uint16_t*)&temp[0]);
    // hexdump(temp,2);
    if (messagesize < maxmessage){
        // cerr << "      (Advertised size was" << messagesize << ")" << endl;
        return connectedSocket->read(buff, maxmessage, &stop_flag);
    } else {
        stringstream ss;
        ss << "Advertised size " << messagesize << " greater than max allowed size " << maxmessage;
        throw Socket::SocketException(ss.str().c_str());
    }
    
}

void TcpWorker::teardown() throw (Socket::SocketException){
    connectedSocket->close();
    delete connectedSocket;
    connectedSocket = NULL;
}

size_t TcpWorker::sendResponse(const char* buff, const size_t buflen) throw(Socket::SocketException){
    char temp[2]={0};
    *((uint16_t*)&temp[0]) = htons(buflen);
    
    if (connectedSocket->write(temp, 2, &stop_flag) != 2)
        throw Socket::SocketException(TRACELINE("Could not write() length info in response"));
    
    return connectedSocket->write(buff, buflen, &stop_flag);
}

string TcpWorker::name() const {return string("TcpWorker");}



