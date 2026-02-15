#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct queue {
    int* queue;
    int front;
    int rear;
    int capacity;
} queue_t;

int front(queue_t* q) {
    if (q->front == -1) {
        printf("queue is empty");
        return -1;
    }
    return q->queue[q->front];
}
int rear(queue_t* q) {
    if (q->rear == -1) {
        printf("queue is empty");
        return -1;
    }
    return q->queue[q->rear];
}
uint8_t isFull(queue_t* q) {
    return q->capacity == q->rear + 1;
}
uint8_t isEmpty(queue_t* q) {
    return q->front == -1;
}
queue_t* queueInit(int capacity) {
    queue_t* q = malloc(sizeof(queue_t));
    q->queue = malloc(sizeof(int) * capacity);
    q->front = -1;
    q->rear = -1;
    q->capacity = capacity;
    return q;
}

int enqueue(queue_t* q, int val) {
    if (isFull(q)) {
        printf("queue is full\n");
        return -1;
    }
    if (q->front == -1)
        q->front = 0;
    q->queue[++q->rear] = val;
    return 0;
}

int dequeue(queue_t* q) {
    if (isEmpty(q)) {
        printf("queue is empty\n");
        return -1;
    }
    int v = q->queue[q->front];
    // If the queue has only one element, reset it to empty state
    if (q->front == q->rear) {
        q->front = -1;
        q->rear = -1;
    } else {
        q->front += 1;
    }
    return v;
}
int main() {
    queue_t* q = queueInit(4);
    enqueue(q, 1);
    enqueue(q, 2);
    enqueue(q, 3);
    enqueue(q, 4);
    enqueue(q, 5);              // queue is full
    printf("%d\n", dequeue(q)); // 1
    printf("%d\n", dequeue(q)); // 2
    printf("%d\n", dequeue(q)); // 3
    printf("%d\n", dequeue(q)); // 4
    printf("%d\n", dequeue(q)); // queue is empty
    return 0;
}