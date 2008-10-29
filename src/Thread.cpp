#include "Thread.h"

using namespace std;

Thread::Thread(Runnable& h) throw ()
    : handler(h), tid(NULL) {}

Thread::~Thread(){}

void Thread::run() throw (ThreadException){
    if ((errno =
            pthread_create(&tid, NULL, &(Thread::helper), (void *) &handler)) != 0)
        throw ThreadException(errno, "Could not pthread_create()");
}

void Thread::join(void* retval) throw (ThreadException){
    if ((errno=pthread_join(tid, &retval) != 0))
        throw ThreadException(errno, "Could not pthread_join()");
}

void* Thread::helper(void* args){
    return (static_cast<Runnable *>(args))->main();
}


// Mutex nested class

Thread::Mutex::Mutex() throw (ThreadException) {
    // Support only default attributes
    if (pthread_mutex_init(&mutex, NULL) != 0)
        throw ThreadException(errno, "Could not pthread_mutex_init()");
}

Thread::Mutex::~Mutex(){
    if (pthread_mutex_destroy(&mutex) != 0)
        cerr << "~Mutex(): Warning: could not destroy mutex at 0x" << hex << &mutex << endl;
}

Thread::Mutex::Mutex(const Mutex& src){} // private copy constructor does nothing

void Thread::Mutex::unlock() throw (ThreadException){
    if ((errno=pthread_mutex_unlock(&mutex) != 0))
        throw ThreadException(errno, "Could not pthread_mutex_unlock()");
}

void Thread::Mutex::lock() throw (ThreadException){
    if ((errno=pthread_mutex_lock(&mutex) != 0))
        throw ThreadException(errno, "Could not pthread_mutex_lock()");
}

// Runnable "abstract" nested class
Thread::Runnable::Runnable() {}
Thread::Runnable::~Runnable() {}

void* Thread::Runnable::main(){return NULL;}

// ThreadException nested class

Thread::ThreadException::ThreadException(const char* s)
    : std::runtime_error(s) {}

Thread::ThreadException::ThreadException(int i, const char* s)
    : std::runtime_error(s), errno_number(i) {}

std::ostream& operator<<(std::ostream& os, const Thread::ThreadException& e){
    std::string s("[Exception: ");
    s += e.what();

    if (e.errno_number != 0){
        s += ": ";
        char buff[Thread::ThreadException::MAXERRNOMSG];
        strerror_r(e.errno_number, buff, Thread::ThreadException::MAXERRNOMSG);
        s += buff;
    };
    s += "]";

    return os << s;
}
