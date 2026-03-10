#include <chrono>
#include <cmath>
#include <iostream>

/*
Imagine IoT devices sending frequent sensor readings. How would you design a system that collapses
redundant readings but still preserves meaningful state changes?
*/
class SensorFilter {
  private:
    double lastValue = 0.0;
    double threshold;
    long long heartbeatIntervalMs;
    long long lastSentTime;

    // Helper to get current time in milliseconds
    long long getCurrentTimeMs() {
        return std::chrono::duration_cast<std::chrono::milliseconds>(
                   std::chrono::system_clock::now().time_since_epoch())
            .count();
    }

  public:
    // threshold: min change to trigger update | heartbeat: max time (ms) between updates
    SensorFilter(double threshold, long long heartbeat)
        : threshold(threshold), heartbeatIntervalMs(heartbeat) {
        lastSentTime = 0;
    }

    bool shouldUpdate(double newValue) {
        long long now = getCurrentTimeMs();

        // 1. Check for significant state change (Delta)
        bool significanceTrigger = std::abs(newValue - lastValue) >= threshold;

        // 2. Check for time-based heartbeat (ensures device isn't seen as "dead")
        bool heartbeatTrigger = (now - lastSentTime) >= heartbeatIntervalMs;

        if (significanceTrigger || heartbeatTrigger) {
            lastValue = newValue;
            lastSentTime = now;
            return true;
        }

        return false; // Redundant data, collapse it
    }
};

int main() {
    // Example: Only report if temp changes by 0.5 degrees or 5 seconds pass
    SensorFilter tempFilter(0.5, 5000);

    double readings[] = {22.0, 22.1, 22.2, 22.7, 22.7, 22.8};

    for (double r : readings) {
        if (tempFilter.shouldUpdate(r)) {
            std::cout << "TRANSMITTING: " << r << std::endl;
        } else {
            std::cout << "Collapsed redundant reading: " << r << std::endl;
        }
    }
    return 0;
}
