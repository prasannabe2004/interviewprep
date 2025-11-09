// lru_packet_cache.h
#ifndef LRU_PACKET_CACHE_H
#define LRU_PACKET_CACHE_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#define CAPACITY          64      // number of entries in cache
#define MAX_PACKET_SIZE   1536    // max packet payload size
// Hash table size should be > CAPACITY, prefer power-of-two for fast mod if you want.
#define HASH_SIZE         131     // prime or power-of-two; > CAPACITY * 1.5 recommended

typedef uint32_t pkt_key_t; // key for the packet (seq number, flow id etc.)

typedef struct {
    pkt_key_t key;
    uint8_t data[MAX_PACKET_SIZE];
    uint16_t len;
    bool valid;
    int prev; // index of previous node in LRU list, -1 none
    int next; // index of next node in LRU list, -1 none
} lru_node_t;

typedef struct {
    lru_node_t nodes[CAPACITY];
    int hash_table[HASH_SIZE]; // maps hash slot -> node index, -1 empty
    int head; // most recently used index, -1 none
    int tail; // least recently used index, -1 none
    int free_count;
    int size;
    int free_list[CAPACITY]; // indices of unused nodes
} lru_cache_t;

// API
void lru_init(lru_cache_t *c);
bool lru_put(lru_cache_t *c, pkt_key_t key, const uint8_t *data, uint16_t len);
// lru_get removes the entry from cache and copies payload into 'out' (user-provided buffer).
// Returns true if found and copied, false otherwise.
bool lru_get(lru_cache_t *c, pkt_key_t key, uint8_t *out, uint16_t *out_len);
// lru_peek copies payload but keeps entry in cache (moves to head). Returns true if found.
bool lru_peek(lru_cache_t *c, pkt_key_t key, uint8_t *out, uint16_t *out_len);
// optional: remove without copying
bool lru_remove(lru_cache_t *c, pkt_key_t key);

#endif // LRU_PACKET_CACHE_H
