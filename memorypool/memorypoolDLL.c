#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

// Forward declaration for self-referential struct
struct MemBlock;

typedef struct MemBlock {
    size_t size;
    struct MemBlock* next;
    struct MemBlock* prev;
    bool free;
} MemBlock;

typedef struct {
    uint8_t* pool;
    size_t poolSize;
    MemBlock* head;
} MemPool;

// Function prototypes
void mempool_init(MemPool* mp, size_t size);
void mempool_destroy(MemPool* mp);
void* mempool_alloc(MemPool* mp, size_t size);
void mempool_free(MemPool* mp, void* ptr);
void mempool_visualize(const MemPool* mp);

/**
 * @brief Initializes a memory pool.
 *
 * @param mp Pointer to the MemPool struct.
 * @param size The total size of the memory pool in bytes.
 */
void mempool_init(MemPool* mp, size_t size) {
    if (!mp || size <= sizeof(MemBlock)) {
        if (mp) {
            mp->pool = NULL;
            mp->poolSize = 0;
            mp->head = NULL;
        }
        return;
    }
    mp->poolSize = size;
    mp->pool = (uint8_t*)malloc(mp->poolSize);
    if (!mp->pool) {
        mp->head = NULL;
        mp->poolSize = 0;
        return;
    }

    mp->head = (MemBlock*)mp->pool;
    mp->head->size = mp->poolSize - sizeof(MemBlock);
    mp->head->next = NULL;
    mp->head->prev = NULL;
    mp->head->free = true;
}

/**
 * @brief Destroys a memory pool, freeing its underlying memory.
 *
 * @param mp Pointer to the MemPool struct.
 */
void mempool_destroy(MemPool* mp) {
    if (mp && mp->pool) {
        free(mp->pool);
    }
    if (mp) {
        mp->pool = NULL;
        mp->head = NULL;
        mp->poolSize = 0;
    }
}

/**
 * @brief Allocates a block of memory from the pool.
 *
 * @param mp Pointer to the MemPool struct.
 * @param size The size of the memory to allocate.
 * @return A pointer to the allocated memory, or NULL if allocation fails.
 */
void* mempool_alloc(MemPool* mp, size_t size) {
    if (!mp || !mp->head)
        return NULL;

    MemBlock* current = mp->head;
    while (current != NULL) {
        if (current->free && current->size >= size) {
            // If the block is large enough, split it
            if (current->size > size + sizeof(MemBlock)) {
                MemBlock* newBlock = (MemBlock*)((uint8_t*)current + sizeof(MemBlock) + size);
                newBlock->size = current->size - size - sizeof(MemBlock);
                newBlock->free = true;

                // Link newBlock into the DLL
                newBlock->next = current->next;
                newBlock->prev = current;

                if (current->next != NULL) {
                    current->next->prev = newBlock;
                }
                current->next = newBlock;
                current->size = size;
            }
            current->free = false;
            return (void*)((uint8_t*)current + sizeof(MemBlock));
        }
        current = current->next;
    }
    return NULL; // No suitable block found
}

/**
 * @brief Frees a previously allocated block of memory.
 *
 * This implementation uses the doubly-linked list to perform O(1) coalescing
 * with adjacent free blocks.
 *
 * @param mp Pointer to the MemPool struct.
 * @param ptr Pointer to the memory to free.
 */
void mempool_free(MemPool* mp, void* ptr) {
    if (!ptr || !mp)
        return;

    // Get the block header from the user pointer
    MemBlock* block_to_free = (MemBlock*)((uint8_t*)ptr - sizeof(MemBlock));
    block_to_free->free = true;

    // --- O(1) Coalescing Logic ---

    // 1. Coalesce with the NEXT block if it's free
    if (block_to_free->next && block_to_free->next->free) {
        block_to_free->size += sizeof(MemBlock) + block_to_free->next->size;

        MemBlock* next_of_next = block_to_free->next->next;
        block_to_free->next = next_of_next;
        if (next_of_next) {
            next_of_next->prev = block_to_free;
        }
    }

    // 2. Coalesce with the PREVIOUS block if it's free
    if (block_to_free->prev && block_to_free->prev->free) {
        block_to_free->prev->size += sizeof(MemBlock) + block_to_free->size;

        block_to_free->prev->next = block_to_free->next;
        if (block_to_free->next) {
            block_to_free->next->prev = block_to_free->prev;
        }
        // The 'block_to_free' is now merged into its previous block.
    }
}

/**
 * @brief Prints the current state of the memory pool for debugging.
 *
 * @param mp Pointer to the MemPool struct.
 */
void mempool_visualize(const MemPool* mp) {
    if (!mp)
        return;
    const MemBlock* current = mp->head;
    int i = 0;
    printf("--- Memory Pool State (Doubly-Linked) ---\n");
    while (current != NULL) {
        printf("Block %d: Addr=%p | Size=%-5zu | Free=%-3s | Prev=%p | Next=%p\n", i,
               (void*)current, current->size, current->free ? "Yes" : "No", (void*)current->prev,
               (void*)current->next);
        current = current->next;
        i++;
    }
    printf("-----------------------------------------\n");
}

int main() {
    MemPool pool;
    mempool_init(&pool, 1024);

    printf("Initial Pool:\n");
    mempool_visualize(&pool);

    void* p1 = mempool_alloc(&pool, 100);
    void* p2 = mempool_alloc(&pool, 200);
    void* p3 = mempool_alloc(&pool, 300);

    printf("\nAfter 3 allocations:\n");
    mempool_visualize(&pool);

    // Freeing the middle block p2
    printf("\nFreeing middle block (200 bytes)...\n");
    mempool_free(&pool, p2);
    mempool_visualize(&pool);

    // Freeing p3 should trigger a merge with the free p2 block
    printf("\nFreeing next block (300 bytes) to trigger coalesce...\n");
    mempool_free(&pool, p3);
    mempool_visualize(&pool);

    // Freeing p1 should return everything to one big block
    printf("\nFreeing first block (100 bytes) to trigger full coalesce...\n");
    mempool_free(&pool, p1);
    mempool_visualize(&pool);

    mempool_destroy(&pool);
    return 0;
}
