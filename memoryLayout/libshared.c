int lib_global_init = 42;       // Library .data segment
int lib_global_uninit;          // Library .bss segment
const char* lib_name = "MyLib"; // Library .rodata segment

void library_api() {           // Library .text segment
    static int call_count = 0; // Library .data segment
    call_count++;
}