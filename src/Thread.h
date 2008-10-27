#ifndef THREAD_H
#define THREAD_H

class Thread {
public:
    class Runnable {
    public:
        virtual ~Runnable();
        virtual void main();
    }



    Thread(Runnable handler);




};

#endif // THREAD_H
