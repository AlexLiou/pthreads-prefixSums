#ifndef _SPIN_BARRIER_H
#define _SPIN_BARRIER_H

#include <atomic>

class spin_barrier {

    public:
        spin_barrier();
        void lock();
        void unlock();
    private:
        std::atomic<bool> locked;
};

#endif
