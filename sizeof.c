#include <stdio.h>

/*
sizeof array without using sizeof operator
*/

int main()
{
    int arr[5] = {1, 2, 3, 4, 5};
    printf("Size of array is %d\n", &arr);
    printf("Size of array is %d\n", *(&arr+1));
    int size = *(&arr + 1) - arr;
    printf("Size of array is %d\n", size);
    return 0;
}