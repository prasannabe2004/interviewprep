#include <stdio.h>
int main() {
    unsigned int x = 0xF000000;           // 4 leading zeros
    int leading_zeros = __builtin_clz(x); // Intrinsic for count leading zeros
    printf("Leading zeros in %x: %d\n", x, leading_zeros);
    unsigned long y = 42;
    unsigned long pop_count = __builtin_popcountl(y); // Count set bits
    printf("Set bits in %lu: %lu\n", y, pop_count);
    return 0;
}