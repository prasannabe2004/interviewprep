#include <atomic>
#include <functional>
#include <iostream>
#include <thread>
#include <vector>

using namespace std;

// Define a simple Node structure for a stack
struct Node {
    int value;
    Node* next;

    Node(int v) : value(v), next(nullptr) {
    }
};

// Define the lock-free stack with basic push/pop operations
class LockFreeStack {
  private:
    std::atomic<Node*> head;

  public:
    LockFreeStack() : head(nullptr) {
    }

    void push(int value) {
        Node* newNode = new Node(value);
        // Set the new node's next to the current head
        newNode->next = head.load(std::memory_order_relaxed);
        // Now push the new node onto the stack
        while (!head.compare_exchange_weak(newNode->next, newNode, std::memory_order_release,
                                           std::memory_order_relaxed)) {
            // The compare_exchange_weak will automatically update newNode->next if the head is not
            // what we expected
        }
    }

    int pop() {
        Node* oldHead = head.load(std::memory_order_relaxed);
        while (oldHead &&
               !head.compare_exchange_weak(oldHead, oldHead->next, std::memory_order_release,
                                           std::memory_order_relaxed)) {
            // Similarly, this will try to update the head to the next element if the oldHead is the
            // head we are expecting
        }
        if (oldHead) {
            int value = oldHead->value;
            delete oldHead;
            return value;
        }
        return -1; // Stack is empty
    }
};

void pushToStack(LockFreeStack& stack, int value) {
    cout << "Pushed " << value << endl;
    stack.push(value);
}

int main() {
    LockFreeStack stack;

    // Spawn threads to perform push operations on the lock-free stack
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
        cout << "Popped: " << popValue << endl;
    }

    return 0;
}