#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <pthread.h>
#include <time.h>

#define MAX_PACKET_SIZE 1024
#define BUFFER_CAPACITY 8

typedef struct {
    uint8_t data[MAX_PACKET_SIZE];
    size_t size;
    struct timespec timestamp;
} Packet;

typedef struct {
    Packet buffer[BUFFER_CAPACITY];
    int head;
    int tail;
    int is_full;
    pthread_mutex_t lock;
} PacketBuffer;

// =============================
// Initialize circular buffer
// =============================
void packet_buffer_init(PacketBuffer* pb) {
    pb->head = 0;
    pb->tail = 0;
    pb->is_full = 0;
    pthread_mutex_init(&pb->lock, NULL);
}

// =============================
// Push packet data
// =============================
int packet_buffer_push(PacketBuffer* pb, const uint8_t* data, size_t size) {
    pthread_mutex_lock(&pb->lock);

    if (size > MAX_PACKET_SIZE) {
        pthread_mutex_unlock(&pb->lock);
        return -1; // packet too large
    }

    Packet* pkt = &pb->buffer[pb->head];

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

    pthread_mutex_unlock(&pb->lock);
    return 0;
}

// =============================
// Pop oldest packet
// =============================
int packet_buffer_pop(PacketBuffer* pb, Packet* out) {
    pthread_mutex_lock(&pb->lock);

    if (pb->head == pb->tail && !pb->is_full) {
        pthread_mutex_unlock(&pb->lock);
        return -1; // empty
    }

    Packet* pkt = &pb->buffer[pb->tail];
    memcpy(out, pkt, sizeof(Packet)); // copy to user

    pb->tail = (pb->tail + 1) % BUFFER_CAPACITY;
    pb->is_full = 0;

    pthread_mutex_unlock(&pb->lock);
    return 0;
}

// =============================
// Example usage
// =============================
int main() {
    PacketBuffer pb;
    packet_buffer_init(&pb);

    // Producer
    for (int i = 0; i < 10; i++) {
        uint8_t payload[8];
        for (int j = 0; j < 8; j++)
            payload[j] = i * 10 + j;

        int ret = packet_buffer_push(&pb, payload, sizeof(payload));
        if (ret == 0)
            printf("Enqueued packet %d with data %d\n", i, payload[0]);
        else
            printf("Failed to enqueue packet %d\n", i);
    }

    // Consumer
    for (int i = 0; i < 10; i++) {
        Packet pkt;
        if (packet_buffer_pop(&pb, &pkt) == 0)
            printf("Dequeued packet size=%zu data=%u\n",
                   pkt.size, pkt.data[0]);
        else
            printf("Buffer empty at %d\n", i);
    }

    return 0;
}
