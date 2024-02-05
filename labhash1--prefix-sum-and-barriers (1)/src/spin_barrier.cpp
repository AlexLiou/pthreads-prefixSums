#include <spin_barrier.h>
#include <io.h>
#include <atomic>

spin_barrier::spin_barrier() {
    locked.store(false);
}

void spin_barrier::lock() {
    bool expected = false;
    while (!locked.compare_exchange_weak(expected, true, std::memory_order_acquire)) {
        // Spin until the lock is acquired
        expected = false;
    }
}

void spin_barrier::unlock() {
    locked.store(false, std::memory_order_release);
}