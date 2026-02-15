#include <stdio.h>

typedef struct QueueNode {
    int data;
    struct QueueNode* next;
} QueueNode;

typedef struct Queue {
    QueueNode* front;
    QueueNode* rear;
} Queue;

Queue* createQueue() {
    Queue* q = (Queue*)malloc(sizeof(Queue));
    q->front = NULL;
    q->rear = NULL;
    return q;
}

void enqueue(Queue* q, int data) {
    QueueNode* newNode = (QueueNode*)malloc(sizeof(QueueNode));
    newNode->data = data;
    newNode->next = NULL;

    if (q->rear == NULL) {
        q->front = newNode;
        q->rear = newNode;
        return;
    }

    q->rear->next = newNode;
    q->rear = newNode;
}

int dequeue(Queue* q) {
    if (isEmpty(q)) {
        printf("Queue is empty\n");
        return -1;
    }

    QueueNode* temp = q->front;
    int data = temp->data;

    q->front = temp->next;

    if (q->front == NULL) {
        q->rear = NULL;
    }

    free(temp);
    return data;
}

int isEmpty(Queue* q) {
    return q->front == NULL;
}

int front(Queue* q) {
    if (isEmpty(q)) {
        printf("Queue is empty\n");
        return -1;
    }
    return q->front->data;
}
int rear(Queue* q) {
    if (isEmpty(q)) {
        printf("Queue is empty\n");
        return -1;
    }
    return q->rear->data;
}
int main() {
    Queue* q = createQueue();
    enqueue(q, 10);
    enqueue(q, 20);
    enqueue(q, 30);

    printf("Dequeued: %d\n", dequeue(q)); // 10
    printf("Dequeued: %d\n", dequeue(q)); // 20
    printf("Dequeued: %d\n", dequeue(q)); // 30
    printf("Dequeued: %d\n", dequeue(q)); // Queue is empty

    return 0;
}