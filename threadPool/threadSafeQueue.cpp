#include <functional>
#include <iostream>
#include <mutex>
#include <thread>
#include <vector>

struct Node {
    int value;
    Node* next;

    Node(int v) : value(v), next(nullptr) {
    }
};

class ThreadSafeQueue {
  private:
    Node* head;
    Node* tail;
    std::mutex mtx;

  public:
    ThreadSafeQueue() : head(nullptr), tail(nullptr) {
    }

    // Push to the TAIL
    void push(int value) {
        Node* newNode = new Node(value);
        std::lock_guard<std::mutex> lock(mtx);

        if (!tail) {
            // Queue was empty
            head = tail = newNode;
        } else {
            tail->next = newNode;
            tail = newNode;
        }
    }

    // Pop from the HEAD
    int pop() {
        std::lock_guard<std::mutex> lock(mtx);
        if (!head)
            return -1; // Queue is empty

        Node* oldHead = head;
        int value = oldHead->value;

        head = head->next;

        // If we just popped the last element, reset tail
        if (!head) {
            tail = nullptr;
        }

        delete oldHead;
        return value;
    }
};

void pushToQueue(ThreadSafeQueue& queue, int value) {
    queue.push(value);
}

int main() {
    ThreadSafeQueue queue;

    std::vector<std::thread> threads;
    for (int i = 0; i < 10; ++i) {
        threads.emplace_back(pushToQueue, std::ref(queue), i);
    }

    for (auto& thread : threads) {
        thread.join();
    }

    int popValue = 0;
    while ((popValue = queue.pop()) != -1) {
        std::cout << "Popped: " << popValue << std::endl;
    }

    return 0;
}