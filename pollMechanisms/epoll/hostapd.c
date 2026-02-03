// epoll_netlink_unix_timer.c
// Compile: gcc -O2 -Wall -o epoll_demo epoll_netlink_unix_timer.c
// Run: sudo ./epoll_demo
//
// Notes: run as root for full NETLINK_ROUTE notifications.
// Clean up: rm -f /tmp/epoll-demo.sock

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <sys/timerfd.h>
#include <sys/signalfd.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <arpa/inet.h>

#define EPOLL_MAX_EVENTS 10
#define UNIX_SOCKET_PATH "/tmp/epoll-demo.sock"
#define TIMER_INTERVAL_SEC 5

static int make_nonblocking(int fd)
{
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1)
        return -1;
    return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

/* Create and bind a NETLINK_ROUTE socket to receive link/address notifications */
static int setup_netlink_socket(void)
{
    int nl = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE);
    if (nl < 0)
    {
        perror("socket(AF_NETLINK)");
        return -1;
    }

    struct sockaddr_nl sa;
    memset(&sa, 0, sizeof(sa));
    sa.nl_family = AF_NETLINK;
    sa.nl_groups = RTMGRP_LINK | RTMGRP_IPV4_IFADDR | RTMGRP_IPV6_IFADDR; // listen groups

    if (bind(nl, (struct sockaddr *)&sa, sizeof(sa)) < 0)
    {
        perror("bind(netlink)");
        close(nl);
        return -1;
    }

    if (make_nonblocking(nl) < 0)
    {
        perror("make_nonblocking(netlink)");
        close(nl);
        return -1;
    }
    return nl;
}

/* Create UNIX domain stream listening socket */
static int setup_unix_socket(const char *path)
{
    int us = socket(AF_UNIX, SOCK_STREAM, 0);
    if (us < 0)
    {
        perror("socket(AF_UNIX)");
        return -1;
    }

    unlink(path); // remove previous socket if present

    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, path, sizeof(addr.sun_path) - 1);

    if (bind(us, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        perror("bind(unix)");
        close(us);
        return -1;
    }

    if (listen(us, 5) < 0)
    {
        perror("listen(unix)");
        close(us);
        return -1;
    }

    if (make_nonblocking(us) < 0)
    {
        perror("make_nonblocking(unix)");
        close(us);
        return -1;
    }

    return us;
}

/* Create a periodic timerfd */
static int setup_timerfd(int sec_interval)
{
    int tfd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
    if (tfd < 0)
    {
        perror("timerfd_create");
        return -1;
    }

    struct itimerspec it;
    it.it_interval.tv_sec = sec_interval;
    it.it_interval.tv_nsec = 0;
    it.it_value.tv_sec = sec_interval;
    it.it_value.tv_nsec = 0;

    if (timerfd_settime(tfd, 0, &it, NULL) < 0)
    {
        perror("timerfd_settime");
        close(tfd);
        return -1;
    }
    return tfd;
}

/* Setup signalfd for SIGINT/SIGTERM */
static int setup_signalfd(void)
{
    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGINT);
    sigaddset(&mask, SIGTERM);
    if (sigprocmask(SIG_BLOCK, &mask, NULL) < 0)
    {
        perror("sigprocmask");
        return -1;
    }
    int sfd = signalfd(-1, &mask, SFD_NONBLOCK | SFD_CLOEXEC);
    if (sfd < 0)
    {
        perror("signalfd");
        return -1;
    }
    return sfd;
}

/* Read and print netlink messages (simple parser: prints message types) */
static void handle_netlink(int nl_fd)
{
    char buf[4096];
    ssize_t len;
    while ((len = recv(nl_fd, buf, sizeof(buf), 0)) > 0)
    {
        struct nlmsghdr *nh;
        for (nh = (struct nlmsghdr *)buf; NLMSG_OK(nh, (size_t)len); nh = NLMSG_NEXT(nh, len))
        {
            if (nh->nlmsg_type == NLMSG_DONE)
                break;
            if (nh->nlmsg_type == NLMSG_ERROR)
            {
                fprintf(stderr, "netlink: NLMSG_ERROR\n");
                continue;
            }
            // Print brief info about netlink message type
            switch (nh->nlmsg_type)
            {
            case RTM_NEWLINK:
                printf("[netlink] RTM_NEWLINK\n");
                break;
            case RTM_DELLINK:
                printf("[netlink] RTM_DELLINK\n");
                break;
            case RTM_NEWADDR:
                printf("[netlink] RTM_NEWADDR\n");
                break;
            case RTM_DELADDR:
                printf("[netlink] RTM_DELADDR\n");
                break;
            default:
                printf("[netlink] msg type=%d len=%u\n", nh->nlmsg_type, nh->nlmsg_len);
            }
        }
    }
    if (len < 0 && errno != EAGAIN && errno != EWOULDBLOCK)
    {
        perror("recv(netlink)");
    }
}

/* Accept and read a single client connection on UNIX socket (non-blocking) */
static void handle_unix_listener(int us_fd, int epfd)
{
    while (1)
    {
        int c = accept(us_fd, NULL, NULL);
        if (c < 0)
        {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
                break;
            perror("accept");
            break;
        }
        make_nonblocking(c);
        // Register client socket for read events
        struct epoll_event ev;
        ev.events = EPOLLIN | EPOLLET;
        ev.data.fd = c;
        if (epoll_ctl(epfd, EPOLL_CTL_ADD, c, &ev) < 0)
        {
            perror("epoll_ctl add client");
            close(c);
        }
        else
        {
            printf("[unix] accepted client fd=%d\n", c);
        }
    }
}

/* Read from a connected unix client socket */
static void handle_unix_client(int client_fd)
{
    char buf[512];
    while (1)
    {
        ssize_t n = read(client_fd, buf, sizeof(buf));
        if (n > 0)
        {
            printf("[unix client %d] read %zd bytes: %.*s\n", client_fd, n, (int)n, buf);
        }
        else if (n == 0)
        {
            printf("[unix client %d] closed\n", client_fd);
            close(client_fd);
            break;
        }
        else
        {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
                break;
            perror("read(unix client)");
            close(client_fd);
            break;
        }
    }
}

int main(void)
{
    int epfd = epoll_create1(0);
    if (epfd < 0)
    {
        perror("epoll_create1");
        return 1;
    }

    int nl = setup_netlink_socket();
    if (nl < 0)
        return 1;
    int us = setup_unix_socket(UNIX_SOCKET_PATH);
    if (us < 0)
    {
        close(nl);
        return 1;
    }
    int tfd = setup_timerfd(TIMER_INTERVAL_SEC);
    if (tfd < 0)
    {
        close(nl);
        close(us);
        return 1;
    }
    int sfd = setup_signalfd();
    if (sfd < 0)
    {
        close(nl);
        close(us);
        close(tfd);
        return 1;
    }

    // Register fds with epoll
    struct epoll_event ev;
    ev.events = EPOLLIN;
    ev.data.fd = nl;
    epoll_ctl(epfd, EPOLL_CTL_ADD, nl, &ev);

    ev.events = EPOLLIN;
    ev.data.fd = us;
    epoll_ctl(epfd, EPOLL_CTL_ADD, us, &ev);

    ev.events = EPOLLIN;
    ev.data.fd = tfd;
    epoll_ctl(epfd, EPOLL_CTL_ADD, tfd, &ev);

    ev.events = EPOLLIN;
    ev.data.fd = sfd;
    epoll_ctl(epfd, EPOLL_CTL_ADD, sfd, &ev);

    printf("epoll demo started. netlink fd=%d unix=%s listener fd=%d timerfd=%d signalfd=%d\n",
           nl, UNIX_SOCKET_PATH, us, tfd, sfd);
    printf("Test netlink: `ip link set dev <if> down` or `ip addr add/del` (needs root)\n");
    printf("Test unix:  `echo hello | socat - UNIX-CONNECT:%s` or `nc -U %s`\n", UNIX_SOCKET_PATH, UNIX_SOCKET_PATH);
    printf("Timer interval: %d sec\n", TIMER_INTERVAL_SEC);

    struct epoll_event events[EPOLL_MAX_EVENTS];
    int running = 1;
    while (running)
    {
        int n = epoll_wait(epfd, events, EPOLL_MAX_EVENTS, -1);
        if (n < 0)
        {
            if (errno == EINTR)
                continue;
            perror("epoll_wait");
            break;
        }
        for (int i = 0; i < n; ++i)
        {
            int fd = events[i].data.fd;
            uint32_t evts = events[i].events;
            if (fd == nl)
            {
                handle_netlink(nl);
            }
            else if (fd == us)
            {
                handle_unix_listener(us, epfd);
            }
            else if (fd == tfd)
            {
                uint64_t expirations;
                ssize_t r = read(tfd, &expirations, sizeof(expirations));
                if (r == sizeof(expirations))
                {
                    printf("[timer] fired %llu times\n", (unsigned long long)expirations);
                }
            }
            else if (fd == sfd)
            {
                struct signalfd_siginfo si;
                ssize_t r = read(sfd, &si, sizeof(si));
                if (r == sizeof(si))
                {
                    printf("[signal] received signal %u\n", si.ssi_signo);
                    running = 0;
                    break;
                }
            }
            else
            {
                // other fds are unix client sockets
                if (evts & (EPOLLIN | EPOLLHUP | EPOLLERR))
                {
                    handle_unix_client(fd);
                }
            }
        }
    }

    printf("shutting down...\n");
    close(nl);
    close(us);
    unlink(UNIX_SOCKET_PATH);
    close(tfd);
    close(sfd);
    close(epfd);
    return 0;
}
