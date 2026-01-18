#include <functional>
#include <iostream>
#include <mutex>
#include <thread>
#include <vector>

// Define a simple Node structure for a stack
struct Node {
    int value;
    Node* next;

    Node(int v) : value(v), next(nullptr) {
    }
};

// Define a thread-safe stack using a mutex
class ThreadSafeStack {
  private:
    Node* head;
    std::mutex mtx;

  public:
    ThreadSafeStack() : head(nullptr) {
    }

    void push(int value) {
        Node* newNode = new Node(value);
        std::lock_guard<std::mutex> lock(mtx);
        newNode->next = head;
        head = newNode;
    }

    int pop() {
        std::lock_guard<std::mutex> lock(mtx);
        if (!head)
            return -1; // Stack is empty
        Node* oldHead = head;
        int value = oldHead->value;
        head = oldHead->next;
        delete oldHead;
        return value;
    }
};

void pushToStack(ThreadSafeStack& stack, int value) {
    stack.push(value);
}

int main() {
    ThreadSafeStack stack;

    // Spawn threads to perform push operations on the stack
    std::vector<std::thread> threads;
    for (int i = 0; i < 10; ++i) {
        threads.emplace_back(pushToStack, std::ref(stack), i);
    }

    // Wait for all push operations to complete
    for (auto& thread : threads) {
        thread.join();
    }

    // Pop all elements in the stack
    int popValue = 0;
    while ((popValue = stack.pop()) != -1) {
        std::cout << "Popped: " << popValue << std::endl;
    }

    return 0;
}