// libc includes
#include "string.h"
#include "pthread.h"

// Project includes
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

pthread_t Thread::self(){
    return ::pthread_self();
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
    cerr << "Thread " << self() << " unlocked mutex @" << hex << &mutex << endl;
    if ((errno=pthread_mutex_unlock(&mutex) != 0))
        throw ThreadException(errno, "Could not pthread_mutex_unlock()");
}

void Thread::Mutex::lock() throw (ThreadException){
    cerr << "Thread " << self() << " locked mutex @" << hex << &mutex << endl;
    if ((errno=pthread_mutex_lock(&mutex) != 0))
        throw ThreadException(errno, "Could not pthread_mutex_lock()");
}


// Runnable "abstract" nested class
Thread::Runnable::Runnable() {}
Thread::Runnable::~Runnable() {}

// ThreadException nested class

Thread::ThreadException::ThreadException(const char* s)
    : std::runtime_error(s) {}

Thread::ThreadException::ThreadException(int i, const char* s)
    : std::runtime_error(s), errno_number(i) {}

const char * Thread::ThreadException::what() const throw(){
    static string s;

    s.assign(std::runtime_error::what());

    if (errno_number != 0){
        char buff[MAXERRNOMSG] = {0};
        s += ": ";
        s.append(strerror_r(errno_number, buff, MAXERRNOMSG));
    }

    return s.c_str();
}
