#include <dlfcn.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct mem_entry {
    void* ptr;
    size_t size;
    const char* file;
    int line;
    struct mem_entry* next;
} mem_entry_t;

static mem_entry_t* head = NULL;
static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

/* Real malloc/free function pointers */
static void* (*real_malloc)(size_t) = NULL;
static void (*real_free)(void*) = NULL;
static void* (*real_calloc)(size_t, size_t) = NULL;
static void* (*real_realloc)(void*, size_t) = NULL;

/* Initialize real function pointers */
static void init_real_funcs() {
    if (!real_malloc) {
        real_malloc = (void* (*)(size_t))dlsym(RTLD_NEXT, "malloc");
        real_free = (void (*)(void*))dlsym(RTLD_NEXT, "free");
        real_calloc = (void* (*)(size_t, size_t))dlsym(RTLD_NEXT, "calloc");
        real_realloc = (void* (*)(void*, size_t))dlsym(RTLD_NEXT, "realloc");
    }
}

/* Add entry to tracking list */
static void add_entry(void* ptr, size_t size, const char* file, int line) {
    mem_entry_t* entry = (mem_entry_t*)real_malloc(sizeof(mem_entry_t));
    entry->ptr = ptr;
    entry->size = size;
    entry->file = file;
    entry->line = line;

    pthread_mutex_lock(&lock);
    entry->next = head;
    head = entry;
    pthread_mutex_unlock(&lock);
}

/* Remove entry */
static void remove_entry(void* ptr) {
    pthread_mutex_lock(&lock);

    mem_entry_t** curr = &head;
    while (*curr) {
        if ((*curr)->ptr == ptr) {
            mem_entry_t* tmp = *curr;
            *curr = (*curr)->next;
            real_free(tmp);
            pthread_mutex_unlock(&lock);
            return;
        }
        curr = &(*curr)->next;
    }

    pthread_mutex_unlock(&lock);
    printf("Warning: Double free or invalid pointer %p\n", ptr);
}

/* Leak report */
static void report_leaks() {
    pthread_mutex_lock(&lock);

    mem_entry_t* curr = head;
    if (!curr) {
        printf("No memory leaks detected.\n");
    }

    while (curr) {
        printf("Leak: %zu bytes at %p (allocated at %s:%d)\n", curr->size, curr->ptr, curr->file,
               curr->line);
        curr = curr->next;
    }

    pthread_mutex_unlock(&lock);
}

/* Wrapped malloc */
void* my_malloc(size_t size, const char* file, int line) {
    init_real_funcs();
    void* ptr = real_malloc(size);
    if (ptr)
        add_entry(ptr, size, file, line);
    return ptr;
}

/* Wrapped calloc */
void* my_calloc(size_t nmemb, size_t size, const char* file, int line) {
    init_real_funcs();
    void* ptr = real_calloc(nmemb, size);
    if (ptr)
        add_entry(ptr, nmemb * size, file, line);
    return ptr;
}

/* Wrapped realloc */
void* my_realloc(void* ptr, size_t size, const char* file, int line) {
    init_real_funcs();

    if (ptr)
        remove_entry(ptr);

    void* new_ptr = real_realloc(ptr, size);
    if (new_ptr)
        add_entry(new_ptr, size, file, line);

    return new_ptr;
}

/* Wrapped free */
void my_free(void* ptr) {
    if (!ptr)
        return;

    init_real_funcs();
    remove_entry(ptr);
    real_free(ptr);
}

/* Macros to override */
#define malloc(size) my_malloc(size, __FILE__, __LINE__)
#define calloc(n, s) my_calloc(n, s, __FILE__, __LINE__)
#define realloc(p, s) my_realloc(p, s, __FILE__, __LINE__)
#define free(ptr) my_free(ptr)

/* Register leak report at exit */
__attribute__((constructor)) static void init() {
    atexit(report_leaks);
}

/* ------------------- Example Usage ------------------- */

int main() {

    int* a = (int*)malloc(100);
    int* b = (int*)malloc(200);

    free(a);

    b = (int*)realloc(b, 300);

    // Intentional leak (b not freed)

    return 0;
}
