#include <stddef.h> // For size_t
#include <stdio.h>  // For error reporting

void* robust_memcpy(void* dest, const void* src, size_t n) {
    if (dest == NULL || src == NULL) {
        // You can choose to return NULL or handle the error more gracefully.
        // Returning NULL is a standard practice for error indication.
        // For a more robust implementation, you might log the error.
        fprintf(stderr, "Error: NULL pointer passed to robust_memcpy\n");
        return NULL;
    }

    char* d = (char*)dest;
    const char* s = (const char*)src;
    printf("d=%p\ts=%p\n", d, s);
/*
Addr:   10  11  12  13  14  15  16  17  18  19
data:   a   b   c   d   e   f   g   h   i   j
index:  0   1   2   3   4   5   6   7   8   9
d<s:    d           s
d>s:    s           d       
*/
    if (d < s) {
        printf("d<s\n");
        // Non-overlapping or non-aliasing case (copy from low to high address)
        // or no overlap. A simple forward loop is most efficient here.
        for (size_t i = 0; i < n; i++) {
            d[i] = s[i];
        }
    } else if (d > s) {
        printf("d>s\n");
        // Overlapping case (copy from high to low address)
        // Start from the end to avoid overwriting source data.
        for (size_t i = n; i > 0; i--) {
            d[i - 1] = s[i - 1];
        }
    } else {
        // dest == src: No-op, do nothing.
    }

    return dest;
}

void* simple_memcpy(void* dest, const void* src, size_t n) {
    if (dest == NULL || src == NULL) {
        // You can choose to return NULL or handle the error more gracefully.
        // Returning NULL is a standard practice for error indication.
        // For a more robust implementation, you might log the error.
        fprintf(stderr, "Error: NULL pointer passed to robust_memcpy\n");
        return NULL;
    }

    char* d = (char*)dest;
    const char* s = (const char*)src;

    for (size_t i = 0; i < n; i++) {
        d[i] = s[i];
    }

    return dest;
}

int main()
{
    char arr[10] = {'a', 'b', 'c', 'd', 'e', 'f','g','h', 'i', 'j'};
    //robust_memcpy(arr+5, arr, 5); // d > s Non overlapping memory between src and dst
    robust_memcpy(arr, arr+3, 5);   // d < s Overlapping memory
    //simple_memcpy(arr+3, arr, 5); // d < s
    printf("src =");
    for(int i = 0; i < 5; i++) {
        printf("%c ", arr[i]);
    }
    printf("\n");
    printf("dst =");
    for(int i = 5; i < 10; i++) {
        printf("%c ", arr[i]);
    }
    printf("\n");
    return 0;
}