#define _GNU_SOURCE
#include <liburing.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

#define QUEUE_DEPTH 128
#define BLOCK_SIZE (64 * 1024) // 64 KB blocks

// Simple timer
double now_sec()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec + tv.tv_usec / 1e6;
}

typedef struct
{
    int fd;
    char *buf;
    off_t offset;
} io_task;

void cat_io_uring_multi(int filec, char **files)
{
    struct io_uring ring;
    io_uring_queue_init(QUEUE_DEPTH, &ring, 0);

    int devnull = open("/dev/null", O_WRONLY);

    // Open all files
    io_task *tasks = calloc(filec, sizeof(io_task));
    for (int i = 0; i < filec; i++)
    {
        tasks[i].fd = open(files[i], O_RDONLY);
        if (tasks[i].fd < 0)
        {
            perror("open");
            exit(1);
        }
        tasks[i].buf = malloc(BLOCK_SIZE);
        tasks[i].offset = 0;
    }

    int active = filec;
    while (active > 0)
    {
        // Submit one read per active file
        for (int i = 0; i < filec; i++)
        {
            if (tasks[i].fd < 0)
                continue; // already finished

            struct io_uring_sqe *sqe = io_uring_get_sqe(&ring);
            if (!sqe)
                break;

            io_uring_prep_read(sqe, tasks[i].fd, tasks[i].buf, BLOCK_SIZE, tasks[i].offset);
            sqe->user_data = i; // tag with file index
            tasks[i].offset += BLOCK_SIZE;
        }

        io_uring_submit(&ring);

        // Collect completions
        int completions = 0;
        while (completions < filec)
        {
            struct io_uring_cqe *cqe;
            if (io_uring_peek_cqe(&ring, &cqe) != 0 || !cqe)
                break;

            int idx = cqe->user_data;
            if (cqe->res > 0)
            {
                // Write synchronously to /dev/null
                write(devnull, tasks[idx].buf, cqe->res);
            }
            else
            {
                // EOF or error
                close(tasks[idx].fd);
                tasks[idx].fd = -1;
                active--;
            }

            io_uring_cqe_seen(&ring, cqe);
            completions++;
        }
    }

    for (int i = 0; i < filec; i++)
    {
        free(tasks[i].buf);
    }
    free(tasks);
    close(devnull);
    io_uring_queue_exit(&ring);
}

/* Traditional: sequentially process each file with read()/write() */
static void cat_traditional_multi(int filec, char **files)
{
    int devnull = open("/dev/null", O_WRONLY);
    if (devnull < 0)
    {
        perror("open /dev/null");
        exit(1);
    }

    char *buf = malloc(BLOCK_SIZE);
    if (!buf)
    {
        perror("malloc");
        exit(1);
    }

    for (int i = 0; i < filec; i++)
    {
        int fd = open(files[i], O_RDONLY);
        if (fd < 0)
        {
            perror("open");
            exit(1);
        }

        ssize_t n;
        while ((n = read(fd, buf, BLOCK_SIZE)) > 0)
        {
            ssize_t off = 0;
            while (off < n)
            {
                ssize_t w = write(devnull, buf + off, n - off);
                if (w < 0)
                {
                    if (errno == EINTR)
                        continue;
                    perror("write");
                    close(fd);
                    close(devnull);
                    free(buf);
                    exit(1);
                }
                off += w;
            }
        }
        if (n < 0)
        {
            perror("read");
        }

        close(fd);
    }

    free(buf);
    close(devnull);
}

// Traditional cat using read/write
void cat_traditional(const char *filename)
{
    printf("Traditional cat of %s\n", filename);
    int fd = open(filename, O_RDONLY);
    if (fd < 0)
    {
        perror("open");
        exit(1);
    }

    int devnull = open("/dev/null", O_WRONLY);
    char buf[BLOCK_SIZE];
    ssize_t n;
    while ((n = read(fd, buf, BLOCK_SIZE)) > 0)
    {
        write(devnull, buf, n);
    }
    close(fd);
    close(devnull);
}

// io_uring cat with batched async reads + writes
void cat_io_uring(const char *filename)
{
    printf("io_uring cat of %s\n", filename);
    struct io_uring ring;
    io_uring_queue_init(QUEUE_DEPTH, &ring, 0);

    int fd = open(filename, O_RDONLY);
    if (fd < 0)
    {
        perror("open");
        exit(1);
    }

    int devnull = open("/dev/null", O_WRONLY);

    off_t offset = 0;
    char *buffers[QUEUE_DEPTH];
    for (int i = 0; i < QUEUE_DEPTH; i++)
    {
        buffers[i] = malloc(BLOCK_SIZE);
    }

    while (1)
    {
        int submitted = 0;

        // Queue a batch of reads
        for (int i = 0; i < QUEUE_DEPTH; i++)
        {
            struct io_uring_sqe *sqe = io_uring_get_sqe(&ring);
            if (!sqe)
                break;

            io_uring_prep_read(sqe, fd, buffers[i], BLOCK_SIZE, offset);
            sqe->user_data = i; // tag buffer index
            offset += BLOCK_SIZE;
            submitted++;
        }

        if (submitted == 0)
            break;
        io_uring_submit(&ring);

        // Collect completions and write synchronously
        int completed = 0;
        while (completed < submitted)
        {
            struct io_uring_cqe *cqe;
            io_uring_wait_cqe(&ring, &cqe);

            if (cqe->res <= 0)
            {
                io_uring_cqe_seen(&ring, cqe);
                break;
            }

            int idx = cqe->user_data;
            write(devnull, buffers[idx], cqe->res);

            io_uring_cqe_seen(&ring, cqe);
            completed++;
        }

        // Stop if EOF reached
        if (submitted > 0 && completed < submitted)
        {
            break;
        }
    }

    for (int i = 0; i < QUEUE_DEPTH; i++)
    {
        free(buffers[i]);
    }
    close(fd);
    close(devnull);
    io_uring_queue_exit(&ring);
}

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        fprintf(stderr, "Usage: %s <file>\n", argv[0]);
        return 1;
    }

    double t1 = now_sec();
    cat_traditional_multi(argc - 1, &argv[1]);
    double t2 = now_sec();

    double t3 = now_sec();
    // cat_io_uring(argv[1]);
    cat_io_uring_multi(argc - 1, &argv[1]);
    double t4 = now_sec();

    printf("\n[Traditional] Time: %.6f sec\n", t2 - t1);
    printf("\n[io_uring] Time: %.6f sec\n", t4 - t3);

    return 0;
}
