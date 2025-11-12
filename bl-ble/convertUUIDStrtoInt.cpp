/*
 * uuid128.cpp — 128-bit UUID Parser (C++17 single file)
 *
 * Features:
 *   - Parses 128-bit UUID strings only
 *   - Ignores dashes, spaces, and case
 *   - Returns as struct { uint64_t hi, lo }
 *   - Includes helper functions for printing and comparison
 *
 * Author: ChatGPT (OpenAI)
 */

#include <cctype>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>

/* ---------- Struct Definition ---------- */
struct UUID128 {
    uint64_t hi;  // most significant 64 bits
    uint64_t lo;  // least significant 64 bits
};

/* ---------- Helper Functions ---------- */
static inline uint8_t hexNibble(char c) {
    if (c >= '0' && c <= '9') return static_cast<uint8_t>(c - '0');
    if (c >= 'a' && c <= 'f') return static_cast<uint8_t>(c - 'a' + 10);
    if (c >= 'A' && c <= 'F') return static_cast<uint8_t>(c - 'A' + 10);
    throw std::invalid_argument("Invalid hex character in UUID");
}

/* ---------- Main Parse Function ---------- */
bool parseUUID128(const std::string& input, UUID128& out) {
    unsigned __int128 value = 0;
    std::string hex;
    hex.reserve(32);

    // Collect only hex digits
    for (char c : input)
        if (std::isxdigit(static_cast<unsigned char>(c))) hex.push_back(c);

    // Only accept 128-bit UUIDs (32 hex characters)
    if (hex.size() != 32) return false;

    uint64_t hi = 0, lo = 0;
    try {
        for (int i = 0; i < 16; ++i) hi = (hi << 4) | hexNibble(hex[i]);
        for (int i = 16; i < 32; ++i) lo = (lo << 4) | hexNibble(hex[i]);
    } catch (...) {
        return false;
    }

    out.hi = hi;
    out.lo = lo;
    return true;
}

/* ---------- Utility: Print as lowercase hex (no dashes) ---------- */
std::string uuidToString(const UUID128& u) {
    std::ostringstream oss;
    oss << std::hex << std::setfill('0') << std::nouppercase;
    oss << std::setw(16) << u.hi << std::setw(16) << u.lo;
    return oss.str();
}

/* ---------- Utility: Comparison ---------- */
bool operator==(const UUID128& a, const UUID128& b) {
    return a.hi == b.hi && a.lo == b.lo;
}
bool operator!=(const UUID128& a, const UUID128& b) { return !(a == b); }

int main() {
    const std::string tests[] = {
        "550e8400-e29b-41d4-a716-446655440000",  // 128-bit
        "0000180d-0000-1000-8000-00805f9b34fb",  // 128-bit
        "550e8400e29b41d4a716446655440000",      // 128-bit no dashes
        "180D",                                  // invalid (too short)
        "12345678"                               // invalid (too short)
    };

    for (const auto& s : tests) {
        UUID128 u{};
        if (parseUUID128(s, u)) {
            std::cout << "Input: " << std::setw(40) << std::left << s << " -> "
                      << uuidToString(u) << "\n";
        } else {
            std::cout << "Input: " << s << " -> parse error\n";
        }
    }

    // Example comparison
    UUID128 a, b;
    parseUUID128("550e8400-e29b-41d4-a716-446655440000", a);
    parseUUID128("550e8400e29b41d4a716446655440000", b);
    std::cout << "\nEqual? " << (a == b ? "YES" : "NO") << "\n";
}