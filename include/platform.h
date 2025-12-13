#ifndef PLATFORM_H
#define PLATFORM_H

#include <stdint.h>

// ======== 網絡 ========
typedef struct NetSocket NetSocket;

int net_init(void);
void net_shutdown(void);

NetSocket* net_tcp_listen(const char* ip, uint16_t port);
NetSocket* net_accept(NetSocket* server);

int net_send(NetSocket* s, const void* buf, int len);
int net_recv(NetSocket* s, void* buf, int len);

void net_close(NetSocket* s);

// ======== 綫程 ========
typedef struct Thread Thread;
typedef struct Mutex Mutex;

Thread* thread_create(void (*func)(void*), void* arg);
void thread_join(Thread* t);
void thread_detach(Thread* t);
void thread_free(Thread* t);
void thread_sleep(long ms);

Mutex* mutex_create(void);
void mutex_lock(Mutex* m);
void mutex_unlock(Mutex* m);
void mutex_free(Mutex* m);

#endif