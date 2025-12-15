#ifndef PLATFORM_H
#define PLATFORM_H

#include <stdint.h>
#include <time.h>

// ======== 網絡 ========
typedef struct NetSocket NetSocket;

int net_init(void);
void net_shutdown(void);

NetSocket* net_tcp_listen(const char* ip, uint16_t port);
NetSocket* net_accept(NetSocket* server);

int net_send(NetSocket* s, const void* buf, int len);
int net_recv(NetSocket* s, void* buf, int len);

const char* net_get_ip(NetSocket* s);
uint16_t net_get_port(NetSocket* s);

void net_close(NetSocket* s);

// ======== 綫程 ========
typedef struct Thread Thread;
typedef struct Mutex Mutex;
typedef struct Cond Cond;

Thread* thread_create(void (*func)(void*), void* arg);
void thread_join(Thread* t);
void thread_detach(Thread* t);
void thread_free(Thread* t);
void thread_sleep(long ms);

Mutex* mutex_create(void);
void mutex_lock(Mutex* m);
void mutex_unlock(Mutex* m);
void mutex_free(Mutex* m);

Cond* cond_create(void);
void cond_wait(Cond* c, Mutex* m);
void cond_signal(Cond* c);
void cond_broadcast(Cond* c);
void cond_free(Cond* c);

// ======== I/O ========
int mkdirectory(const char* path);

// ======== 時間 ========
int localtime_safe(const time_t* t, struct tm* out_tm);

#endif