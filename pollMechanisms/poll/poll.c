// poll_demo.c
// Compile: gcc -O2 -o poll_demo poll_demo.c
// Run: ./poll_demo [num_fds=1000] [num_writers=10] [writer_delay_ms=50]

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <poll.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>

static long now_ms()
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
}

int main(int argc, char **argv)
{
    int N = argc > 1 ? atoi(argv[1]) : 1000;
    int M = argc > 2 ? atoi(argv[2]) : 10;
    int delay = argc > 3 ? atoi(argv[3]) : 50;

    if (N <= 0)
        N = 1000;
    if (M <= 0)
        M = 10;

    int (*fds)[2] = malloc(sizeof(int[2]) * N);
    if (!fds)
    {
        perror("malloc");
        return 1;
    }
    for (int i = 0; i < N; i++)
        if (pipe(fds[i]) < 0)
        {
            perror("pipe");
            return 1;
        }

    for (int w = 0; w < M; w++)
    {
        pid_t p = fork();
        if (p < 0)
        {
            perror("fork");
            return 1;
        }
        if (p == 0)
        {
            usleep(delay * 1000);
            int start = (w * N) / M;
            int end = ((w + 1) * N) / M;
            for (int i = start; i < end; i++)
            {
                char c = 'a' + (w % 26);
                write(fds[i][1], &c, 1);
            }
            _exit(0);
        }
    }

    struct pollfd *pfds = calloc(N, sizeof(struct pollfd));
    for (int i = 0; i < N; i++)
    {
        pfds[i].fd = fds[i][0];
        pfds[i].events = POLLIN;
    }

    long t0 = now_ms();
    int rv = poll(pfds, N, -1); // block indefinitely
    long t1 = now_ms();

    if (rv < 0)
        perror("poll");
    else
    {
        printf("poll returned %d ready fds in %ld ms\n", rv, t1 - t0);
        int seen = 0;
        for (int i = 0; i < N && seen < rv; i++)
        {
            if (pfds[i].revents & POLLIN)
            {
                char b;
                read(pfds[i].fd, &b, 1);
                seen++;
            }
        }
    }

    for (int i = 0; i < N; i++)
    {
        close(fds[i][0]);
        close(fds[i][1]);
    }
    free(pfds);
    free(fds);
    while (waitpid(-1, NULL, WNOHANG) > 0)
    {
    }
    return 0;
}
