#include <condition_variable>
#include <iostream>
#include <mutex>
#include <queue>
#include <thread>

using namespace std;

template <typename T> class ThreadSafeQueue {
  private:
    queue<T> q;
    std::mutex mtx;
    std::condition_variable cv;

  public:
    void push(T val) {
        std::lock_guard<std::mutex> lock(mtx);
        q.push(std::move(val));
        cv.notify_one();
    }
    void pop(T& value) {
        std::unique_lock<std::mutex> lock(mtx);
        cv.wait(lock, [this] { return !q.empty(); });
        value = std::move(q.front());
        q.pop();
    }
};

void producer(ThreadSafeQueue<int>& thread) {
    for (int i = 0; i < 1000000; i++) {
        thread.push(i);
    }
}

void consumer(ThreadSafeQueue<int>& thread) {
    int val;
    while (1) {
        thread.pop(val);
        std::cout << "Processed " << val << endl;
    }
}

int main() {

    ThreadSafeQueue<int> t;

    std::thread p1(producer, std::ref(t));
    std::thread p2(producer, std::ref(t));

    std::thread c1(consumer, std::ref(t));
    std::thread c2(consumer, std::ref(t));

    p1.join();
    p2.join();
    c1.join();
    c2.join();

    return 0;
}