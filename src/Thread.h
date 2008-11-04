#ifndef THREAD_H
#define THREAD_H

// stdl includes

#include <iostream>
#include <stdexcept>

// libc includes

#include <pthread.h>
#include <semaphore.h>
#include <errno.h>


class Thread {
public:

    // Thread exception
    class ThreadException : public std::runtime_error {
    public:
        ThreadException(int i, const char* s);
        ThreadException(const char* s);
        const char * what() const throw();
    private:
        int errno_number;
        static const ssize_t MAXERRNOMSG=200;
    };

    // Runnable nested class
    class Runnable {
    public:
        Runnable();
        virtual ~Runnable() = 0;
        virtual void* main() = 0;
    };
    // Single constructor
    Thread(Runnable& handler) throw ();
    ~Thread();

    // Mutex nested class
    class Mutex {
    public:
        Mutex() throw(ThreadException);
        ~Mutex();
        void lock() throw (ThreadException);
        void unlock() throw (ThreadException);
    private:
        // private copy constructor
        Mutex(const Mutex& src);
        pthread_mutex_t mutex;
    };

    // Semaphore nested class
    class Semaphore {
    public:
        Semaphore(int level = 0, int value = 0) throw(ThreadException);
        ~Semaphore();
        void wait() throw (ThreadException);
        void post() throw (ThreadException);
    private:
        Semaphore(const Semaphore& src);
        sem_t sem;
    };

    // pthread wrappers
    void run() throw (ThreadException);
    void join(void* retval) throw (ThreadException);
    void kill(int signal) throw (ThreadException);

    // accessor
    pthread_t getTid() const {
        return tid;
    }

    static pthread_t self();

private:
    Runnable& handler;
    pthread_t tid;  // thread id pthread_create()
    static void* helper(void* args);

};

#endif // THREAD_H
