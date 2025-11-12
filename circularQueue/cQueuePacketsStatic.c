/*
 * cQueuePacketsStatic.c - Thread-safe Circular Buffer for Network Packets
 *
 * This file implements a fixed-size circular buffer designed for storing
 * network packets with timestamps. Features include:
 * - Thread-safe operations using pthread mutexes
 * - Automatic overwrite of oldest packets when buffer is full
 * - Packet timestamping for debugging/analysis
 * - Static memory allocation (no dynamic allocation)
 *
 * Use case: High-throughput packet processing where dropping packets
 * is acceptable but maintaining recent packet history is important.
 */

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <pthread.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>

#define MAX_PACKET_SIZE 1024
#define BUFFER_CAPACITY 8

typedef struct
{
    uint8_t data[MAX_PACKET_SIZE];
    size_t size;
    struct timespec timestamp;
} Packet;

typedef struct
{
    Packet buffer[BUFFER_CAPACITY];
    int head;
    int tail;
    int is_full;
    pthread_mutex_t lock;
    pthread_cond_t not_empty;
} PacketBuffer;

// Global buffer and control flag
PacketBuffer g_buffer;
volatile int g_running = 1;

// Signal handler for graceful shutdown
void signal_handler(int sig)
{
    g_running = 0;
    pthread_cond_signal(&g_buffer.not_empty);
}

// =============================
// Initialize circular buffer
// =============================
void packet_buffer_init(PacketBuffer *pb)
{
    pb->head = 0;
    pb->tail = 0;
    pb->is_full = 0;
    pthread_mutex_init(&pb->lock, NULL);
    pthread_cond_init(&pb->not_empty, NULL);
}

// =============================
// Push packet data
// =============================
int packet_buffer_push(PacketBuffer *pb, const uint8_t *data, size_t size)
{
    pthread_mutex_lock(&pb->lock);

    if (size > MAX_PACKET_SIZE)
    {
        pthread_mutex_unlock(&pb->lock);
        return -1; // packet too large
    }

    Packet *pkt = &pb->buffer[pb->head];

    // Copy packet data
    memcpy(pkt->data, data, size);
    pkt->size = size;
    clock_gettime(CLOCK_REALTIME, &pkt->timestamp);

    // Move head forward
    pb->head = (pb->head + 1) % BUFFER_CAPACITY;

    // If buffer is full, move tail too (overwrite oldest)
    if (pb->is_full)
        pb->tail = (pb->tail + 1) % BUFFER_CAPACITY;

    pb->is_full = (pb->head == pb->tail);

    // Signal consumer that data is available
    pthread_cond_signal(&pb->not_empty);
    pthread_mutex_unlock(&pb->lock);
    return 0;
}

// =============================
// Pop oldest packet
// =============================
int packet_buffer_pop(PacketBuffer *pb, Packet *out)
{
    pthread_mutex_lock(&pb->lock);

    // Wait for data to be available
    while (pb->head == pb->tail && !pb->is_full && g_running)
    {
        pthread_cond_wait(&pb->not_empty, &pb->lock);
    }

    if (!g_running)
    {
        pthread_mutex_unlock(&pb->lock);
        return -1; // shutting down
    }

    Packet *pkt = &pb->buffer[pb->tail];
    memcpy(out, pkt, sizeof(Packet)); // copy to user

    pb->tail = (pb->tail + 1) % BUFFER_CAPACITY;
    pb->is_full = 0;

    pthread_mutex_unlock(&pb->lock);
    return 0;
}

// =============================
// Producer thread
// =============================
void *producer_thread(void *arg)
{
    int packet_id = 0;

    while (g_running)
    {
        uint8_t payload[8];
        for (int j = 0; j < 8; j++)
            payload[j] = packet_id * 10 + j;

        int ret = packet_buffer_push(&g_buffer, payload, sizeof(payload));
        if (ret == 0)
            printf("Producer: Enqueued packet %d\n", packet_id);
        else
            printf("Producer: Failed to enqueue packet %d\n", packet_id);

        packet_id++;
        usleep(200000); // 200ms delay
    }

    printf("Producer: Exiting\n");
    return NULL;
}

// =============================
// Consumer thread
// =============================
void *consumer_thread(void *arg)
{
    while (g_running)
    {
        Packet pkt;
        if (packet_buffer_pop(&g_buffer, &pkt) == 0)
        {
            printf("Consumer: Dequeued packet size=%zu data=%u\n",
                   pkt.size, pkt.data[0]);
        }
    }

    printf("Consumer: Exiting\n");
    return NULL;
}

// =============================
// Main function
// =============================
int main()
{
    pthread_t producer, consumer;

    // Set up signal handler for Ctrl+C
    signal(SIGINT, signal_handler);

    packet_buffer_init(&g_buffer);

    // Create threads
    pthread_create(&producer, NULL, producer_thread, NULL);
    pthread_create(&consumer, NULL, consumer_thread, NULL);

    printf("Producer-Consumer running infinitely. Press Ctrl+C to stop.\n");

    // Wait for threads to finish
    pthread_join(producer, NULL);
    pthread_join(consumer, NULL);

    printf("Main: Program finished\n");
    return 0;
}