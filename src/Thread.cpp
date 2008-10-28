#include "Thread.h"

Thread::Thread(Runnable& h) throw ()
    : handler(h), tid(NULL) {}

~Thread::Thread(){
    delete &h;
}


void Thread::run() throw (ThreadException){
    if ((errno =
            pthread_create(&tid, NULL, &(Thread::helper), (void *) &handler)) != 0)
        throw ThreadException(errno, "Could not pthread_create()");
}

void Thread::join(void* retval) throw (ThreadException){
    if ((errno=pthread_join(tid, &retval) != 0))
        throw ThreadException(errno, "Could not pthread_join()");
}

void Thread::lock(pthread_mutex_t& t) const throw (ThreadException){
    if ((errno=pthread_mutex_lock(&t) != 0))
        throw ThreadException(errno, "Could not pthread_mutex_lock()");
}

void Thread::unlock(pthread_mutex_t& t) const throw (ThreadException){
    if ((errno=pthread_mutex_unlock(&t) != 0))
        throw ThreadException(errno, "Could not pthread_mutex_unlock()");
}

void* Thread::helper(void* args){
    return (static_cast<Runnable *>(args))->main();
}

// Runnable abstract nested class
void* Thread::Runnable::main(){}

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



// unit tests

using namespace std;

class TestRunnable : public Thread::Runnable {
    const int i;
public:

    TestRunnable(int ii)
        : i(i) {}

    void* main(){
        cout << "  TestRunnable " << i << " starting...";
        sleep(3);
        cout << "  TestRunnable " << i << " ending...";
    }
};


bool simpleRunnableTest() throw (){
    try {
        TestRunnable handlers[] = {
            TestRunnable(2),
            TestRunnable(4)
        };

        Thread competing[] = {
            Thread(handlers[0]),
            Thread(handlers[1])
        };
        for (int i=0; i < 2; i++) competing[i].run();
        for (int i=0; i < 2; i++) competing[i].join(NULL);
        cout << "Done!" << endl;
        return true;

    } catch (exception& e) {
        cout << "  exception: " << e.what() << endl;
        cout << "Failed!" << endl;
        return false;
    }
}

// int main(int argc, char* argv[]){
//     cout << "Starting Thread unit tests\n";
//     simpleRunnableTest();
//     cout << "Done with Thread unit tests\n";
// }

