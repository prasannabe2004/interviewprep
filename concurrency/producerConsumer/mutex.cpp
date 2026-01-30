#include <iostream>
#include <mutex>
#include <queue>
#include <thread>

using namespace std;

queue<int> q;
std::mutex mtx;

void producer() {
    for (int i = 0; i < 1000000; i++) {
        std::lock_guard<std::mutex> lock(mtx);
        q.push(i);
    }
}

void consumer() {
    std::lock_guard<std::mutex> lock(mtx);
    while (!q.empty()) {
        int v = q.front();
        q.pop();
        // cout << "The value is " << v << endl;
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