#include <stdio.h>
#include <stdlib.h>

#include "libshared.h"

int main_global = 100; // Main .data segment

int main() {                   // Main .text segment
    int local_var = 5;         // Stack
    int* dynamic = malloc(16); // 'dynamic' is on Stack, points to Heap

    dynamic[0] = 0;
    printf("local variable = %d\n", local_var);
    printf("global variable = %d\n", main_global);
    printf("dynamic variable = %d\n", dynamic[0]);

    library_api(); // Calls into Library .text
    return 0;
}