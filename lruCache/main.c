#include <stdio.h>
#include <string.h>
#include "lru_packet_cache.h"

static void dump_cache_state(lru_cache_t *c)
{
    printf("Cache size: %d\n", c->size);
    printf("LRU order (head -> tail): ");
    int idx = c->head;
    while (idx != -1)
    {
        printf("%u ", c->nodes[idx].key);
        idx = c->nodes[idx].next;
    }
    printf("\n");
}

int main(void)
{
    lru_cache_t cache;
    lru_init(&cache);

    printf("==== Inserting packets ====\n");
    for (uint32_t i = 1; i <= 5; i++)
    {
        uint8_t payload[8];
        snprintf((char *)payload, sizeof(payload), "P%u", i);
        lru_put(&cache, i, payload, strlen((char *)payload) + 1);
    }
    dump_cache_state(&cache);

    printf("\n==== Access packet 2 (should become most recent) ====\n");
    uint8_t buf[64];
    uint16_t len = sizeof(buf);
    if (lru_peek(&cache, 2, buf, &len))
    {
        printf("Peeked key=2 data=%s\n", buf);
    }
    dump_cache_state(&cache);

    printf("\n==== Insert more packets to cause eviction ====\n");
    for (uint32_t i = 6; i <= 8; i++)
    {
        uint8_t payload[8];
        snprintf((char *)payload, sizeof(payload), "P%u", i);
        lru_put(&cache, i, payload, strlen((char *)payload) + 1);
    }
    dump_cache_state(&cache);

    printf("\n==== Consume packet 4 (remove from cache) ====\n");
    len = sizeof(buf);
    if (lru_get(&cache, 4, buf, &len))
    {
        printf("Got key=4, data=%s\n", buf);
    }
    else
    {
        printf("Key=4 not found\n");
    }
    dump_cache_state(&cache);

    printf("\n==== Insert packet 9 ====\n");
    uint8_t payload[8];
    snprintf((char *)payload, sizeof(payload), "P9");
    lru_put(&cache, 9, payload, strlen((char *)payload) + 1);
    dump_cache_state(&cache);

    printf("\n==== Try to get packet 1 (might be evicted) ====\n");
    len = sizeof(buf);
    if (lru_get(&cache, 1, buf, &len))
        printf("Got key=1, data=%s\n", buf);
    else
        printf("Key=1 not found (evicted)\n");

    dump_cache_state(&cache);

    printf("\n==== Test complete ====\n");
    return 0;
}
