/*
 * cQueuePacketsDynamic.c - Dynamic Circular Buffer for Network Packets
 *
 * This file implements a dynamically allocated circular buffer for storing
 * network packets with variable-size payloads. Key features include:
 * - Dynamic memory allocation for buffer and packet data
 * - Thread-safe operations using pthread mutexes
 * - Configurable buffer capacity at runtime
 * - Automatic overwrite of oldest packets when buffer is full
 * - Packet timestamping for analysis
 * - Memory management with proper cleanup
 *
 * Use case: Applications requiring flexible buffer sizes and variable
 * packet lengths where memory efficiency is important and buffer size
 * needs to be determined at runtime.
 */

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <pthread.h>
#include <string.h>
#include <time.h>

#define MAX_PACKET_SIZE 1024

typedef struct
{
    uint8_t *data;
    size_t size;
    struct timespec timestamp;
} Packet;

typedef struct
{
    Packet **buffer;
    int capacity;
    int head;
    int tail;
    int is_full;
    volatile int running;

    pthread_mutex_t lock;
    pthread_cond_t not_empty;
} PacketBuffer;

PacketBuffer *create_buffer(int capacity)
{
    PacketBuffer *buf = (PacketBuffer *)malloc(sizeof(PacketBuffer));
    buf->capacity = capacity;
    buf->buffer = (Packet **)calloc(capacity, sizeof(Packet *));
    buf->head = 0;
    buf->tail = 0;
    buf->is_full = 0;
    buf->running = 1;
    pthread_mutex_init(&buf->lock, NULL);
    pthread_cond_init(&buf->not_empty, NULL);
    return buf;
}

void destroy_buffer(PacketBuffer *buf)
{
    pthread_mutex_lock(&buf->lock);
    for (int i = 0; i < buf->capacity; i++)
    {
        if (buf->buffer[i])
        {
            free(buf->buffer[i]->data);
            free(buf->buffer[i]);
        }
    }
    free(buf->buffer);
    pthread_mutex_unlock(&buf->lock);
    pthread_mutex_destroy(&buf->lock);
    pthread_cond_destroy(&buf->not_empty);
    free(buf);
}

int enqueue(PacketBuffer *buf, uint8_t *data, size_t size)
{
    pthread_mutex_lock(&buf->lock);

    if (buf->is_full)
    {
        // Overwrite: free oldest packet
        free(buf->buffer[buf->tail]->data);
        free(buf->buffer[buf->tail]);
        buf->tail = (buf->tail + 1) % buf->capacity;
    }

    Packet *pkt = (Packet *)malloc(sizeof(Packet));
    pkt->data = (uint8_t *)malloc(size);
    memcpy(pkt->data, data, size);
    pkt->size = size;
    clock_gettime(CLOCK_MONOTONIC, &pkt->timestamp);

    buf->buffer[buf->head] = pkt;
    buf->head = (buf->head + 1) % buf->capacity;

    buf->is_full = (buf->head == buf->tail);

    // Signal consumer that data is available
    pthread_cond_signal(&buf->not_empty);
    pthread_mutex_unlock(&buf->lock);
    return 1;
}

Packet *dequeue(PacketBuffer *buf)
{
    pthread_mutex_lock(&buf->lock);

    // Wait for data to be available
    while (buf->head == buf->tail && !buf->is_full && buf->running)
    {
        pthread_cond_wait(&buf->not_empty, &buf->lock);
    }

    if (!buf->running)
    {
        pthread_mutex_unlock(&buf->lock);
        return NULL; // shutting down
    }

    Packet *pkt = buf->buffer[buf->tail];
    buf->buffer[buf->tail] = NULL;
    buf->tail = (buf->tail + 1) % buf->capacity;
    buf->is_full = 0;
    pthread_mutex_unlock(&buf->lock);
    return pkt;
}

void *producer(void *arg)
{
    PacketBuffer *buf = (PacketBuffer *)arg;
    for (int i = 0; i < 10; i++)
    {
        uint8_t sample_data[2] = {i, i + 1};
        enqueue(buf, sample_data, sizeof(sample_data));
        printf("Produced packet: [%d, %d]\n", sample_data[0], sample_data[1]);
        usleep(100000); // 100ms
    }
    return NULL;
}

void *consumer(void *arg)
{
    PacketBuffer *buf = (PacketBuffer *)arg;
    for (int i = 0; i < 10; i++)
    {
        Packet *pkt = dequeue(buf);
        if (pkt)
        {
            printf("Consumed packet of size %zu: [%d, %d]\n",
                   pkt->size, pkt->data[0], pkt->data[1]);
            free(pkt->data);
            free(pkt);
        }
    }
    return NULL;
}

int main()
{
    PacketBuffer *buffer = create_buffer(5);

    pthread_t t1, t2;
    pthread_create(&t1, NULL, producer, buffer);
    pthread_create(&t2, NULL, consumer, buffer);

    pthread_join(t1, NULL);

    // Signal consumer to stop waiting
    buffer->running = 0;
    pthread_cond_signal(&buffer->not_empty);

    pthread_join(t2, NULL);

    destroy_buffer(buffer);
    return 0;
}