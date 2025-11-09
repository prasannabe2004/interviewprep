#include "lru_packet_cache.h"
#include "hash.h"

size_t hash_key(pkt_key_t k)
{
    // simple multiplicative hash, cheap and effective for embedded
    uint32_t x = k;
    x = ((x >> 16) ^ x) * 0x45d9f3b;
    x = ((x >> 16) ^ x) * 0x45d9f3b;
    x = (x >> 16) ^ x;
    return (size_t)(x % HASH_SIZE);
}

void hash_clear(int *table)
{
    for (int i = 0; i < HASH_SIZE; ++i)
        table[i] = -1;
}

int hash_find_index(int *table, pkt_key_t key)
{
    size_t idx = hash_key(key);
    for (int probe = 0; probe < HASH_SIZE; ++probe)
    {
        int slot = (int)((idx + probe) % HASH_SIZE);
        int node_idx = table[slot];
        if (node_idx == -1)
            return -1; // not present
        // if occupied, compare keys
        // caller must ensure node_idx valid
        // but to keep decoupled, we assume node_idx >=0 ; otherwise it's not present
        // comparison will be done by caller using node_idx -> nodes[]
        return slot;
    }
    return -1;
}

// We need a function to look up node index by key (search with probing).
int hash_lookup_node(lru_cache_t *c, pkt_key_t key)
{
    size_t base = hash_key(key);
    for (int probe = 0; probe < HASH_SIZE; ++probe)
    {
        int slot = (int)((base + probe) % HASH_SIZE);
        int node_idx = c->hash_table[slot];
        if (node_idx == -1)
            return -1; // empty => key not in table
        if (c->nodes[node_idx].valid && c->nodes[node_idx].key == key)
            return node_idx;
        // else continue probing
    }
    return -1;
}

bool hash_insert(lru_cache_t *c, pkt_key_t key, int node_index)
{
    size_t base = hash_key(key);
    for (int probe = 0; probe < HASH_SIZE; ++probe)
    {
        int slot = (int)((base + probe) % HASH_SIZE);
        if (c->hash_table[slot] == -1)
        {
            c->hash_table[slot] = node_index;
            return true;
        }
    }
    return false; // table full (should not happen if HASH_SIZE >> CAPACITY)
}

bool hash_remove(lru_cache_t *c, pkt_key_t key)
{
    size_t base = hash_key(key);
    for (int probe = 0; probe < HASH_SIZE; ++probe)
    {
        int slot = (int)((base + probe) % HASH_SIZE);
        int node_idx = c->hash_table[slot];
        if (node_idx == -1)
            return false; // not found
        if (c->nodes[node_idx].valid && c->nodes[node_idx].key == key)
        {
            // remove this slot and reinsert subsequent cluster entries (Robin Hood / linear-probing deletion fix)
            c->hash_table[slot] = -1;
            // reinsert any following items in cluster
            int next_slot = (slot + 1) % HASH_SIZE;
            while (c->hash_table[next_slot] != -1)
            {
                int re_node = c->hash_table[next_slot];
                c->hash_table[next_slot] = -1;
                // reinsert re_node
                hash_insert(c, c->nodes[re_node].key, re_node);
                next_slot = (next_slot + 1) % HASH_SIZE;
            }
            return true;
        }
    }
    return false;
}