#include <chrono>
#include <cstring>
#include <iostream>
#include <thread>
#include <unordered_map>
#include <vector>

using namespace std;
using Clock = chrono::steady_clock;

constexpr size_t MAX_PAYLOAD = 8;

struct FragmentHeader {
    uint32_t message_id;
    uint16_t fragment_id;
    uint16_t total_fragments;
    uint32_t payload_size;
};

struct ReassemblyBuffer {
    uint16_t total_fragments{};
    vector<vector<uint8_t>> fragments;
    vector<bool> received;
    size_t received_count{0};
    Clock::time_point first_receive_time;
};

// =====================================================
// Fragmenter Class
// =====================================================
class Fragmenter {
  public:
    vector<vector<uint8_t>> fragment(const vector<uint8_t>& message, uint32_t message_id) const {
        vector<vector<uint8_t>> fragments;

        size_t total = (message.size() + MAX_PAYLOAD - 1) / MAX_PAYLOAD;

        for (size_t i = 0; i < total; ++i) {
            size_t offset = i * MAX_PAYLOAD;
            size_t len = min(MAX_PAYLOAD, message.size() - offset);

            FragmentHeader hdr{message_id, static_cast<uint16_t>(i), static_cast<uint16_t>(total),
                               static_cast<uint32_t>(len)};

            vector<uint8_t> packet(sizeof(hdr) + len);
            memcpy(packet.data(), &hdr, sizeof(hdr));
            memcpy(packet.data() + sizeof(hdr), message.data() + offset, len);

            fragments.push_back(move(packet));
        }

        return fragments;
    }
};

// =====================================================
// Reassembler Class
// =====================================================
class Reassembler {
  public:
    explicit Reassembler(int timeout_seconds) : timeout_seconds_(timeout_seconds) {
    }

    void processFragment(const vector<uint8_t>& packet) {
        FragmentHeader hdr;
        memcpy(&hdr, packet.data(), sizeof(hdr));

        auto now = Clock::now();
        auto& buffer = reassembly_map_[hdr.message_id];

        if (buffer.fragments.empty()) {
            buffer.total_fragments = hdr.total_fragments;
            buffer.fragments.resize(hdr.total_fragments);
            buffer.received.resize(hdr.total_fragments, false);
            buffer.received_count = 0;
            buffer.first_receive_time = now;
        }

        if (!buffer.received[hdr.fragment_id]) {
            buffer.fragments[hdr.fragment_id] =
                vector<uint8_t>(packet.begin() + sizeof(hdr), packet.end());
            buffer.received[hdr.fragment_id] = true;
            buffer.received_count++;

            cout << "Received fragment " << hdr.fragment_id << "\n";
        }

        if (buffer.received_count == buffer.total_fragments) {
            assemble(hdr.message_id);
        }
    }

    void cleanupExpired() {
        auto now = Clock::now();

        for (auto it = reassembly_map_.begin(); it != reassembly_map_.end();) {

            auto elapsed =
                chrono::duration_cast<chrono::seconds>(now - it->second.first_receive_time);

            if (elapsed.count() > timeout_seconds_) {
                cout << "Message " << it->first << " discarded due to timeout\n\n";
                it = reassembly_map_.erase(it);
            } else {
                ++it;
            }
        }
    }

  private:
    void assemble(uint32_t message_id) {
        auto& buffer = reassembly_map_[message_id];

        vector<uint8_t> full;
        for (size_t i = 0; i < buffer.total_fragments; ++i)
            full.insert(full.end(), buffer.fragments[i].begin(), buffer.fragments[i].end());

        cout << "Reassembled Message: ";
        for (auto c : full)
            cout << c;
        cout << "\n\n";

        reassembly_map_.erase(message_id);
    }

    // Map of message_id to its reassembly buffer
    unordered_map<uint32_t, ReassemblyBuffer> reassembly_map_;
    // Timeout for incomplete messages in seconds
    int timeout_seconds_;
};

// =====================================================
// MAIN DRIVER
// =====================================================
int main() {
    Fragmenter fragmenter;
    Reassembler reassembler(3);

    string msg = "FragmentationDemoExample";
    vector<uint8_t> message(msg.begin(), msg.end());

    // --------------------------------------------------
    // 1️⃣ In-order
    // --------------------------------------------------
    cout << "=== Case 1: In-order delivery ===\n";
    auto frags1 = fragmenter.fragment(message, 1);
    for (auto& f : frags1)
        reassembler.processFragment(f);

    // --------------------------------------------------
    // 2️⃣ Out-of-order
    // --------------------------------------------------
    cout << "=== Case 2: Out-of-order delivery ===\n";
    auto frags2 = fragmenter.fragment(message, 2);

    for (int i = frags2.size() - 1; i >= 0; --i)
        reassembler.processFragment(frags2[i]);

    // --------------------------------------------------
    // 3️⃣ Missing fragment
    // --------------------------------------------------
    cout << "=== Case 3: Missing fragment ===\n";
    auto frags3 = fragmenter.fragment(message, 3);

    for (size_t i = 0; i < frags3.size(); ++i) {
        FragmentHeader hdr;
        memcpy(&hdr, frags3[i].data(), sizeof(hdr));
        if (hdr.fragment_id == 1)
            continue; // simulate loss
        reassembler.processFragment(frags3[i]);
    }

    cout << "Waiting for timeout...\n";
    this_thread::sleep_for(chrono::seconds(4));

    reassembler.cleanupExpired();

    return 0;
}
