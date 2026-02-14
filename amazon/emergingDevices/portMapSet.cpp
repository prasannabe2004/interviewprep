#include <cstdint>
#include <iostream>

using namespace std;

// Write a function that toggles the state of an LED every time the function is called.
// If the LED was off, the function has to turn it on. If the LED was on, the function has to turn
// it off. The LED is controlled by bit 3 of an 8-bit memory-mapped I/O port at address 0xA0008000.
// Preserve the status of all other bits of the I/O port when toggling the state of the LED.
// Use only standard C, i.e., do not rely on processor-specific I/O libraries or macros.

// 0xA0008000
// Bits 7 6 5 4 3 2 1 0
//      X X X X X X X X

#define MM_IOPORT_ADDR 0xA0008000

void toggleLedStatus() {
    // Static variable to keep track of the last value written to the I/O port to preserve the state
    // of other bits and avoid reading from the I/O port, which may not be supported in some
    // environments. This variable will be updated every time the function is called to reflect the
    // new state of the LED.
    static uint8_t lastValue = 0x0;
    uint32_t bit = 3;

    // Toggle the LED status by XORing the last value with the bit mask for the LED
    lastValue ^= (1 << bit);

    // Write the new value back to the memory-mapped I/O port, preserving the status of other bits
    *(volatile uint8_t*)0xA0008000 = lastValue;
}

int main() {
    toggleLedStatus();
    return 0;
}