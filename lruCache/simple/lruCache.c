// lru_embedded.c
// Embedded-friendly fixed-size LRU cache
// Key: uint32_t, Value: uint32_t
// Uses open-addressing hash table and intrusive doubly-linked list via indices.

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

#define CAPACITY 16                  // max number of entries
#define HT_SIZE  32                  // must be power of two, > CAPACITY*1.5 recommended
#define NULL_IDX (-1)

typedef int16_t idx_t;

typedef struct {
    uint32_t key;
    uint32_t value;
    idx_t prev;   // index in nodes[]
    idx_t next;   // index in nodes[]
    bool used;
} node_t;

typedef struct {
    node_t nodes[CAPACITY];
    idx_t head;   // most recently used
    idx_t tail;   // least recently used
    idx_t free_head; // free list head (index)
    int size;

    // Hash table: stores index into nodes[] or NULL_IDX
    idx_t ht[HT_SIZE];
} lru_cache_t;

// Simple power-of-two mask helper
static inline uint32_t ht_mask(void) { return HT_SIZE - 1; }

// 32-bit integer hash (xorshift mix)
static inline uint32_t hash32(uint32_t x) {
    x ^= x >> 16;
    x *= 0x7feb352d;
    x ^= x >> 15;
    x *= 0x846ca68b;
    x ^= x >> 16;
    return x;
}

// Initialize cache
void lru_init(lru_cache_t *c) {
    memset(c, 0, sizeof(*c));
    for (int i = 0; i < CAPACITY; ++i) {
        c->nodes[i].used = false;
        c->nodes[i].prev = c->nodes[i].next = NULL_IDX;
    }
    // Initialize free list: 0 -> 1 -> 2 -> ...
    c->free_head = 0;
    for (int i = 0; i < CAPACITY; ++i) {
        c->nodes[i].next = (i == CAPACITY-1) ? NULL_IDX : (i+1);
    }
    c->head = c->tail = NULL_IDX;
    c->size = 0;
    for (int i = 0; i < HT_SIZE; ++i) c->ht[i] = NULL_IDX;
}

// Pop an index from free list, return NULL_IDX if none
static idx_t alloc_node(lru_cache_t *c) {
    if (c->free_head == NULL_IDX) return NULL_IDX;
    idx_t i = c->free_head;
    c->free_head = c->nodes[i].next;
    c->nodes[i].next = c->nodes[i].prev = NULL_IDX;
    c->nodes[i].used = true;
    return i;
}

// Push an index onto free list
static void free_node(lru_cache_t *c, idx_t i) {
    c->nodes[i].used = false;
    c->nodes[i].next = c->free_head;
    c->nodes[i].prev = NULL_IDX;
    c->free_head = i;
}

// Hash table find: returns node index for key or NULL_IDX
static idx_t ht_find(lru_cache_t *c, uint32_t key) {
    uint32_t h = hash32(key) & ht_mask();
    for (uint32_t probe = 0; probe < HT_SIZE; ++probe) {
        idx_t slot = c->ht[(h + probe) & ht_mask()];
        if (slot == NULL_IDX) return NULL_IDX; // empty slot -> not found
        if (c->nodes[slot].used && c->nodes[slot].key == key) return slot;
    }
    return NULL_IDX;
}

// Insert into hash table (key -> node_index). Assumes node filled already.
static bool ht_insert(lru_cache_t *c, uint32_t key, idx_t node_idx) {
    uint32_t h = hash32(key) & ht_mask();
    for (uint32_t probe = 0; probe < HT_SIZE; ++probe) {
        idx_t *slot = &c->ht[(h + probe) & ht_mask()];
        if (*slot == NULL_IDX) {
            *slot = node_idx;
            return true;
        }
    }
    return false; // table full — shouldn't happen if HT_SIZE > CAPACITY*2
}

// Remove key from hash table (tombstone-free linear probing removal)
static void ht_remove(lru_cache_t *c, uint32_t key) {
    uint32_t h = hash32(key) & ht_mask();
    for (uint32_t probe = 0; probe < HT_SIZE; ++probe) {
        uint32_t idx = (h + probe) & ht_mask();
        idx_t slot = c->ht[idx];
        if (slot == NULL_IDX) return; // not found
        if (c->nodes[slot].used && c->nodes[slot].key == key) {
            // remove this slot and re-insert the following cluster
            c->ht[idx] = NULL_IDX;
            // re-insert cluster that follows
            uint32_t j = (idx + 1) & ht_mask();
            while (c->ht[j] != NULL_IDX) {
                idx_t re_idx = c->ht[j];
                c->ht[j] = NULL_IDX;
                // re-insert re_idx
                ht_insert(c, c->nodes[re_idx].key, re_idx);
                j = (j + 1) & ht_mask();
            }
            return;
        }
    }
}

// Move node to head (MRU)
static void move_to_head(lru_cache_t *c, idx_t i) {
    if (c->head == i) return;
    idx_t p = c->nodes[i].prev;
    idx_t n = c->nodes[i].next;
    // unlink
    if (p != NULL_IDX) c->nodes[p].next = n;
    if (n != NULL_IDX) c->nodes[n].prev = p;
    if (c->tail == i) c->tail = p;
    // push front
    c->nodes[i].prev = NULL_IDX;
    c->nodes[i].next = c->head;
    if (c->head != NULL_IDX) c->nodes[c->head].prev = i;
    c->head = i;
    if (c->tail == NULL_IDX) c->tail = i;
}

// Remove tail node (LRU) and return its index or NULL_IDX if empty
static idx_t remove_tail(lru_cache_t *c) {
    idx_t t = c->tail;
    if (t == NULL_IDX) return NULL_IDX;
    idx_t p = c->nodes[t].prev;
    if (p != NULL_IDX) c->nodes[p].next = NULL_IDX;
    c->tail = p;
    if (c->tail == NULL_IDX) c->head = NULL_IDX;
    c->nodes[t].prev = c->nodes[t].next = NULL_IDX;
    return t;
}

// Public API: get. Returns true if found and sets *value
bool lru_get(lru_cache_t *c, uint32_t key, uint32_t *value) {
    idx_t idx = ht_find(c, key);
    if (idx == NULL_IDX) return false;
    // move to head
    move_to_head(c, idx);
    if (value) *value = c->nodes[idx].value;
    return true;
}

// Public API: put key/value
bool lru_put(lru_cache_t *c, uint32_t key, uint32_t value) {
    idx_t idx = ht_find(c, key);
    if (idx != NULL_IDX) {
        // update value + move to head
        c->nodes[idx].value = value;
        move_to_head(c, idx);
        return true;
    }
    // need to insert; allocate node (if full, evict LRU)
    if (c->size >= CAPACITY) {
        idx_t ev = remove_tail(c);
        if (ev == NULL_IDX) return false; // shouldn't happen
        // remove from hash table
        ht_remove(c, c->nodes[ev].key);
        // reuse ev as new node
        idx = ev;
        // no change in size
    } else {
        idx = alloc_node(c);
        if (idx == NULL_IDX) return false;
        c->size++;
    }
    // fill node
    c->nodes[idx].key = key;
    c->nodes[idx].value = value;
    c->nodes[idx].prev = c->nodes[idx].next = NULL_IDX;
    // insert into hash table
    if (!ht_insert(c, key, idx)) {
        // Hash table insert failure (shouldn't on correct sizing)
        // put node back to free and return error
        free_node(c, idx);
        c->size--;
        return false;
    }
    // push to head
    if (c->head != NULL_IDX) {
        c->nodes[idx].next = c->head;
        c->nodes[c->head].prev = idx;
    }
    c->head = idx;
    if (c->tail == NULL_IDX) c->tail = idx;
    return true;
}

// For debug: print cache from head->tail
void lru_dump(lru_cache_t *c) {
    printf("LRU dump (MRU->LRU), size=%d\n", c->size);
    idx_t i = c->head;
    while (i != NULL_IDX) {
        printf(" [%d] key=%u val=%u\n", i, (unsigned)c->nodes[i].key, (unsigned)c->nodes[i].value);
        i = c->nodes[i].next;
    }
}

int main(void) {
    lru_cache_t cache;
    lru_init(&cache);

    for (uint32_t k = 1; k <= 18; ++k) {
        lru_put(&cache, k, k*10);
        if (k % 5 == 0) {
            uint32_t v;
            if (lru_get(&cache, k-2, &v)) {
                printf("Got key %u => %u\n", k-2, v);
            } else {
                printf("Key %u not found\n", k-2);
            }
        }
    }
    lru_dump(&cache);
    return 0;
}
