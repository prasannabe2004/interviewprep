#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <time.h>

#define BUFFER_SIZE (1024*1024*100)  // 100 MB

static uint8_t buffer[BUFFER_SIZE];

// ----------------------------
// 1. Naive byte-by-byte memset
// ----------------------------
void *memset_naive(void *dst, int val, size_t n) {
    uint8_t *p = (uint8_t *)dst;
    for (size_t i = 0; i < n; i++)
        p[i] = (uint8_t)val;
    return dst;
}

// ----------------------------
// 2. Word-wise optimized memset
// ----------------------------
void *memset_fast(void *dst, int val, size_t n) {
    uint8_t *p = (uint8_t *)dst;
    uint8_t v = (uint8_t)val;

    // Build word pattern
    uintptr_t pattern = v;
    pattern |= pattern << 8;
    pattern |= pattern << 16;
#if UINTPTR_MAX > 0xFFFFFFFF
    pattern |= pattern << 32;
#endif

    // Align to word
    while (n && ((uintptr_t)p & (sizeof(uintptr_t) - 1))) {
        *p++ = v;
        n--;
    }

    // Word-wise write
    uintptr_t *wp = (uintptr_t *)p;
    while (n >= sizeof(uintptr_t)) {
        *wp++ = pattern;
        n -= sizeof(uintptr_t);
    }

    // Remaining bytes
    p = (uint8_t *)wp;
    while (n--) *p++ = v;

    return dst;
}

// ----------------------------
// Timer helper
// ----------------------------
double time_sec() {
    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC, &t);
    return t.tv_sec + t.tv_nsec / 1e9;
}

// ----------------------------
// Benchmark function
// ----------------------------
void benchmark(const char *name, void *(*func)(void*, int, size_t)) {
    double start = time_sec();
    func(buffer, 0xAA, BUFFER_SIZE);
    double end = time_sec();
    double elapsed = end - start;
    double mbps = (BUFFER_SIZE / (1024.0*1024.0)) / elapsed;
    printf("%-20s: %.6f s, %.2f MB/s\n", name, elapsed, mbps);

    // Prevent compiler optimizing out memset
    volatile uint8_t sink = buffer[0];
    (void)sink;
}

// ----------------------------
// Main
// ----------------------------
int main() {
    printf("Benchmarking memset for %zu MB buffer\n", BUFFER_SIZE/(1024*1024));

    benchmark("Naive byte", memset_naive);
    benchmark("Word-wise fast", memset_fast);
    benchmark("Standard memset", memset);

    return 0;
}
