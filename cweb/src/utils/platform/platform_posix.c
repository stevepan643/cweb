#include "utils/platform/platform.h"
#include "utils/log/logger.h"

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include <pthread.h>
#include <time.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <sys/stat.h>

struct NetSocket {
    int sock;
};

int net_init(void)
{
    LOG_INFO("Server starting...");
    return 0;
}

void net_shutdown(void)
{
    LOG_INFO("Server shutting down");
}

NetSocket* net_tcp_listen(const char* ip, uint16_t port)
{
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0)
        return NULL;

    int opt = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port   = htons(port);
    addr.sin_addr.s_addr = ip ? inet_addr(ip) : INADDR_ANY;

    if (bind(fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        close(fd);
        return NULL;
    }

    if (listen(fd, 128) < 0) {
        close(fd);
        return NULL;
    }

    NetSocket* s = malloc(sizeof(NetSocket));
    s->sock = fd;

    LOG_INFO("Listening on %s:%d", ip, port);

    return s;
}

NetSocket* net_accept(NetSocket* server)
{
    if (!server) return NULL;

    int client_fd = accept(server->fd, NULL, NULL);
    if (client_fd < 0)
        return NULL;

    NetSocket* c = malloc(sizeof(NetSocket));
    c->fd = client_fd;
    return c;
}

int net_send(NetSocket* s, const void* buf, int len)
{
    if (!s) return -1;
    return (int)send(s->sock, buf, len, 0);
}

int net_recv(NetSocket* s, void* buf, int len)
{
    if (!s) return -1;
    return (int)recv(s->sock, buf, len, 0);
}

const char* net_get_ip(NetSocket* s)
{
    static char ip_str[INET6_ADDRSTRLEN];
    if (!s) return NULL;

    struct sockaddr_storage addr;
    socklen_t addr_len = sizeof(addr);
    if (getpeername(s->sock, (struct sockaddr*)&addr, &addr_len) == -1)
        return NULL;

    if (addr.ss_family == AF_INET) {
        struct sockaddr_in* s4 = (struct sockaddr_in*)&addr;
        inet_ntop(AF_INET, &s4->sin_addr, ip_str, sizeof(ip_str));
    } else if (addr.ss_family == AF_INET6) {
        struct sockaddr_in6* s6 = (struct sockaddr_in6*)&addr;
        inet_ntop(AF_INET6, &s6->sin6_addr, ip_str, sizeof(ip_str));
    } else {
        return NULL;
    }

    return ip_str;
}

uint16_t net_get_port(NetSocket* s)
{
    if (!s) return 0;

    struct sockaddr_storage addr;
    socklen_t addr_len = sizeof(addr);
    if (getpeername(s->sock, (struct sockaddr*)&addr, &addr_len) == -1)
        return 0;

    if (addr.ss_family == AF_INET) {
        struct sockaddr_in* s4 = (struct sockaddr_in*)&addr;
        return ntohs(s4->sin_port);
    } else if (addr.ss_family == AF_INET6) {
        struct sockaddr_in6* s6 = (struct sockaddr_in6*)&addr;
        return ntohs(s6->sin6_port);
    }

    return 0;
}

void net_close(NetSocket* s)
{
    if (!s) return;
    close(s->sock);
    free(s);
}

struct Thread {
    pthread_t thread;
};

struct Mutex {
    pthread_mutex_t mutex;
};

typedef struct {
    void (*func)(void*);
    void* arg;
} ThreadStartArg;

static void* thread_start(void* arg)
{
    ThreadStartArg* ts = arg;
    ts->func(ts->arg);
    free(ts);
    return NULL;
}

Thread* thread_create(void (*func)(void*), void* arg)
{
    Thread* t = malloc(sizeof(Thread));
    if (!t) return NULL;

    ThreadStartArg* ts = malloc(sizeof(ThreadStartArg));
    if (!ts) {
        free(t);
        return NULL;
    }

    ts->func = func;
    ts->arg  = arg;

    if (pthread_create(&t->thread, NULL, thread_start, ts) != 0) {
        free(ts);
        free(t);
        return NULL;
    }

    return t;
}

void thread_join(Thread* t)
{
    if (!t) return;
    pthread_join(t->thread, NULL);
}

void thread_detach(Thread* t)
{
    if (!t) return;
    pthread_detach(t->thread);
}

void thread_free(Thread* t)
{
    if (!t) return;
    free(t);
}

void thread_sleep(long ms)
{
    if (ms <= 0) return;

    struct timespec ts;
    ts.tv_sec  = ms / 1000;
    ts.tv_nsec = (ms % 1000) * 1000000L;
    nanosleep(&ts, NULL);
}

Mutex* mutex_create(void)
{
    Mutex* m = malloc(sizeof(Mutex));
    if (!m) return NULL;

    pthread_mutex_init(&m->mutex, NULL);
    return m;
}

void mutex_lock(Mutex* m)
{
    if (!m) return;
    pthread_mutex_lock(&m->mutex);
}

void mutex_unlock(Mutex* m)
{
    if (!m) return;
    pthread_mutex_unlock(&m->mutex);
}

void mutex_free(Mutex* m)
{
    if (!m) return;
    pthread_mutex_destroy(&m->mutex);
    free(m);
}

int mkdirectory(const char* path)
{
    if (mkdir(path, 0755) == 0) return 0;
    if (errno == EEXIST) return 0;
    return -1;
}

int localtime_safe(const time_t* t, struct tm* out_tm)
{
    return localtime_r(t, out_tm) ? 0 : -1;
}