#include <cstdint>
#include <cstdlib>
#include <iostream>

struct MemBlock {
    size_t size;
    MemBlock* next;
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
    head->free = true;
}

MemPool::~MemPool() {
    delete[] pool;
}

void* MemPool::alloc(size_t size) {
    MemBlock* current = head;
    while (current != nullptr) {
        if (current->free && current->size >= size) {
            if (current->size > size + sizeof(MemBlock)) {
                MemBlock* newBlock = reinterpret_cast<MemBlock*>(
                    reinterpret_cast<uint8_t*>(current) + sizeof(MemBlock) + size);
                newBlock->size = current->size - size - sizeof(MemBlock);
                newBlock->next = current->next;
                newBlock->free = true;
                current->size = size;
                current->next = newBlock;
            }
            current->free = false;
            return reinterpret_cast<void*>(reinterpret_cast<uint8_t*>(current) + sizeof(MemBlock));
        }
        current = current->next;
    }
    return nullptr;
}

void MemPool::free(void* ptr) {
    MemBlock* block =
        reinterpret_cast<MemBlock*>(reinterpret_cast<uint8_t*>(ptr) - sizeof(MemBlock));
    block->free = true;
    MemBlock* current = head;
    while (current != nullptr) {
        if (current->free && current->next != nullptr && current->next->free) {
            current->size += current->next->size + sizeof(MemBlock);
            current->next = current->next->next;
        }
        current = current->next;
    }
}

void MemPool::visualize() const {
    MemBlock* current = head;
    while (current != nullptr) {
        std::cout << "Block size: " << current->size << ", Free: " << (current->free ? "Yes" : "No")
                  << std::endl;
        current = current->next;
    }
}

int main() {
    MemPool pool(1024);
    void* ptr1 = pool.alloc(100);
    void* ptr2 = pool.alloc(200);
    void* ptr3 = pool.alloc(300);
    pool.visualize();
    pool.free(ptr3);
    pool.visualize();
    return 0;
}