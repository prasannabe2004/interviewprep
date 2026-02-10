#include <chrono>
#include <condition_variable>
#include <iostream>
#include <mutex>
#include <thread>

using namespace std;

/*
Phase 1: The "Entry & Wait" (Before and During the Event)

Users 1 and 2 Start:
These threads launch first. One of them (let's say User 1) grabs mutex_. User 2
is blocked at the unique_lock line.

User 1 hits cv_.wait:
It sees event_complete is false. It atomically releases the lock and goes to sleep.

User 2 takes the Lock: Now that the lock is free, User 2 enters, sees false, releases the lock, and
sleeps. The Event Starts: The event_t thread starts. It sleeps for 2 seconds (outside of any lock).

User 3 Arrives:
Even while the event is "running," User 3 calls reg_cb. It grabs the lock, sees event_complete is
still false, releases the lock, and joins Users 1 and 2 in the sleep state. Status: All 3 users are
asleep. The mutex is unlocked.

Phase 2: The "State Change & Notification"

Event Finishes:
The 2-second sleep in event_handler ends.

Updating the Flag:
The event_handler enters the {} block and creates a lock_guard. It locks mutex_, sets event_complete
= true, and prints the completion message.

Lock Release:
As soon as it hits the } brace, the lock_guard is destroyed, and the mutex is released.

The Alarm:
cv_.notify_all() is called. This sends a signal to the OS to wake up Users 1, 2, and 3
simultaneously.

Phase 3: The "Serialization" (The Funnel)
Even though all three users are "awake," your code requires them to finish the cv_.wait call. To
exit that call, a thread must hold the mutex.

The Race: Users 1, 2, and 3 all try to grab mutex_. Only one wins (e.g., User 2).

User 2 Exits Wait: User 2 re-checks the condition [] { return event_complete; }. It is now true.
User 2 prints "Executing reg_cb method."

User 2 Finishes: When reg_cb reaches the end of the function, the
unique_lock is destroyed, releasing the mutex.

The Next User: Now the mutex is free again. User 1 or 3 grabs it, sees the flag is true, executes,
and releases the lock for the final thread.

Direct Access (Post-Event): If a User 4 were to call reg_cb now, they would grab the lock, see
event_complete is already true, and the cv_.wait would return immediately without ever putting the
thread to sleep.

*/
mutex event_mutex;
condition_variable cv_;
bool event_complete = false;

// The method called by multiple users
void reg_cb(int user_id) {
    cout << "User " << user_id << ": called reg_cb" << endl;
    unique_lock<mutex> lock(event_mutex);
    cout << "User " << user_id << ": Got Mutex. Releasing lock and waiting for event." << endl;

    // Wait until the event_complete flag becomes true
    // The wait function atomically releases the lock and blocks the thread.
    // When a notification is received (or on spurious wakeup), it reacquires the lock and checks
    // the predicate.
    cv_.wait(lock, [] { return event_complete; });

    // Once the event is complete, the lock is held again, and reg_cb can execute immediately
    cout << "User " << user_id << ": Event finished. Executing reg_cb method." << endl;
    // ... actual reg_cb logic ...
}

// The function simulating the event
void event_handler() {
    cout << "Event handler: Event started (simulated with sleep for 2 seconds)." << endl;
    this_thread::sleep_for(chrono::seconds(2)); // Simulate event execution time

    // Lock the mutex to modify the shared flag and notify waiting threads
    {
        lock_guard<mutex> lock(event_mutex);
        event_complete = true;
        cout << "Event handler: Event complete. Notifying all waiting users." << endl;
    }

    // Notify all threads waiting on the condition variable
    cv_.notify_all();
}

int main() {
    // Create multiple user threads calling reg_cb at different times
    thread user1(reg_cb, 1);
    thread user2(reg_cb, 2);

    // Simulate some delay before the event starts
    this_thread::sleep_for(chrono::milliseconds(500));

    // Create the event thread
    thread event_t(event_handler);

    thread user3(reg_cb, 3);

    // Join all threads
    user1.join();
    user2.join();
    user3.join();
    event_t.join();

    cout << "Main: All operations finished." << endl;

    return 0;
}
