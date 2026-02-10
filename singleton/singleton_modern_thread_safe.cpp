#include <iostream>
#include <mutex>
#include <thread>
#include <vector>

using namespace std;

class Singleton {
  public:
    // 1. Thread-safe instance creation using C++11's magic statics
    static Singleton& GetInstance() {
        static Singleton instance;
        return instance;
    }

    // 2. PREVENT COPYING (Crucial!)
    Singleton(const Singleton&) = delete;
    Singleton& operator=(const Singleton&) = delete;

  private:
    Singleton() {
        cout << "Singleton Created" << endl;
    }
    ~Singleton() {
        cout << "Singleton Destroyed" << endl;
    }
};

void work(int name) {
    // Accessing the singleton
    Singleton& s = Singleton::GetInstance();
}

int main() {
    std::vector<std::thread> threads;
    for (int i = 0; i < 10; i++) {
        threads.push_back(std::thread(work, i));
    }
    for (auto& t : threads) {
        if (t.joinable())
            t.join();
    }
    return 0;
}