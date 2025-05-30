#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <pthread.h>
#include <string.h>
#include <time.h>

#define MAX_PACKET_SIZE 1024

typedef struct {
    uint8_t* data;
    size_t size;
    struct timespec timestamp;
} Packet;

typedef struct {
    Packet** buffer;
    int capacity;
    int head;
    int tail;
    int is_full;

    pthread_mutex_t lock;
} PacketBuffer;

PacketBuffer* create_buffer(int capacity) {
    PacketBuffer* buf = (PacketBuffer*)malloc(sizeof(PacketBuffer));
    buf->capacity = capacity;
    buf->buffer = (Packet**)calloc(capacity, sizeof(Packet*));
    buf->head = 0;
    buf->tail = 0;
    buf->is_full = 0;
    pthread_mutex_init(&buf->lock, NULL);
    return buf;
}

void destroy_buffer(PacketBuffer* buf) {
    pthread_mutex_lock(&buf->lock);
    for (int i = 0; i < buf->capacity; i++) {
        if (buf->buffer[i]) {
            free(buf->buffer[i]->data);
            free(buf->buffer[i]);
        }
    }
    free(buf->buffer);
    pthread_mutex_unlock(&buf->lock);
    pthread_mutex_destroy(&buf->lock);
    free(buf);
}

int enqueue(PacketBuffer* buf, uint8_t* data, size_t size) {
    pthread_mutex_lock(&buf->lock);

    if (buf->is_full) {
        // Overwrite: free oldest packet
        free(buf->buffer[buf->tail]->data);
        free(buf->buffer[buf->tail]);
        buf->tail = (buf->tail + 1) % buf->capacity;
    }

    Packet* pkt = (Packet*)malloc(sizeof(Packet));
    pkt->data = (uint8_t*)malloc(size);
    memcpy(pkt->data, data, size);
    pkt->size = size;
    clock_gettime(CLOCK_MONOTONIC, &pkt->timestamp);

    buf->buffer[buf->head] = pkt;
    buf->head = (buf->head + 1) % buf->capacity;

    buf->is_full = (buf->head == buf->tail);
    pthread_mutex_unlock(&buf->lock);
    return 1;
}

Packet* dequeue(PacketBuffer* buf) {
    pthread_mutex_lock(&buf->lock);
    if (buf->head == buf->tail && !buf->is_full) {
        pthread_mutex_unlock(&buf->lock);
        return NULL; // Empty
    }

    Packet* pkt = buf->buffer[buf->tail];
    buf->buffer[buf->tail] = NULL;
    buf->tail = (buf->tail + 1) % buf->capacity;
    buf->is_full = 0;
    pthread_mutex_unlock(&buf->lock);
    return pkt;
}

void* producer(void* arg) {
    PacketBuffer* buf = (PacketBuffer*)arg;
    for (int i = 0; i < 10; i++) {
        uint8_t sample_data[2] = {i, i+1};
        enqueue(buf, sample_data, sizeof(sample_data));
        printf("Produced packet: [%d, %d]\n", sample_data[0], sample_data[1]);
        usleep(100000); // 100ms
    }
    return NULL;
}

void* consumer(void* arg) {
    PacketBuffer* buf = (PacketBuffer*)arg;
    for (int i = 0; i < 10; i++) {
        Packet* pkt = dequeue(buf);
        if (pkt) {
            printf("Consumed packet of size %zu: [%d, %d]\n",
                   pkt->size, pkt->data[0], pkt->data[1]);
            free(pkt->data);
            free(pkt);
        } else {
            printf("Buffer empty\n");
        }
        usleep(150000); // 150ms
    }
    return NULL;
}

int main() {
    PacketBuffer* buffer = create_buffer(5);

    pthread_t t1, t2;
    pthread_create(&t1, NULL, producer, buffer);
    pthread_create(&t2, NULL, consumer, buffer);

    pthread_join(t1, NULL);
    pthread_join(t2, NULL);

    destroy_buffer(buffer);
    return 0;
}
