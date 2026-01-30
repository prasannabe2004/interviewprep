#include <condition_variable>
#include <iostream>
#include <mutex>
#include <queue>
#include <thread>

using namespace std;

queue<int> q;
std::mutex mtx;
std::condition_variable cv;

void producer() {
    for (int i = 0; i < 1000000; i++) {
        std::lock_guard<std::mutex> lock(mtx);
        q.push(i);
        cv.notify_one();
    }
}

void consumer() {
    while (1) {
        std::unique_lock<std::mutex> lock(mtx);
        cv.wait(lock, [] { return !q.empty(); });
        int v = q.front();
        q.pop();
        cout << "processed " << v << endl;
    }
}

int main() {

    std::thread p1(producer);
    std::thread p2(producer);

    std::thread c1(consumer);
    std::thread c2(consumer);

    p1.join();
    p2.join();
    c1.join();
    c2.join();

    return 0;
}