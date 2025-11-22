// C++ Program to demonstrate thread pooling

#include <chrono>
#include <condition_variable>
#include <functional>
#include <iostream>
#include <mutex>
#include <queue>
#include <sstream>
#include <thread>
using namespace std;

// Mutex to synchronize console output
static std::mutex cout_mutex;

// Class that represents a simple thread pool
class ThreadPool {
  public:
    // Constructor to create a thread pool with given number of threads
    ThreadPool(size_t num_threads = thread::hardware_concurrency()) {
        // Creating worker threads
        for (size_t i = 0; i < num_threads; ++i) {
            threads_.emplace_back([this] {
                while (true) {
                    function<void()> task;

                    // Lock the queue while waiting for a task or stop signal
                    unique_lock<mutex> lock(queue_mutex_);

                    // Waiting until there is a task to execute or the pool is stopped
                    cv_.wait(lock, [this] { return !tasks_.empty() || stop_; });

                    // exit the thread in case the pool is stopped and there are no tasks
                    if (stop_ && tasks_.empty()) {
                        return;
                    }

                    // Get the next task from the queue
                    task = move(tasks_.front());
                    tasks_.pop();

                    // explicitly unlock before executing the task so other threads can enqueue
                    lock.unlock();

                    task();
                }
            });
        }
    }

    // Destructor to stop the thread pool
    ~ThreadPool() {
        {
            // Lock the queue to update the stop flag safely
            unique_lock<mutex> lock(queue_mutex_);
            stop_ = true;
        }

        // Notify all threads
        cv_.notify_all();

        // Joining all worker threads to ensure they have completed their tasks
        for (auto& thread : threads_) {
            thread.join();
        }
    }

    // Enqueue task for execution by the thread pool
    void enqueue(function<void()> task) {
        {
            unique_lock<std::mutex> lock(queue_mutex_);
            tasks_.emplace(move(task));
        }
        cv_.notify_one();
    }

  private:
    // Vector to store worker threads
    vector<thread> threads_;

    // Queue of tasks
    queue<function<void()>> tasks_;

    // Mutex to synchronize access to shared data
    mutex queue_mutex_;

    // Condition variable to signal changes in the state of the tasks queue
    condition_variable cv_;

    // Flag to indicate whether the thread pool should stop or not
    bool stop_ = false;
};

// Named task function
void task_function(int i) {
    // Build the message first to avoid interleaving multiple << calls
    std::ostringstream oss;
    oss << "Task " << i << " is running on thread " << this_thread::get_id() << '\n';

    // Print the message under a short lock
    {
        std::lock_guard<std::mutex> lock(cout_mutex);
        std::cout << oss.str();
    }

    // Simulate work (do not hold the cout lock while sleeping)
    this_thread::sleep_for(chrono::milliseconds(100));
}

int main() {
    // Create a thread pool with 4 threads
    ThreadPool pool(4);

    // Enqueue tasks for execution using the named function
    for (int i = 1; i <= 10; ++i) {
        pool.enqueue(std::bind(task_function, i));
    }

    return 0;
}