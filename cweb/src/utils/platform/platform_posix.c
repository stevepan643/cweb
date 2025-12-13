#include "utils/platform/platform.h"

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

struct NetSocket {
    int fd;
};

int net_init(void)
{
    return 0;
}

void net_shutdown(void)
{
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
    s->fd = fd;
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
    return (int)send(s->fd, buf, len, 0);
}

int net_recv(NetSocket* s, void* buf, int len)
{
    if (!s) return -1;
    return (int)recv(s->fd, buf, len, 0);
}

void net_close(NetSocket* s)
{
    if (!s) return;
    close(s->fd);
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
