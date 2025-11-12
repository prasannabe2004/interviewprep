#include "lru_packet_cache.h"

#ifndef HASH_SIZE
#error "HASH_SIZE must be defined and > CAPACITY"
#endif

size_t hash_key(pkt_key_t k);

void hash_clear(int *table);

int hash_find_index(int *table, pkt_key_t key);

// We need a function to look up node index by key (search with probing).
int hash_lookup_node(lru_cache_t *c, pkt_key_t key);

bool hash_insert(lru_cache_t *c, pkt_key_t key, int node_index);

bool hash_remove(lru_cache_t *c, pkt_key_t key);