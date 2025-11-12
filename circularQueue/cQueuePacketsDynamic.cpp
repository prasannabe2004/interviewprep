/**
 * @file cQueuePackets.cpp
 * @brief Thread-Safe Circular Queue Implementation for Packet Buffering
 *
 * This file implements a thread-safe circular queue (ring buffer) specifically designed
 * for handling network packets or data packets in a producer-consumer scenario.
 *
 * Key Features:
 * - Thread-safe operations using std::mutex for concurrent access
 * - Circular buffer with fixed capacity to efficiently manage memory
 * - Automatic timestamp recording for each packet upon creation
 * - Overwrite behavior: when buffer is full, oldest packets are overwritten
 * - Support for producer-consumer pattern with multiple threads
 * - Immediate consumption using condition variables for event-driven processing
 *
 * Components:
 * - Packet: Structure containing data payload and timestamp
 * - PacketBuffer: Thread-safe circular queue implementation
 * - Producer/Consumer functions: Demonstration of multi-threaded usage
 *
 * Use Cases:
 * - Network packet buffering in communication systems
 * - Real-time data streaming applications
 * - Inter-thread communication with bounded buffer
 * - Any scenario requiring efficient FIFO data structure with overwrite capability
 *
 * @author Interview Preparation Code
 * @date Created for demonstrating circular queue implementation
 */

#include <iostream>
#include <vector>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <cstdint>
#include <thread>

struct Packet
{
    std::vector<uint8_t> data;
    std::chrono::steady_clock::time_point timestamp;

    Packet(const std::vector<uint8_t> &d)
        : data(d), timestamp(std::chrono::steady_clock::now()) {}
};

class PacketBuffer
{
private:
    std::vector<Packet *> buffer;
    int head = 0;
    int tail = 0;
    int capacity;
    bool isFull = false;
    mutable std::mutex mtx;             // good if you want to lock inside const methods
    mutable std::condition_variable cv; // for immediate consumption notification

public:
    PacketBuffer(int size) : capacity(size), buffer(size, nullptr) {}

    ~PacketBuffer()
    {
        std::lock_guard<std::mutex> lock(mtx);
        for (auto pkt : buffer)
        {
            delete pkt;
        }
        buffer.clear();
        head = tail = 0;
        isFull = false;
    }

    // Enqueue a new packet
    bool enqueue(const std::vector<uint8_t> &data)
    {
        std::lock_guard<std::mutex> lock(mtx);

        if (isFull)
        {
            // Overwrite the oldest
            delete buffer[tail];
            buffer[tail] = new Packet(data);
            tail = (tail + 1) % capacity;
            head = tail;
        }
        else
        {
            buffer[head] = new Packet(data);
            head = (head + 1) % capacity;
            if (head == tail)
                isFull = true;
        }

        // Notify waiting consumer that new packet is available
        cv.notify_one();
        return true;
    }

    // Dequeue the oldest packet (blocks until packet is available)
    Packet *dequeue()
    {
        std::unique_lock<std::mutex> lock(mtx);

        // Wait until a packet is available (immediate consumption)
        cv.wait(lock, [this]
                { return !isEmpty(); });

        Packet *pkt = buffer[tail];
        buffer[tail] = nullptr;
        tail = (tail + 1) % capacity;
        isFull = false;

        return pkt;
    }

    bool isEmpty() const
    {
        return (!isFull && head == tail);
    }

    int size() const
    {
        std::lock_guard<std::mutex> lock(mtx);
        if (isFull)
            return capacity;
        if (head >= tail)
            return head - tail;
        return capacity - tail + head;
    }
};

void producer(PacketBuffer &buf)
{
    for (int i = 0; i < 10; ++i)
    {
        std::cout << "Produced packet: " << std::endl;
        buf.enqueue({static_cast<uint8_t>(i), static_cast<uint8_t>(i + 1)});
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

void consumer(PacketBuffer &buf)
{
    for (int i = 0; i < 10; ++i)
    {
        // dequeue() now blocks until packet is available (immediate consumption)
        Packet *pkt = buf.dequeue();
        std::cout << "Consumed packet immediately! Size: " << pkt->data.size()
                  << " Data: [" << static_cast<int>(pkt->data[0]) << ", "
                  << static_cast<int>(pkt->data[1]) << "]" << std::endl;
        delete pkt;
    }
}

int main()
{
    PacketBuffer buffer(5);

    std::thread t1(producer, std::ref(buffer));
    std::thread t2(consumer, std::ref(buffer));

    t1.join();
    t2.join();

    return 0;
}
