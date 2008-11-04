// libc includes
#include <string.h>
#include <pthread.h>
#include <signal.h>

// Project includes
#include "trace.h"
#include "Thread.h"

// usings
using namespace std;

// statics

const int Thread::ThreadException::EINVAL_error   = EINVAL;
const int Thread::ThreadException::EFAULT_error   = EFAULT;
const int Thread::ThreadException::ESRCH_error    = ESRCH;

Thread::Thread(Runnable& h) throw ()
    : handler(h), tid(NULL) {}

Thread::~Thread(){}

void Thread::run() throw (ThreadException){
    if ((errno =
            pthread_create(&tid, NULL, &(Thread::helper), (void *) &handler)) != 0)
        throw ThreadException(errno, TRACELINE("Could not pthread_create()"));
}

void Thread::join(void* retval) throw (ThreadException){
    if ((errno = pthread_join(tid, &retval) != 0))
        throw ThreadException(errno, TRACELINE("Could not pthread_join()"));
}

void Thread::kill(int signal) throw (ThreadException){
    if ((errno = pthread_kill(tid, signal)) != 0)
        throw ThreadException(errno, TRACELINE("Could not pthread_kill()"));
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
        throw ThreadException(errno, TRACELINE("Could not pthread_mutex_init()"));
}

Thread::Mutex::~Mutex(){
    if (pthread_mutex_destroy(&mutex) != 0)
        cerror << "~Mutex(): could not destroy mutex at 0x" << hex << &mutex << endl;
}

Thread::Mutex::Mutex(const Mutex& src){} // private copy constructor does nothing

void Thread::Mutex::unlock() throw (ThreadException){
    if ((errno=pthread_mutex_unlock(&mutex) != 0))
        throw ThreadException(errno, "Could not pthread_mutex_unlock()");
    ctrace << "        (Thread " << hex << self() << " unlocked mutex @" << hex << &mutex << ")"<< endl << dec;
}

void Thread::Mutex::lock() throw (ThreadException){
    ctrace << "        (Thread " << hex << self() << " waiting for mutex @" << hex << &mutex << ")" << endl << dec;
    if ((errno=pthread_mutex_lock(&mutex) != 0))
        throw ThreadException(errno, "Could not pthread_mutex_lock()");
    ctrace << "        (Thread " << hex << self() << " locked mutex @" << hex << &mutex << ")" << endl << dec;
}

// Semaphore nested class
Thread::Semaphore::Semaphore(int pshared, int value) throw (ThreadException){
    if (sem_init(&sem, pshared, value) == -1){
        throw ThreadException(errno, TRACELINE("Could not sem_init()"));
    }
}

Thread::Semaphore::~Semaphore(){
    if (sem_destroy(&sem) != 0)
        cerror << "~Semaphore(): could not destroy semaphore at 0x" << hex << &sem << endl;
}

Thread::Semaphore::Semaphore(const Semaphore& src){} // private copy constructor does nothing


void Thread::Semaphore::wait() throw (ThreadException){
    int sem_retval;
    while (((sem_retval = sem_wait(&sem)) == -1) && (errno == EINTR))
        continue;       /* Restart if interrupted by some handler */
    if (sem_retval != 0)
        throw ThreadException(errno, TRACELINE("Could not sem_wait()"));
}

void Thread::Semaphore::post() throw (ThreadException){
    if (sem_post(&sem) != 0)
        throw ThreadException(errno, TRACELINE("Could not sem_post()"));
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

int Thread::ThreadException::what_errno() const throw(){ return errno_number; }



