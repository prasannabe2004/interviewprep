// lru_packet_cache.c
#include "lru_packet_cache.h"
#include "hash.h"
#include <string.h>

void lru_init(lru_cache_t *c)
{
    memset(c, 0, sizeof(*c));
    hash_clear(c->hash_table);
    c->head = c->tail = -1;
    c->size = 0;
    c->free_count = 0;
    // all nodes invalid initially
    for (int i = 0; i < CAPACITY; ++i)
    {
        c->nodes[i].valid = false;
        c->nodes[i].prev = c->nodes[i].next = -1;
        c->free_list[c->free_count++] = i;
    }
}

// move node to head (most recently used)
static void move_to_head(lru_cache_t *c, int idx)
{
    if (idx == c->head)
        return;
    int p = c->nodes[idx].prev;
    int n = c->nodes[idx].next;

    // unlink
    if (p != -1)
        c->nodes[p].next = n;
    if (n != -1)
        c->nodes[n].prev = p;

    if (idx == c->tail)
        c->tail = p;

    // insert at head
    c->nodes[idx].prev = -1;
    c->nodes[idx].next = c->head;
    if (c->head != -1)
        c->nodes[c->head].prev = idx;
    c->head = idx;
    if (c->tail == -1)
        c->tail = idx;
}

static int allocate_node(lru_cache_t *c)
{
    if (c->free_count > 0)
    {
        return c->free_list[--c->free_count];
    }
    // evict tail if no free nodes
    int evict = c->tail;
    if (evict == -1)
        return -1; // should not happen
    // remove mapping
    pkt_key_t evict_key = c->nodes[evict].key;
    hash_remove(c, evict_key);
    // unlink from LRU list
    int p = c->nodes[evict].prev;
    if (p != -1)
        c->nodes[p].next = -1;
    c->tail = p;
    if (c->tail == -1)
        c->head = -1; // cache became empty
    c->nodes[evict].valid = false;
    c->nodes[evict].prev = c->nodes[evict].next = -1;
    c->size--;
    // now reuse evict index
    return evict;
}

bool lru_put(lru_cache_t *c, pkt_key_t key, const uint8_t *data, uint16_t len)
{
    if (len > MAX_PACKET_SIZE)
        return false; // too large
    // if exists, update and move to head
    int node_idx = hash_lookup_node(c, key);
    if (node_idx != -1)
    {
        // update payload
        memcpy(c->nodes[node_idx].data, data, len);
        c->nodes[node_idx].len = len;
        move_to_head(c, node_idx);
        return true;
    }
    // else allocate node (or evict)
    int idx = allocate_node(c);
    if (idx < 0)
        return false;
    // insert data
    c->nodes[idx].key = key;
    memcpy(c->nodes[idx].data, data, len);
    c->nodes[idx].len = len;
    c->nodes[idx].valid = true;
    c->nodes[idx].prev = c->nodes[idx].next = -1;

    // insert to hash
    if (!hash_insert(c, key, idx))
    {
        // this should rarely happen if HASH_SIZE is sized properly
        c->nodes[idx].valid = false;
        // push back to free_list
        c->free_list[c->free_count++] = idx;
        return false;
    }

    // insert at head
    c->nodes[idx].next = c->head;
    if (c->head != -1)
        c->nodes[c->head].prev = idx;
    c->head = idx;
    if (c->tail == -1)
        c->tail = idx;
    c->size++;
    return true;
}

// copy out and remove from cache
bool lru_get(lru_cache_t *c, pkt_key_t key, uint8_t *out, uint16_t *out_len)
{
    int idx = hash_lookup_node(c, key);
    if (idx == -1)
        return false;
    if (out != NULL && out_len != NULL)
    {
        if (*out_len < c->nodes[idx].len)
            return false; // buffer too small, caller must pass size
        memcpy(out, c->nodes[idx].data, c->nodes[idx].len);
        *out_len = c->nodes[idx].len;
    }
    else if (out_len != NULL)
    {
        *out_len = c->nodes[idx].len;
    }
    // remove from hash
    hash_remove(c, key);
    // unlink from list
    int p = c->nodes[idx].prev;
    int n = c->nodes[idx].next;
    if (p != -1)
        c->nodes[p].next = n;
    if (n != -1)
        c->nodes[n].prev = p;
    if (idx == c->head)
        c->head = n;
    if (idx == c->tail)
        c->tail = p;
    // mark free
    c->nodes[idx].valid = false;
    c->nodes[idx].prev = c->nodes[idx].next = -1;
    c->free_list[c->free_count++] = idx;
    c->size--;
    return true;
}

// peek: copy payload but keep item in cache and move to head (mark as used)
bool lru_peek(lru_cache_t *c, pkt_key_t key, uint8_t *out, uint16_t *out_len)
{
    int idx = hash_lookup_node(c, key);
    if (idx == -1)
        return false;
    if (out != NULL && out_len != NULL)
    {
        if (*out_len < c->nodes[idx].len)
            return false;
        memcpy(out, c->nodes[idx].data, c->nodes[idx].len);
        *out_len = c->nodes[idx].len;
    }
    else if (out_len != NULL)
    {
        *out_len = c->nodes[idx].len;
    }
    move_to_head(c, idx);
    return true;
}

bool lru_remove(lru_cache_t *c, pkt_key_t key)
{
    int idx = hash_lookup_node(c, key);
    if (idx == -1)
        return false;
    // remove mapping
    hash_remove(c, key);
    // unlink
    int p = c->nodes[idx].prev;
    int n = c->nodes[idx].next;
    if (p != -1)
        c->nodes[p].next = n;
    if (n != -1)
        c->nodes[n].prev = p;
    if (idx == c->head)
        c->head = n;
    if (idx == c->tail)
        c->tail = p;
    c->nodes[idx].valid = false;
    c->nodes[idx].prev = c->nodes[idx].next = -1;
    c->free_list[c->free_count++] = idx;
    c->size--;
    return true;
}
