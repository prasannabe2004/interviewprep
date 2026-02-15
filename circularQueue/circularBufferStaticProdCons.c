#include <pthread.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

typedef struct {
    int* buffer;
    size_t capacity;
    size_t head;
    size_t tail;
    size_t size;
    pthread_mutex_t lock;
    pthread_cond_t not_full;
    pthread_cond_t not_empty;
} CircularBuffer;

volatile sig_atomic_t keep_running = 1;
CircularBuffer* g_cb = NULL;

void handle_sigint(int sig) {
    keep_running = 0;
    if (g_cb) {
        pthread_cond_broadcast(&g_cb->not_full);
        pthread_cond_broadcast(&g_cb->not_empty);
    }
}

/**
 * @brief Initialize the circular buffer.
 *
 * @param cb Pointer to the CircularBuffer struct.
 * @param capacity Maximum number of elements.
 * @return true if initialization succeeded, false otherwise.
 */
bool cb_init(CircularBuffer* cb, size_t capacity) {
    if (!cb)
        return false;

    cb->buffer = (int*)malloc(capacity * sizeof(int));
    if (!cb->buffer)
        return false;

    cb->capacity = capacity;
    cb->head = 0;
    cb->tail = 0;
    cb->size = 0;
    pthread_mutex_init(&cb->lock, NULL);
    pthread_cond_init(&cb->not_full, NULL);
    pthread_cond_init(&cb->not_empty, NULL);
    return true;
}

/**
 * @brief Free the memory associated with the buffer.
 */
void cb_destroy(CircularBuffer* cb) {
    if (cb && cb->buffer) {
        free(cb->buffer);
        cb->buffer = NULL;
        cb->capacity = 0;
        cb->size = 0;
        pthread_mutex_destroy(&cb->lock);
        pthread_cond_destroy(&cb->not_full);
        pthread_cond_destroy(&cb->not_empty);
    }
}

bool cb_empty(const CircularBuffer* cb) {
    return cb->size == 0;
}

bool cb_full(const CircularBuffer* cb) {
    return cb->size == cb->capacity;
}

/**
 * @brief Add an element to the buffer.
 */
bool cb_push(CircularBuffer* cb, int value) {
    pthread_mutex_lock(&cb->lock);
    while (cb_full(cb) && keep_running) {
        pthread_cond_wait(&cb->not_full, &cb->lock);
    }

    if (!keep_running) {
        pthread_mutex_unlock(&cb->lock);
        return false;
    }

    cb->buffer[cb->head] = value;
    cb->head = (cb->head + 1) % cb->capacity;
    cb->size++;
    pthread_cond_signal(&cb->not_empty);
    pthread_mutex_unlock(&cb->lock);
    return true;
}

/**
 * @brief Remove an element from the buffer.
 */
bool cb_pop(CircularBuffer* cb, int* value) {
    pthread_mutex_lock(&cb->lock);
    while (cb_empty(cb) && keep_running) {
        pthread_cond_wait(&cb->not_empty, &cb->lock);
    }

    if (cb_empty(cb)) {
        pthread_mutex_unlock(&cb->lock);
        return false;
    }

    if (value) {
        *value = cb->buffer[cb->tail];
    }

    cb->tail = (cb->tail + 1) % cb->capacity;
    cb->size--;
    pthread_cond_signal(&cb->not_full);
    pthread_mutex_unlock(&cb->lock);
    return true;
}

void* producer(void* arg) {
    CircularBuffer* cb = (CircularBuffer*)arg;
    int i = 0;
    while (keep_running) {
        i++;
        printf("Producer: Pushing %d\n", i);
        if (!cb_push(cb, i))
            break;
        usleep(100000); // 100ms
    }
    return NULL;
}

void* consumer(void* arg) {
    CircularBuffer* cb = (CircularBuffer*)arg;
    while (keep_running) {
        int val;
        if (!cb_pop(cb, &val))
            break;
        printf("Consumer: Popped %d\n", val);
        usleep(200000); // 200ms
    }
    return NULL;
}

int main() {
    CircularBuffer cb;
    pthread_t prod_thread, cons_thread;

    signal(SIGINT, handle_sigint);

    // Initialize with capacity 5
    if (!cb_init(&cb, 5)) {
        fprintf(stderr, "Failed to initialize buffer\n");
        return 1;
    }

    g_cb = &cb;

    pthread_create(&prod_thread, NULL, producer, &cb);
    pthread_create(&cons_thread, NULL, consumer, &cb);

    pthread_join(prod_thread, NULL);
    pthread_join(cons_thread, NULL);

    cb_destroy(&cb);
    return 0;
}
