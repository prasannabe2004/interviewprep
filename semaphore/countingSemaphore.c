#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

typedef struct {
    int count;
    pthread_mutex_t lock;
    pthread_cond_t cond;
} semaphore_t;

/* ---------- Semaphore APIs ---------- */

void sem_init_custom(semaphore_t* sem, int initial_value) {
    sem->count = initial_value;
    pthread_mutex_init(&sem->lock, NULL);
    pthread_cond_init(&sem->cond, NULL);
}

void sem_wait_custom(semaphore_t* sem) {
    pthread_mutex_lock(&sem->lock);

    while (sem->count == 0) {
        pthread_cond_wait(&sem->cond, &sem->lock);
    }

    sem->count--;
    pthread_mutex_unlock(&sem->lock);
}

void sem_post_custom(semaphore_t* sem) {
    pthread_mutex_lock(&sem->lock);

    sem->count++;
    pthread_cond_signal(&sem->cond);

    pthread_mutex_unlock(&sem->lock);
}

void sem_destroy_custom(semaphore_t* sem) {
    pthread_mutex_destroy(&sem->lock);
    pthread_cond_destroy(&sem->cond);
}

/* ---------- Example Usage ---------- */

#define NUM_THREADS 5

semaphore_t sem;

void* worker(void* arg) {
    int id = *(int*)arg;

    printf("Thread %d waiting...\n", id);

    sem_wait_custom(&sem); // acquire

    printf("Thread %d entered critical section\n", id);
    sleep(2); // simulate work
    printf("Thread %d leaving critical section\n", id);

    sem_post_custom(&sem); // release

    return NULL;
}

int main() {
    pthread_t threads[NUM_THREADS];
    int ids[NUM_THREADS];

    // Allow only 2 threads at a time
    sem_init_custom(&sem, 2);

    for (int i = 0; i < NUM_THREADS; i++) {
        ids[i] = i;
        pthread_create(&threads[i], NULL, worker, &ids[i]);
    }

    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    sem_destroy_custom(&sem);

    return 0;
}
