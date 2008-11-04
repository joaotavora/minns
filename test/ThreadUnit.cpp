#include "Thread.h"

using namespace std;

class TestRunnable : public Thread::Runnable {
    int i;
public:

    TestRunnable(int ii)
        : i(ii) {}

    void* main(){
        cout << "  TestRunnable waiting " << i << " seconds...\n";
        sleep(i);
        cout << "  TestRunnable ending after " << i << " seconds...\n";
        return NULL;
    }
};

class TestRunnableMutex : public Thread::Runnable {
    int i;
    Thread::Mutex& mutex;
public:

    TestRunnableMutex(int ii, Thread::Mutex& m)
        : i(ii), mutex(m) {}

    void* main(){
        mutex.lock();
        cout << "  TestRunnableMutex (i=" << i << ") got mutex!...\n";
        cout << "  TestRunnableMutex waiting " << i << " seconds...\n";
        sleep(i);
        cout << "  TestRunnableMutex ending after " << i << " seconds...\n";
        mutex.unlock();
        return NULL;
    }
};


bool simpleRunnableTest() throw (){
    try {
        cout << "Starting simpleRunnableTest()...\n";
        TestRunnable handlers[] = {
            TestRunnable(2),
            TestRunnable(4)
        };

        Thread competing[] = {
            Thread(handlers[0]),
            Thread(handlers[1])
        };
        cout << "  Running threads...\n";
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

bool simpleMutexRunnableTest() throw (){
    try {
        cout << "Starting simpleMutexRunnableTest()...\n";

        Thread::Mutex common;

        TestRunnableMutex handlers[] = {
            TestRunnableMutex(2, common),
            TestRunnableMutex(4, common)
        };

        Thread competing[] = {
            Thread(handlers[0]),
            Thread(handlers[1])
        };
        cout << "  Running threads...\n";
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

int main(int argc, char* argv[]){
    cout << "Starting Thread unit tests\n";
    simpleRunnableTest();
    simpleMutexRunnableTest();
    cout << "Done with Thread unit tests\n";
}

