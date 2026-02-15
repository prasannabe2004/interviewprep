#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

/*
Create a simple memory pool allocator in C. The memory pool will be a fixed-size block of memory
from which we can allocate and free smaller blocks. The allocator should manage the memory
efficiently, allowing for allocation and deallocation of variable-sized blocks.

Key features to implement:
1. Initialization: Create a memory pool of a specified size.
2. Allocation: Implement a function to allocate a block of memory from the pool. The function should
take the requested size as an argument and return a pointer to the allocated block.
3. Deallocation: Implement a function to free a previously allocated block of memory, allowing it to
be reused for future allocations.
4. Visualization: Implement a function to visualize the current state of the memory pool, showing
allocated and free blocks.
5. Coalescing: When freeing a block, if adjacent blocks are also free, they should be coalesced into
a single larger block to reduce fragmentation.


*/
#define POOL_SIZE 1024

/*
clearing last 2 bits will make it mulitple of 4
but it might size down
do add +3 to make it mulitple of 4 before clearing last 2 bits
*/
#define ALIGN4(x) (((x) + 3) & ~3) // Align size to 4 bytes

typedef struct mem_block {
    size_t size;            // Size of the block (excluding header)
    uint8_t free;           // 1 = free, 0 = allocated
    struct mem_block* next; // Pointer to next block
} mem_block_t;

typedef struct {
    uint8_t* pool;     // Pointer to the start of memory pool
    size_t pool_size;  // Total size of the pool
    mem_block_t* head; // Head of the block list
} mempool_t;

static uint8_t pool_memory[POOL_SIZE];

// Initialize memory pool
void mempool_init(mempool_t* pool) {
    pool->pool = (uint8_t*)pool_memory;
    pool->pool_size = POOL_SIZE;
    pool->head = (mem_block_t*)pool_memory;

    pool->head->size = POOL_SIZE - sizeof(mem_block_t);
    pool->head->free = 1;
    pool->head->next = NULL;
}

// Allocate memory (like malloc)
void* mempool_alloc(mempool_t* pool, size_t req_size) {
    size_t size = ALIGN4(req_size);
    mem_block_t* current = pool->head;

    while (current) {
        if (current->free && current->size >= size) {
            // Split if there’s enough leftover space for another block
            if (current->size >= size + sizeof(mem_block_t) + 4) {
                mem_block_t* new_block =
                    (mem_block_t*)((uint8_t*)current + sizeof(mem_block_t) + size);
                new_block->size = current->size - size - sizeof(mem_block_t);
                new_block->free = 1;
                new_block->next = current->next;

                current->next = new_block;
                current->size = size;
            }

            current->free = 0;
            void* a = (uint8_t*)current + sizeof(mem_block_t);
            // printf("Request to allocate %zu bytes received and returned %p\n", size, a);
            return a;
        }
        current = current->next;
    }

    return NULL; // Out of memory
}

// Free memory (like free)
void mempool_free(mempool_t* pool, void* ptr) {
    // printf("Request to free %p received\n", ptr);
    if (!ptr)
        return;

    mem_block_t* block = (mem_block_t*)((uint8_t*)ptr - sizeof(mem_block_t));
    block->free = 1;

    // Coalesce adjacent free blocks
    mem_block_t* current = pool->head;
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
void visualize_memory_pool(mempool_t* pool) {
    mem_block_t* current = pool->head;
    int block_num = 0;

    while (current != NULL) {
        printf("Address:%p size:%zu, Free: %s, Next: %p\n", (void*)current, current->size,
               (current->free ? "Yes" : "No"), (void*)current->next);
        current = current->next;
        block_num++;
    }
}

int main(void) {
    static mempool_t mypool;
    mempool_init(&mypool);
    void* ptr1 = mempool_alloc(&mypool, 100);
    void* ptr2 = mempool_alloc(&mypool, 200);
    void* ptr3 = mempool_alloc(&mypool, 300);
    visualize_memory_pool(&mypool);
    mempool_free(&mypool, ptr3);
    visualize_memory_pool(&mypool);
    return 0;
}
