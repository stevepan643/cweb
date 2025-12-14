#include "utils/platform/platform.h"
#include "utils/log/logger.h"

// ======== 網絡 ========
#include <stdlib.h>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <direct.h>

struct NetSocket
{
    SOCKET sock;
};

int net_init(void)
{
    LOG_INFO("Server starting...");
    WSADATA wsa;
    return WSAStartup(MAKEWORD(2, 2), &wsa);
}
void net_shutdown(void)
{
    LOG_INFO("Server shutting down");
    WSACleanup();
}

NetSocket* net_tcp_listen(const char* ip, uint16_t port)
{
    SOCKET s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (s == INVALID_SOCKET) return NULL;

    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    inet_pton(AF_INET, ip, &addr.sin_addr);

    bind(s, (struct sockaddr*)&addr, sizeof(addr));
    listen(s, 16);

    NetSocket* ns = malloc(sizeof(NetSocket));
    ns->sock = s;

    LOG_INFO("Listening on %s:%d", ip, port);

    return ns;
}
NetSocket* net_accept(NetSocket* server)
{
    SOCKET s = accept(server->sock, NULL, NULL);
    if (s == INVALID_SOCKET) return NULL;

    NetSocket* ns = malloc(sizeof(NetSocket));
    ns->sock = s;
    return ns;
}

int net_send(NetSocket* s, const void* buf, int len)
{
    return send(s->sock, buf, len, 0);
}
int net_recv(NetSocket* s, void* buf, int len)
{
    return recv(s->sock, buf, len, 0);
}


const char* net_get_ip(NetSocket* s)
{
    static char ip_str[INET6_ADDRSTRLEN]; // IPv4/IPv6 通用
    if (!s) return NULL;

    struct sockaddr_storage addr;
    int addr_len = sizeof(addr);
    if (getpeername(s->sock, (struct sockaddr*)&addr, &addr_len) == SOCKET_ERROR) {
        return NULL;
    }

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
    int addr_len = sizeof(addr);
    if (getpeername(s->sock, (struct sockaddr*)&addr, &addr_len) == SOCKET_ERROR) {
        return 0;
    }

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
    closesocket(s->sock);
    free(s);
}

// ======== 綫程 ========
struct Thread
{
    HANDLE handle;
    DWORD thread_id;
};

struct Mutex
{
    CRITICAL_SECTION cs;
};

typedef struct
{
    void (*func)(void*);
    void* arg;
} ThreadStartArg;

static DWORD WINAPI thread_start(LPVOID lpParam)
{
    ThreadStartArg* ts = (ThreadStartArg*)lpParam;
    ts->func(ts->arg);
    free(ts);
    return 0;
}

Thread* thread_create(void (*func)(void*), void* arg)
{
    Thread* t = (Thread*)malloc(sizeof(Thread));
    if (!t) return NULL;

    ThreadStartArg* ts = (ThreadStartArg*)malloc(sizeof(ThreadStartArg));
    if (!ts) { free(t); return NULL; }

    ts->func = func;
    ts->arg = arg;

    t->handle = CreateThread(
        NULL,            // 默认安全属性
        0,               // 默认堆栈大小
        thread_start,    // 线程函数
        ts,              // 参数
        0,               // 创建后立即运行
        &t->thread_id
    );

    if (!t->handle) {
        free(ts);
        free(t);
        return NULL;
    }

    return t;
}

void thread_join(Thread* t)
{
    if (!t) return;
    WaitForSingleObject(t->handle, INFINITE);
}

void thread_detach(Thread* t)
{
    if (!t) return;
    CloseHandle(t->handle);
}

void thread_free(Thread* t)
{
    if (!t) return;
    CloseHandle(t->handle);
    free(t);
}

void thread_sleep(long ms)
{
    if (ms <= 0) return;
    Sleep((DWORD)ms);
}

Mutex* mutex_create(void) 
{
    Mutex* m = (Mutex*)malloc(sizeof(Mutex));
    if (!m) return NULL;
    InitializeCriticalSection(&m->cs);
    return m;
}

void mutex_lock(Mutex* m)
{
    if (!m) return;
    EnterCriticalSection(&m->cs);
}

void mutex_unlock(Mutex* m) 
{
    if (!m) return;
    LeaveCriticalSection(&m->cs);
}

void mutex_free(Mutex* m) 
{
    if (!m) return;
    DeleteCriticalSection(&m->cs);
    free(m);
}

int mkdir(const char* path)
{
    if (_mkdir(path) == 0) return 0;
    if (errno == EEXIST) return 0;
    return -1;
}