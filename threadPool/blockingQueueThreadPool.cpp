#include <condition_variable>
#include <iostream>
#include <mutex>
#include <queue> // Using std::queue for internal storage to focus on synchronization
#include <thread>
#include <vector>

class BlockingQueue {
  private:
    std::queue<int> internal_queue;
    std::mutex mtx;
    std::condition_variable cv;
    bool done = false; // Flag to signal threads to stop waiting

  public:
    // Producer calls this
    void push(int value) {
        {
            std::lock_guard<std::mutex> lock(mtx);
            internal_queue.push(value);
        }
        // Wake up one sleeping consumer thread
        cv.notify_one();
    }

    // Consumer calls this
    bool pop(int& value) {
        std::unique_lock<std::mutex> lock(mtx);

        // Wait until queue is not empty OR we are done
        cv.wait(lock, [this] { return !internal_queue.empty() || done; });

        if (internal_queue.empty() && done) {
            return false; // Signal to the thread to exit
        }

        value = internal_queue.front();
        internal_queue.pop();
        return true;
    }

    // Signal that no more data is coming
    void shutdown() {
        {
            std::lock_guard<std::mutex> lock(mtx);
            done = true;
        }
        cv.notify_all(); // Wake up ALL consumers to let them exit
    }
};

void consumer_work(BlockingQueue& bq, int id) {
    int val;
    while (bq.pop(val)) {
        std::cout << "Consumer " << id << " processed: " << val << std::endl;
    }
    std::cout << "Consumer " << id << " shutting down." << std::endl;
}

int main() {
    BlockingQueue bq;

    // Start 3 consumer threads
    std::vector<std::thread> consumers;
    for (int i = 0; i < 3; ++i) {
        consumers.emplace_back(consumer_work, std::ref(bq), i);
    }

    // Produce 10 items
    for (int i = 0; i < 10; ++i) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Simulate work
        bq.push(i);
    }

    bq.shutdown(); // Tell consumers to finish

    for (auto& t : consumers) {
        t.join();
    }

    return 0;
}