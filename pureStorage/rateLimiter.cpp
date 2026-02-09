#include <chrono>
#include <condition_variable>
#include <deque>
#include <fstream>
#include <iostream>
#include <mutex>
#include <queue>
#include <thread>
#include <unordered_map>

using namespace std;
using Clock = chrono::steady_clock;

/* ---------------- Rate Limiter ---------------- */

class RateLimiter {
    int max_requests;
    chrono::milliseconds window;
    unordered_map<string, deque<Clock::time_point>> logs;
    mutex mtx;

  public:
    RateLimiter(int maxReq, int window_ms) : max_requests(maxReq), window(window_ms) {
    }

    bool allowRequest(const string& key) {
        auto now = Clock::now();
        lock_guard<mutex> lock(mtx);

        auto& q = logs[key];

        while (!q.empty() &&
               chrono::duration_cast<chrono::milliseconds>(now - q.front()) > window) {
            q.pop_front();
        }

        if (q.size() < max_requests) {
            q.push_back(now);
            return true;
        }
        return false;
    }
};

/* ---------------- Blocking Queue ---------------- */

class BlockingQueue {
    queue<string> q;
    mutex mtx;
    condition_variable cv;
    bool stop = false;

  public:
    void push(const string& msg) {
        {
            lock_guard<mutex> lock(mtx);
            q.push(msg);
        }
        cv.notify_one();
    }

    bool pop(string& msg) {
        unique_lock<mutex> lock(mtx);
        cv.wait(lock, [this] { return !q.empty() || stop; });

        if (stop && q.empty())
            return false;

        msg = q.front();
        q.pop();
        return true;
    }

    void shutdown() {
        {
            lock_guard<mutex> lock(mtx);
            stop = true;
        }
        cv.notify_all();
    }
};

/* ---------------- Consumer Thread ---------------- */

void consumer(BlockingQueue& bq) {
    ofstream file("logs.txt", ios::app);
    string msg;

    while (bq.pop(msg)) {
        file << msg << endl;
        file.flush();
        cout << "[FILE WRITE] " << msg << endl;
    }

    file.close();
}

/* ---------------- Producer ---------------- */

void producer(RateLimiter& limiter, BlockingQueue& bq, int id) {
    string user = "user" + to_string(id);

    for (int i = 0; i < 10; i++) {
        string msg = "Thread " + to_string(id) + " log " + to_string(i);

        if (limiter.allowRequest(user)) {
            bq.push(msg);
        } else {
            cout << "[DROP] Rate limit exceeded for " << user << endl;
        }

        this_thread::sleep_for(chrono::milliseconds(200));
    }
}

/* ---------------- Main ---------------- */

int main() {
    RateLimiter limiter(3, 2000);
    BlockingQueue bq;

    thread consumerThread(consumer, ref(bq));

    thread t1(producer, ref(limiter), ref(bq), 1);
    thread t2(producer, ref(limiter), ref(bq), 2);
    thread t3(producer, ref(limiter), ref(bq), 3);

    t1.join();
    t2.join();
    t3.join();

    bq.shutdown();
    consumerThread.join();

    return 0;
}
