#include <stdio.h>

void test(char* base) {
    volatile int arr[100];
    char local;
    printf("%ld\n", base - &local);
}

int main() {
    char base;
    test(&base);
}
