#include <chrono>
#include <iostream>
#include <string>
#include <thread>
#include <unordered_map>

class MessageFilter {
  public:
    MessageFilter(int cooldownSeconds) : cooldownSeconds(cooldownSeconds) {
    }

    // Returns true if the message should be forwarded, false if suppressed
    bool filter(const std::string& msg) {
        using namespace std::chrono;
        auto now = steady_clock::now();
        size_t h = std::hash<std::string>{}(msg); // store only hash

        auto it = lastSeen.find(h);
        if (it == lastSeen.end()) {
            // First time seeing this message
            lastSeen[h] = now;
            return true;
        } else {
            auto elapsed = duration_cast<seconds>(now - it->second).count();
            if (elapsed >= cooldownSeconds) {
                // Enough time passed, allow again
                it->second = now;
                return true;
            } else {
                // Too soon, suppress
                return false;
            }
        }
    }

  private:
    int cooldownSeconds;
    std::unordered_map<size_t, std::chrono::steady_clock::time_point> lastSeen;
};

// Example usage
int main() {
    MessageFilter filter(3); // allow same message again after 3 seconds

    std::string messages[] = {"msg1", "msg2", "msg3", "msg1", "msg1",
                              "msg1", "msg4", "msg2", "msg3", "msg4"};

    for (const auto& msg : messages) {
        if (filter.filter(msg)) {
            std::cout << "[FORWARD] " << msg << std::endl;
        } else {
            std::cout << "[SUPPRESS] " << msg << std::endl;
        }
        std::this_thread::sleep_for(std::chrono::seconds(1)); // simulate time passing
    }

    return 0;
}
