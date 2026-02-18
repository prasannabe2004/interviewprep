#include <atomic>
#include <chrono>
#include <cstring>
#include <iostream>
#include <thread>
#include <vector>

/*
The Scenario
You are designing a High-Frequency Sensor Data Service for a specialized Android-based IoT device.
A custom hardware sensor connected via SPI/DMA generates data at 100kHz (100,000 samples/sec).
This data must be processed by a Vendor HAL and then made available to multiple unprivileged Android
Apps (NDK-based) for real-time visualization and logging.

Design a C++ system that acts as the bridge between the Kernel Space (Character Device) and the
Application Layer. Your solution must address: Latency: The "Glass-to-Wire" latency (from sensor
interrupt to App receiving data) must be under 500 microseconds. Multi-Client Support: Multiple apps
must be able to read the same stream without the HAL performing multiple data copies. Security: Apps
cannot have direct access to /dev/sensor0; they must interface through your HAL. Resilience: If one
App hangs or slows down, it must not cause the HAL to "block" or drop data for other healthy Apps.
*/
constexpr size_t BUFFER_SIZE = 1024; // power of 2 for fast modulo
constexpr size_t NUM_CONSUMERS = 2;

struct Sample {
    uint64_t timestamp;
    float value;
};

class LockFreeRingBuffer {
  public:
    LockFreeRingBuffer() : writeIndex(0) {
        buffer.resize(BUFFER_SIZE);
        for (size_t i = 0; i < NUM_CONSUMERS; ++i)
            readIndex[i].store(0);
    }

    void produce(const Sample& s) {
        uint64_t idx = writeIndex.load(std::memory_order_relaxed);

        buffer[idx & (BUFFER_SIZE - 1)] = s;

        std::atomic_thread_fence(std::memory_order_release);
        writeIndex.store(idx + 1, std::memory_order_relaxed);
    }

    bool consume(size_t consumerId, Sample& out) {
        uint64_t r = readIndex[consumerId].load(std::memory_order_relaxed);
        uint64_t w = writeIndex.load(std::memory_order_acquire);

        if (r >= w)
            return false; // no new data

        // Overrun detection
        if (w - r > BUFFER_SIZE) {
            std::cout << "Consumer " << consumerId << " overrun. Skipping ahead.\n";
            r = w - BUFFER_SIZE;
        }

        out = buffer[r & (BUFFER_SIZE - 1)];
        readIndex[consumerId].store(r + 1, std::memory_order_relaxed);
        return true;
    }

  private:
    std::vector<Sample> buffer;
    std::atomic<uint64_t> writeIndex;
    std::atomic<uint64_t> readIndex[NUM_CONSUMERS];
};

LockFreeRingBuffer ringBuffer;
std::atomic<bool> running(true);

void producer(int artificialDelayMs) {
    uint64_t counter = 0;

    while (running) {
        Sample s;
        s.timestamp = counter;
        s.value = static_cast<float>(counter);
        std::cout << "Producer:  " << s.timestamp << "\n";

        ringBuffer.produce(s);

        counter++;

        // simulate 100kHz (10µs interval)
        // std::this_thread::sleep_for(std::chrono::microseconds(artificialDelayMs));

        if (artificialDelayMs > 0)
            std::this_thread::sleep_for(std::chrono::milliseconds(artificialDelayMs));
    }
}

void consumer(size_t id, int artificialDelayMs) {
    while (running) {
        Sample s;
        if (ringBuffer.consume(id, s)) {
            std::cout << "Consumer " << id << " got sample: " << s.timestamp << "\n";

            if (artificialDelayMs > 0)
                std::this_thread::sleep_for(std::chrono::milliseconds(artificialDelayMs));
        } else {
            std::this_thread::yield();
        }
    }
}

int main() {

    std::thread prod(producer, 2000);

    // Consumer 0 (fast)
    std::thread c1(consumer, 0, 2000);

    // Consumer 1 (slow)
    std::thread c2(consumer, 1, 5000);

    std::this_thread::sleep_for(std::chrono::seconds(10));
    running = false;

    prod.join();
    c1.join();
    c2.join();

    return 0;
}
