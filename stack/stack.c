#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct stack {
    int* stack;
    int top;
    int capacity;
} stack_t;

stack_t* stackInit(int capacity) {
    stack_t* s = malloc(sizeof(stack_t));
    s->stack = malloc(sizeof(int) * capacity);
    s->top = -1;
    s->capacity = capacity;
    return s;
}

uint8_t isFull(stack_t* s) {
    return s->capacity == s->top;
}
int push(stack_t* s, int val) {
    if (isFull(s)) {
        printf("stack is full");
        return -1;
    }
    s->top += 1;
    s->stack[s->top] = val;
    s->capacity += 1;
    return 0;
}
uint8_t isEmpty(stack_t* s) {
    return s->top == -1;
}
int pop(stack_t* s) {
    if (isEmpty(s)) {
        printf("stack is empty");
        return -1;
    }
    int v = s->stack[s->top];
    s->top -= 1;
    s->capacity -= 1;
    return v;
}
int main() {
    stack_t* s = stackInit(10);
    push(s, 1);
    push(s, 2);
    push(s, 3);
    push(s, 4);
    push(s, 5);

    printf("%d\n", pop(s)); // 5
    printf("%d\n", pop(s)); // 4
    printf("%d\n", pop(s)); // 3
    printf("%d\n", pop(s)); // 2
    printf("%d\n", pop(s)); // 1
    printf("%d\n", pop(s)); // stack is empty

    return 0;
}