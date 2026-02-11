#include <iostream>
#include <vector>

using namespace std;

template <typename T> class CircularBuffer {
  public:
    explicit CircularBuffer(size_t capacity)
        : buffer_(capacity), capacity_(capacity), head_(0), tail_(0), size_(0) {
    }

    // Add element
    void push(const T& value) {
        if (full()) {
            throw std::runtime_error("Buffer is full");
        }

        buffer_[head_] = value;
        head_ = (head_ + 1) % capacity_;
        ++size_;
    }

    // Remove element
    T pop() {
        if (empty()) {
            throw std::runtime_error("Buffer is empty");
        }

        T value = buffer_[tail_];
        tail_ = (tail_ + 1) % capacity_;
        --size_;
        return value;
    }

    bool empty() const {
        return size_ == 0;
    }

    bool full() const {
        return size_ == capacity_;
    }

    size_t size() const {
        return size_;
    }

  private:
    std::vector<T> buffer_;
    size_t capacity_;
    size_t head_;
    size_t tail_;
    size_t size_;
};

int main() {
    CircularBuffer<int> cb(3);

    cout << "Pushing 1" << endl;
    cb.push(1);
    cout << "Pushing 2" << endl;
    cb.push(2);
    cout << "Pushing 3" << endl;
    cb.push(3);

    cout << "Popping " << cb.pop() << endl; // 1
    cout << "Pushing 4" << endl;
    cb.push(4);

    cout << "Popping all" << endl;
    while (!cb.empty()) {
        cout << cb.pop() << endl;
    }
}
