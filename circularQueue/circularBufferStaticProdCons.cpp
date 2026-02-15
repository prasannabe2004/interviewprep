#include <atomic>
#include <chrono>
#include <condition_variable>
#include <iostream>
#include <mutex>
#include <thread>
#include <vector>

template <typename T> class CircularBuffer {
  public:
    explicit CircularBuffer(size_t capacity)
        : capacity_(capacity), head_(0), tail_(0), size_(0), running_(true) {
        buffer_.resize(capacity);
    }

    /**
     * @brief Pushes an item into the buffer. Blocks if full.
     * @return true if successful, false if buffer was stopped.
     */
    bool push(T item) {
        std::unique_lock<std::mutex> lock(mutex_);

        // Wait until not full or stopped
        // C++ wait() handles the while loop and spurious wakeups automatically via the lambda
        not_full_.wait(lock, [this]() { return size_ < capacity_ || !running_; });

        if (!running_)
            return false;

        buffer_[head_] = item;
        head_ = (head_ + 1) % capacity_;
        size_++;

        // Notify a single consumer that data is available
        // We use notify_one() because we know we only added 1 item
        not_empty_.notify_one();
        return true;
    }

    /**
     * @brief Pops an item from the buffer. Blocks if empty.
     * @return true if successful, false if buffer was stopped and empty.
     */
    bool pop(T& item) {
        std::unique_lock<std::mutex> lock(mutex_);

        // Wait until not empty or stopped
        not_empty_.wait(lock, [this]() { return size_ > 0 || !running_; });

        if (size_ == 0 && !running_)
            return false;

        item = buffer_[tail_];
        tail_ = (tail_ + 1) % capacity_;
        size_--;

        // Notify a single producer that space is available
        not_full_.notify_one();
        return true;
    }

    void stop() {
        std::lock_guard<std::mutex> lock(mutex_);
        running_ = false;
        // Wake everyone up so they can exit gracefully
        not_full_.notify_all();
        not_empty_.notify_all();
    }

  private:
    std::vector<T> buffer_;
    size_t capacity_;
    size_t head_;
    size_t tail_;
    size_t size_;
    bool running_;

    std::mutex mutex_;
    // We still use 2 condition variables for the same efficiency reasons as C
    std::condition_variable not_full_;
    std::condition_variable not_empty_;
};

// --- Test Code ---

CircularBuffer<int> g_cb(5);

void producer() {
    int i = 0;
    while (true) {
        i++;
        std::cout << "Producer: Pushing " << i << std::endl;
        if (!g_cb.push(i)) {
            std::cout << "Producer: Stopped" << std::endl;
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

void consumer() {
    while (true) {
        int val;
        if (!g_cb.pop(val)) {
            std::cout << "Consumer: Stopped" << std::endl;
            break;
        }
        std::cout << "Consumer: Popped " << val << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }
}

int main() {
    std::cout << "Starting C++ Producer-Consumer (Press Enter to stop)..." << std::endl;

    std::thread prod(producer);
    std::thread cons(consumer);

    // Wait for user input to stop
    std::cin.get();

    std::cout << "Stopping..." << std::endl;
    g_cb.stop();

    prod.join();
    cons.join();

    return 0;
}
