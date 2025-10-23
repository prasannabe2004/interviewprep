#include <stdint.h>
#include <stddef.h>
#include <stdio.h>

#define POOL_SIZE 1024

/*
clearing last 2 bits will make it mulitple of 4
but it might size down
do add +3 to make it mulitple of 4 before clearing last 2 bits
*/
#define ALIGN4(x) (((x) + 3) & ~3)  // Align size to 4 bytes

typedef struct mem_block {
    size_t size;              // Size of the block (excluding header)
    uint8_t free;             // 1 = free, 0 = allocated
    struct mem_block *next;   // Pointer to next block
} mem_block_t;

typedef struct {
    uint8_t *pool;            // Pointer to the start of memory pool
    size_t pool_size;         // Total size of the pool
    mem_block_t *head;        // Head of the block list
} mempool_t;

static uint8_t pool_memory[POOL_SIZE];
static mempool_t mypool;

// Initialize memory pool
void mempool_init() {
    mypool.pool = (uint8_t *)pool_memory;
    mypool.pool_size = POOL_SIZE;
    mypool.head = (mem_block_t *)pool_memory;

    mypool.head->size = POOL_SIZE - sizeof(mem_block_t);
    mypool.head->free = 1;
    mypool.head->next = NULL;
}

// Allocate memory (like malloc)
void *mempool_alloc(size_t req_size) {
    size_t size = ALIGN4(req_size);
    mem_block_t *current = mypool.head;

    while (current) {
        if (current->free && current->size >= size) {
            // Split if there’s enough leftover space for another block
            if (current->size >= size + sizeof(mem_block_t) + 4) {
                mem_block_t *new_block = (mem_block_t *)((uint8_t *)current + sizeof(mem_block_t) + size);
                new_block->size = current->size - size - sizeof(mem_block_t);
                new_block->free = 1;
                new_block->next = current->next;

                current->next = new_block;
                current->size = size;
            }

            current->free = 0;
            void *a = (uint8_t *)current + sizeof(mem_block_t);
            //printf("Request to allocate %zu bytes received and returned %p\n", size, a);
            return a;
        }
        current = current->next;
    }

    return NULL; // Out of memory
}

// Free memory (like free)
void mempool_free(void *ptr) {
    //printf("Request to free %p received\n", ptr);
    if (!ptr) return;

    mem_block_t *block = (mem_block_t *)((uint8_t *)ptr - sizeof(mem_block_t));
    block->free = 1;

    // Coalesce adjacent free blocks
    mem_block_t *current = mypool.head;
    while (current && current->next) {
        if (current->free && current->next->free) {
            current->size += sizeof(mem_block_t) + current->next->size;
            current->next = current->next->next;
        } else {
            current = current->next;
        }
    }
}

// Function to visualize memory pool
void visualize_memory_pool() {
    mem_block_t *current = mypool.head;
    int block_num = 0;

    while (current != NULL) {
        printf("Address:%p size:%zu, Free: %s, Next: %p\n", (void*)current, current->size, (current->free ? "Yes" : "No"), (void*)current->next);
        current = current->next;
        block_num++;
    }
}

int main(void) {
    mempool_init();
    void *ptr1 = mempool_alloc(100);
    void *ptr2 = mempool_alloc(200);
    void *ptr3 = mempool_alloc(300);
    visualize_memory_pool();
    mempool_free(ptr3);
    visualize_memory_pool();
    return 0;
}
