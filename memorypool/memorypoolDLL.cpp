#include <cstdint>
#include <cstdlib>
#include <iostream>

struct MemBlock {
    size_t size;
    MemBlock* next;
    MemBlock* prev; // Added for O(1) coalescing
    bool free;
};

class MemPool {
  private:
    uint8_t* pool;
    size_t poolSize;
    MemBlock* head;

  public:
    MemPool(size_t size);
    ~MemPool();

    void* alloc(size_t size);
    void free(void* ptr);
    void visualize() const;
};

MemPool::MemPool(size_t size) {
    poolSize = size;
    pool = new uint8_t[poolSize];

    head = reinterpret_cast<MemBlock*>(pool);
    head->size = poolSize - sizeof(MemBlock);
    head->next = nullptr;
    head->prev = nullptr; // Initialize prev
    head->free = true;
}

MemPool::~MemPool() {
    delete[] pool;
}

void* MemPool::alloc(size_t size) {
    MemBlock* current = head;

    while (current != nullptr) {
        if (current->free && current->size >= size) {
            // Check if we can split the block
            if (current->size > size + sizeof(MemBlock)) {
                MemBlock* newBlock = reinterpret_cast<MemBlock*>(
                    reinterpret_cast<uint8_t*>(current) + sizeof(MemBlock) + size);

                newBlock->size = current->size - size - sizeof(MemBlock);
                newBlock->free = true;

                // Link newBlock into the DLL
                newBlock->next = current->next;
                newBlock->prev = current;

                // update current to link yo newBlock
                if (current->next) {
                    current->next->prev = newBlock;
                }
                current->next = newBlock;
                current->size = size;
            }

            current->free = false;
            // Return pointer to memory right after the header
            return reinterpret_cast<void*>(reinterpret_cast<uint8_t*>(current) + sizeof(MemBlock));
        }
        current = current->next;
    }
    return nullptr; // Out of memory
}

void MemPool::free(void* ptr) {
    if (!ptr)
        return;

    // Move pointer back to get to the header
    MemBlock* block =
        reinterpret_cast<MemBlock*>(reinterpret_cast<uint8_t*>(ptr) - sizeof(MemBlock));
    block->free = true;

    // O(1) COALESCING LOGIC

    // 1. Merge with NEXT block if it exists and is free
    if (block->next && block->next->free) {
        block->size += sizeof(MemBlock) + block->next->size;

        MemBlock* afterNext = block->next->next;
        block->next = afterNext;
        if (afterNext) {
            afterNext->prev = block;
        }
    }

    // 2. Merge with PREVIOUS block if it exists and is free
    if (block->prev && block->prev->free) {
        block->prev->size += sizeof(MemBlock) + block->size;

        block->prev->next = block->next;
        if (block->next) {
            block->next->prev = block->prev;
        }
        // Note: 'block' pointer is now invalid as it's merged into 'block->prev'
    }
}

void MemPool::visualize() const {
    std::cout << "--- Memory Pool State ---" << std::endl;
    MemBlock* current = head;
    while (current != nullptr) {
        std::cout << "[Addr: " << current << " | Size: " << current->size
                  << " | Free: " << (current->free ? "YES" : "NO") << " | Prev: " << current->prev
                  << " | Next: " << current->next << "]" << std::endl;
        current = current->next;
    }
    std::cout << "-------------------------" << std::endl;
}

int main() {
    // Initialize pool with 1KB
    MemPool pool(1024);

    std::cout << "Initial Pool:" << std::endl;
    pool.visualize();

    void* p1 = pool.alloc(100);
    void* p2 = pool.alloc(200);
    void* p3 = pool.alloc(300);

    std::cout << "\nAfter 3 allocations:" << std::endl;
    pool.visualize();

    // Freeing the middle block p2
    std::cout << "\nFreeing middle block (200 bytes)..." << std::endl;
    pool.free(p2);
    pool.visualize();

    // Freeing p3 should trigger a merge with the free p2 block
    std::cout << "\nFreeing next block (300 bytes) to trigger coalesce..." << std::endl;
    pool.free(p3);
    pool.visualize();

    // Freeing p1 should return everything to one big block
    std::cout << "\nFreeing first block (100 bytes) to trigger full coalesce..." << std::endl;
    pool.free(p1);
    pool.visualize();

    return 0;
}