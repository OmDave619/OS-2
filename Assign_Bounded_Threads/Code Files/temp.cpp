#include <iostream>
#include <vector>
#include <thread>
#include <pthread.h>
#include <sched.h>

void threadFunction() {
    // Placeholder for thread's work
    std::cout << "Thread " << std::this_thread::get_id() << " executing\n";
}

void setThreadAffinity(std::thread& th, int cpuId) {
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(cpuId, &cpuset);

    // Get the native thread handle and set affinity
    auto handle = th.native_handle();
    if (pthread_setaffinity_np(handle, sizeof(cpu_set_t), &cpuset) != 0) {
        std::cerr << "Error setting thread affinity\n";
    }
}

int main() {
    const int numThreads = 10; // Total number of threads
    const int numAffinityThreads = 3; // Number of threads to set affinity for
    int cpuId = 0; // CPU core ID to set affinity to

    std::vector<std::thread> threads;

    // Create and launch threads
    for (int i = 0; i < numThreads; ++i) {
        threads.push_back(std::thread(threadFunction));
    }

    // Set affinity for the first few threads
    for (int i = 0; i < numAffinityThreads; ++i) {
        setThreadAffinity(threads[i], cpuId + i); // Example: setting each to a different core, if available
    }

    // Join threads
    for (auto& th : threads) {
        th.join();
    }

    return 0;
}
