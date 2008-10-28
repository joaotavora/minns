#ifndef THREAD_H
#define THREAD_H

// stdl includes

#include <iostream>
#include <stdexcept>

// libc includes

#include <pthread.h>
#include <errno.h>


class Thread {
public:

    // Thread exception
    class ThreadException : public std::runtime_error {
    public:
        ThreadException(int i, const char* s);
        ThreadException(const char* s);
        friend std::ostream& operator<<(std::ostream& os, const ThreadException& e);
    private:
        int errno_number;
        static const ssize_t MAXERRNOMSG=200;
    };

    // Runnable nested class
    class Runnable {
    public:
        virtual ~Runnable();
        virtual void* main();
    };
    // Single constructor
    Thread(Runnable& handler) throw ();
    ~Thread();

    // pthread wrappers
    void run() throw (ThreadException);
    void join(void* retval) throw (ThreadException);
    void lock(pthread_mutex_t& t) const throw (ThreadException);
    void unlock(pthread_mutex_t& t) const throw (ThreadException);

private:
    Runnable& handler;
    pthread_t tid;  // thread id pthread_create()
    static void* helper(void* args);

};

#endif // THREAD_H
