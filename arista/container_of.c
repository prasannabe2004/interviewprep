#include <stddef.h> // Required for offsetof
#include <stdio.h>

struct student {
    char name[20];
    int roll_no;
    int course_code;
    int marks;
};

/*
https://medium.com/@abhi1kush/understanding-container-of-macro-in-c-1fce3114b419
*/
int main() {
    struct student stu;
    int* pMarks = &stu.marks; // Pointer to `marks`

    // Approach 1 - Using manual offset calculation
    // int offset = (char*)&stu.marks - (char*)&stu;
    // struct student* pStu = (struct student*)((char*)pMarks - offset);

    // Approach 2 - Generalize
    /*
        struct student* pStu = ({
            const typeof(((struct student*)0)->marks)* __mptr = (pMarks);
            (struct student*)((char*)__mptr - __builtin_offsetof(struct student, marks));
        });
    */

    // Approach 3 - Make it a Macro
    // Get the address of `stu` using `pMarks`

#define container_of(ptr, type, member)                                                            \
    ({                                                                                             \
        const typeof(((type*)0)->member)* __mptr = (ptr);                                          \
        (type*)((char*)__mptr - offsetof(type, member));                                           \
    })
    struct student* pStu = container_of(pMarks, struct student, marks);

    if (pStu == &stu) {
        printf("Successfully retrieved struct address using container_of!\n");
    }
}