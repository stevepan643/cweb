#include "utils/thread_pool/tread_pool.h"
#include "utils/platform/platform.h"

#include <stdlib.h>
#include <stdio.h>

struct Task {
    TaskFunc func;
    void* arg;
    int priority;
    struct Task* next;
};

struct Future {
    void* result;
    int done;
    Mutex* lock;
    Cond* cond;
};

struct ThreadPool {
    Thread** threads;
    int thread_count;

    Task* head;
    Task* tail;

    Mutex* lock;
    Cond* cond;

    int stop;
    int paused;

    int working_count;
};

static void worker_main(void* arg) {
    ThreadPool* pool = arg;

    for (;;) {
        mutex_lock(pool->lock);

        while (!pool->head && !pool->stop || pool->paused) {
            cond_wait(pool->cond, pool->lock);
        }

        if (pool->stop) {
            mutex_unlock(pool->lock);
            break;
        }

        // 取任务（优先级队列可拓展，这里简单FIFO）
        Task* task = pool->head;
        pool->head = task->next;
        if (!pool->head) pool->tail = NULL;

        pool->working_count++;
        mutex_unlock(pool->lock);

        // 执行任务
        task->func(task->arg);
        free(task);

        mutex_lock(pool->lock);
        pool->working_count--;
        mutex_unlock(pool->lock);
    }
}

static void* future_task_wrapper(void* arg) {
    struct { Future* f; TaskFunc func; void* t_arg; }* wrapper = arg;
    void* res = wrapper->func(wrapper->t_arg); // 注意 func 返回 void*
    
    mutex_lock(wrapper->f->lock);
    wrapper->f->result = res;
    wrapper->f->done = 1;
    cond_signal(wrapper->f->cond);
    mutex_unlock(wrapper->f->lock);

    free(wrapper);
    return NULL; // 因为 thread_pool_submit 返回类型是 void*，必须 return
}

// -------------------- ThreadPool API --------------------
ThreadPool* thread_pool_create(int n) {
    ThreadPool* pool = calloc(1, sizeof(ThreadPool));
    pool->threads = calloc(n, sizeof(Thread*));
    pool->thread_count = n;
    pool->lock = mutex_create();
    pool->cond = cond_create();
    pool->stop = 0;
    pool->paused = 0;
    pool->working_count = 0;

    for (int i = 0; i < n; i++) {
        pool->threads[i] = thread_create(worker_main, pool);
    }

    return pool;
}

void thread_pool_submit(ThreadPool* pool, TaskFunc func, void* arg) {
    Task* task = malloc(sizeof(Task));
    task->func = func;
    task->arg = arg;
    task->priority = 0;
    task->next = NULL;

    mutex_lock(pool->lock);
    if (pool->tail)
        pool->tail->next = task;
    else
        pool->head = task;
    pool->tail = task;
    cond_signal(pool->cond);
    mutex_unlock(pool->lock);
}

void thread_pool_submit_ex(ThreadPool* pool, TaskFunc func, void* arg, int priority) {
    // 这里简单支持优先级，直接放在队列头（更复杂可以做堆）
    Task* task = malloc(sizeof(Task));
    task->func = func;
    task->arg = arg;
    task->priority = priority;
    task->next = NULL;

    mutex_lock(pool->lock);
    if (priority > 0) {
        task->next = pool->head;
        pool->head = task;
        if (!pool->tail) pool->tail = task;
    } else {
        if (pool->tail)
            pool->tail->next = task;
        else
            pool->head = task;
        pool->tail = task;
    }
    cond_signal(pool->cond);
    mutex_unlock(pool->lock);
}

void thread_pool_destroy(ThreadPool* pool) {
    mutex_lock(pool->lock);
    pool->stop = 1;
    cond_broadcast(pool->cond);
    mutex_unlock(pool->lock);

    for (int i = 0; i < pool->thread_count; i++) {
        thread_join(pool->threads[i]);
        thread_free(pool->threads[i]);
    }

    mutex_free(pool->lock);
    cond_free(pool->cond);

    free(pool->threads);
    free(pool);
}

void thread_pool_pause(ThreadPool* pool) {
    mutex_lock(pool->lock);
    pool->paused = 1;
    mutex_unlock(pool->lock);
}

void thread_pool_resume(ThreadPool* pool) {
    mutex_lock(pool->lock);
    pool->paused = 0;
    cond_broadcast(pool->cond);
    mutex_unlock(pool->lock);
}

// -------------------- Future API --------------------
Future* thread_pool_submit_future(ThreadPool* pool, TaskFunc func, void* arg) {
    Future* f = malloc(sizeof(Future));
    f->result = NULL;
    f->done = 0;
    f->lock = mutex_create();
    f->cond = cond_create();

    // 包装任务
    struct { Future* f; TaskFunc func; void* t_arg; } *wrapper = malloc(sizeof(*wrapper));
    wrapper->f = f;
    wrapper->func = func;
    wrapper->t_arg = arg;

    thread_pool_submit(pool, future_task_wrapper, wrapper);
    return f;
}

void* future_get(Future* f) {
    mutex_lock(f->lock);
    while (!f->done)
        cond_wait(f->cond, f->lock);
    mutex_unlock(f->lock);
    void* res = f->result;
    // 可以选择 free Future 由用户控制
    return res;
}

// -------------------- 状态查询 --------------------
int thread_get_working_count(ThreadPool* pool) {
    int c;
    mutex_lock(pool->lock);
    c = pool->working_count;
    mutex_unlock(pool->lock);
    return c;
}

int thread_get_free_count(ThreadPool* pool) {
    int c;
    mutex_lock(pool->lock);
    c = pool->thread_count - pool->working_count;
    mutex_unlock(pool->lock);
    return c;
}

int thread_get_total_count(ThreadPool* pool) {
    return pool->thread_count;
}