// select_small.c
// Simple demo that monitors only 3 or 5 fds with select()
// Compile: gcc -O2 -o select_small select_small.c
// Usage: ./select_small [3|5]

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/select.h>
#include <sys/wait.h>
#include <time.h>

static long now_ms(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
}

int main(int argc, char **argv)
{
    int N = 3; // default
    if (argc > 1)
    {
        int v = atoi(argv[1]);
        if (v == 3 || v == 5)
            N = v;
        else
        {
            fprintf(stderr, "Only 3 or 5 allowed — defaulting to 3\n");
            N = 3;
        }
    }

    int pipes[N][2];
    for (int i = 0; i < N; ++i)
    {
        if (pipe(pipes[i]) < 0)
        {
            perror("pipe");
            return 1;
        }
    }

    // single writer child: wait briefly, then write to all write-ends
    pid_t p = fork();
    if (p < 0)
    {
        perror("fork");
        return 1;
    }
    if (p == 0)
    {
        usleep(50 * 1000); // 50 ms
        for (int i = 0; i < N; ++i)
        {
            char c = 'A' + (char)i;
            if (write(pipes[i][1], &c, 1) < 0)
            { /* ignore write errors */
            }
            // close write end in child after writing
            close(pipes[i][1]);
        }
        _exit(0);
    }

    // parent: close writer ends, monitor read ends with select()
    for (int i = 0; i < N; ++i)
        close(pipes[i][1]);

    int maxfd = 0;
    fd_set readset;
    FD_ZERO(&readset);
    for (int i = 0; i < N; ++i)
    {
        FD_SET(pipes[i][0], &readset);
        if (pipes[i][0] > maxfd)
            maxfd = pipes[i][0];
    }

    long t0 = now_ms();
    int rv = select(maxfd + 1, &readset, NULL, NULL, NULL);
    long t1 = now_ms();

    if (rv < 0)
    {
        perror("select");
    }
    else
    {
        printf("select returned %d ready fds in %ld ms\n", rv, t1 - t0);
        for (int i = 0; i < N; ++i)
        {
            if (FD_ISSET(pipes[i][0], &readset))
            {
                char b = 0;
                ssize_t r = read(pipes[i][0], &b, 1);
                printf("  fd %d ready; read '%c' (bytes=%zd)\n", pipes[i][0], (r > 0 ? b : '?'), r);
            }
        }
    }

    // cleanup
    for (int i = 0; i < N; ++i)
        close(pipes[i][0]);
    // reap child
    waitpid(p, NULL, 0);
    return 0;
}
