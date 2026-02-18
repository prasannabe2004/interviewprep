#include <stdio.h>
#include <stdlib.h>

#define CAPACITY 3

struct Node {
    int key, value;
    struct Node *prev, *next;
};

struct LRUCache {
    int size, capacity;
    struct Node *head, *tail;
    struct Node** hash; // Array for hash table
};
struct LRUCache* createCache(int capacity) {
    struct LRUCache* cache = (struct LRUCache*)malloc(sizeof(struct LRUCache));
    cache->size = 0;
    cache->capacity = capacity;
    cache->head = (struct Node*)malloc(sizeof(struct Node));
    cache->tail = (struct Node*)malloc(sizeof(struct Node));
    cache->head->next = cache->tail;
    cache->tail->prev = cache->head;
    cache->hash = (struct Node**)calloc(100, sizeof(struct Node*)); // Simple hash
    return cache;
}
void moveToHead(struct LRUCache* cache, struct Node* node) {
    node->prev->next = node->next;
    node->next->prev = node->prev;
    node->next = cache->head->next;
    node->prev = cache->head;
    cache->head->next->prev = node;
    cache->head->next = node;
}
int get(struct LRUCache* cache, int key) {
    int idx = key % 10000;
    struct Node* node = cache->hash[idx];
    if (node) {
        moveToHead(cache, node);
        return node->value;
    }
    return -1;
}
void put(struct LRUCache* cache, int key, int value) {
    int idx = key % 10000;
    struct Node* node = cache->hash[idx];
    if (node) {
        node->value = value;
        moveToHead(cache, node);
    } else {
        node = (struct Node*)malloc(sizeof(struct Node));
        node->key = key;
        node->value = value;
        cache->hash[idx] = node;
        moveToHead(cache, node);
        cache->size++;
        if (cache->size > cache->capacity) {
            struct Node* lru = cache->tail->prev;
            lru->prev->next = cache->tail;
            cache->tail->prev = lru->prev;
            cache->hash[lru->key % 10000] = NULL;
            free(lru);
            cache->size--;
        }
    }
}

int main() {
    struct LRUCache* cache = createCache(CAPACITY);
    put(cache, 1, 100);
    put(cache, 2, 200);
    put(cache, 3, 300);
    printf("Get 1: %d\n", get(cache, 1)); // 100
    put(cache, 4, 400);                   // Evicts 2
    printf("Get 2: %d\n", get(cache, 2)); // -1
    return 0;
}