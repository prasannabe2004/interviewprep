#include <iostream>
#include <vector>
#include <mutex>
#include <vector>
#include <chrono>
#include <cstdint>
#include <thread>

struct Packet {
    std::vector<uint8_t> data;
    std::chrono::steady_clock::time_point timestamp;

    Packet(const std::vector<uint8_t>& d)
        : data(d), timestamp(std::chrono::steady_clock::now()) {}
};

class PacketBuffer {
private:
    std::vector<Packet*> buffer;
    int head = 0;
    int tail = 0;
    int capacity;
    bool isFull = false;
    mutable std::mutex mtx;  // good if you want to lock inside const methods

public:
    PacketBuffer(int size) : capacity(size), buffer(size, nullptr) {}

    ~PacketBuffer() {
        std::lock_guard<std::mutex> lock(mtx);
        for (auto pkt : buffer) {
            delete pkt;
        }
    }

    // Enqueue a new packet
    bool enqueue(const std::vector<uint8_t>& data) {
        std::lock_guard<std::mutex> lock(mtx);

        if (isFull) {
            // Overwrite the oldest
            delete buffer[tail];
            buffer[tail] = new Packet(data);
            tail = (tail + 1) % capacity;
            head = tail;
        } else {
            buffer[head] = new Packet(data);
            head = (head + 1) % capacity;
            if (head == tail) isFull = true;
        }

        return true;
    }

    // Dequeue the oldest packet
    Packet* dequeue() {
        std::lock_guard<std::mutex> lock(mtx);

        if (isEmpty()) return nullptr;

        Packet* pkt = buffer[tail];
        buffer[tail] = nullptr;
        tail = (tail + 1) % capacity;
        isFull = false;

        return pkt;
    }

    bool isEmpty() const {
        return (!isFull && head == tail);
    }

    bool isBufferFull() const {
        return isFull;
    }

    int size() const {
        std::lock_guard<std::mutex> lock(mtx);
        if (isFull) return capacity;
        if (head >= tail) return head - tail;
        return capacity - tail + head;
    }
};

void producer(PacketBuffer& buf) {
    for (int i = 0; i < 10; ++i) {
        std::cout << "Produced packet: " << std::endl;
        buf.enqueue({static_cast<uint8_t>(i), static_cast<uint8_t>(i + 1)});
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

void consumer(PacketBuffer& buf) {
    for (int i = 0; i < 10; ++i) {
        Packet* pkt = buf.dequeue();
        if (pkt) {
            std::cout << "Consumed packet of size: " << pkt->data.size() << std::endl;
            delete pkt;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(150));
    }
}

int main() {
    PacketBuffer buffer(5);

    std::thread t1(producer, std::ref(buffer));
    std::thread t2(consumer, std::ref(buffer));

    t1.join();
    t2.join();

    return 0;
}
