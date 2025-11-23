// thread_pool_unlock_no_braces.cpp
// Compile: g++ -std=c++17 thread_pool_unlock_no_braces.cpp -pthread -O2

#include <condition_variable>
#include <functional>
#include <iostream>
#include <mutex>
#include <queue>
#include <sstream>
#include <thread>
#include <vector>
#include <chrono>
#include <cstdio>

// Helper to get thread id as string
std::string get_thread_id() {
    std::stringstream ss;
    ss << std::this_thread::get_id();
    return ss.str();
}

class ThreadPool {
public:
    explicit ThreadPool(std::size_t numThreads) : stop(false) {
        if (numThreads == 0)
            numThreads = 1;

        std::cout << "Initializing Thread Pool with "
                  << numThreads << " threads.\n";

        for (std::size_t i = 0; i < numThreads; ++i) {
            workers.emplace_back(&ThreadPool::workerThread, this);
        }
    }

    // Prevent copying
    ThreadPool(const ThreadPool&) = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;

    ~ThreadPool() {
        std::unique_lock<std::mutex> lock(queueMutex);
        stop = true;
        lock.unlock();

        condition.notify_all();

        for (std::thread& worker : workers) {
            if (worker.joinable())
                worker.join();
        }
    }

    // Enqueue a task (callable object)
    template <class F>
    void enqueue(F&& task) {
        std::unique_lock<std::mutex> lock(queueMutex);
        tasks.emplace(std::forward<F>(task));
        lock.unlock();

        condition.notify_one();
    }

private:
    void workerThread() {
        while (true) {
            std::function<void()> task;

            std::unique_lock<std::mutex> lock(queueMutex);

            // Wait until we have tasks or stop is set
            while (!stop && tasks.empty()) {
                condition.wait(lock);
            }

            // Stop AND no tasks left => exit thread
            if (stop && tasks.empty()) {
                lock.unlock();
                return;
            }

            // Fetch the task
            task = std::move(tasks.front());
            tasks.pop();

            // Unlock BEFORE running the task
            lock.unlock();

            // Execute the task (no mutex held here)
            task();
        }
    }

    std::vector<std::thread> workers;
    std::queue<std::function<void()>> tasks;

    std::mutex queueMutex;
    std::condition_variable condition;
    bool stop;
};

// A task functor (no lambda)
struct PrintTask {
    int id;

    void operator()() const {
        std::printf("Task %d executed by thread %s\n",
                    id, get_thread_id().c_str());
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
};

int main() {
    ThreadPool pool(2);   // Create 2 worker threads

    std::cout << "Thread Pool Created\n";
    std::cout << "Enqueue (assign) some tasks\n";

    for (int i = 0; i < 4; ++i) {
        pool.enqueue(PrintTask{i});
    }

    // main returns -> destructor joins threads
    return 0;
}
