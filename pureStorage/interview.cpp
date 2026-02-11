/*

Design a system such a way that it can registers a callack and callback is fired on when a event is
triggered

make sure the system dont block to fire the call back
if the registration happens after the event, it can call the callback directly

*/

#include <functional>
#include <iostream>
#include <mutex>
#include <thread>
#include <unordered_map>
#include <vector>

enum class EventType { DATA_READY, ERROR };

class EventManager {
  public:
    using Callback = std::function<void()>;

    // Register callback
    void registerCallback(EventType type, Callback cb) {
        bool alreadyFired = false;

        {
            std::lock_guard<std::mutex> lock(mtx_);

            if (eventFired_[type]) {
                alreadyFired = true;
            } else {
                callbacks_[type].push_back(cb);
            }
        }

        // If already fired, invoke immediately (non-blocking)
        if (alreadyFired) {
            std::thread(cb).detach();
        }
    }

    // Trigger event
    void triggerEvent(EventType type) {
        std::vector<Callback> local;

        {
            std::lock_guard<std::mutex> lock(mtx_);
            eventFired_[type] = true;

            if (callbacks_.count(type))
                local = callbacks_[type];
        }

        for (auto& cb : local) {
            std::thread(cb).detach();
        }
    }

  private:
    std::unordered_map<EventType, std::vector<Callback>> callbacks_;
    std::unordered_map<EventType, bool> eventFired_;
    std::mutex mtx_;
};

int main() {
    EventManager mgr;

    mgr.triggerEvent(EventType::DATA_READY);

    mgr.registerCallback(EventType::DATA_READY, []() {
        std::cout << "Callback fired immediately because event already happened\n";
    });

    std::this_thread::sleep_for(std::chrono::seconds(1));
}
