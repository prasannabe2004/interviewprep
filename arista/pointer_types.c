#include <stdio.h>

int main() {
    int arr[5] = {1, 2, 3, 4, 5};

    printf("Address of arr (int*):     %p\n", (void*)arr);
    printf("Address of &arr (int(*)[5]): %p\n", (void*)&arr);
    printf("Same numeric value? YES\n\n");

    printf("arr + 1:  %p  (increments by sizeof(int) = 4 bytes)\n", (void*)(arr + 1));
    printf("&arr + 1: %p  (increments by sizeof(int[5]) = 20 bytes)\n", (void*)(&arr + 1));
    printf("\nDifference: %ld bytes\n", (char*)(&arr + 1) - (char*)(arr));

    return 0;
}
