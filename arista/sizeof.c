#include <stdio.h>

/*
sizeof array without using sizeof operator
*/

int main() {
    int arr[5] = {1, 2, 3, 4, 5};
    printf("Size of array is %lu\n", &arr);
    printf("Size of array is %lu\n", arr);
    printf("Size of array is %lu\n", *(&arr + 1));

    int size = *(&arr + 1) - arr;

    printf("Size of array is in length %d\n", size);
    void* ptr;
    printf("Size of ptr is %lu\n", sizeof(ptr));
    return 0;
}