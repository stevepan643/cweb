#ifndef THREAD_POOL_H
#define THREAD_POOL_H

typedef void* (*TaskFunc)(void*);

typedef struct Task Task;
typedef struct ThreadPool ThreadPool;
typedef struct Future Future;

ThreadPool* thread_pool_create(int n);
void thread_pool_submit(ThreadPool* pool, TaskFunc func, void* arg);
void thread_pool_submit_ex(ThreadPool* pool, TaskFunc func, void* arg, int priority);
void thread_pool_destroy(ThreadPool* pool);

void thread_pool_pause(ThreadPool* pool);
void thread_pool_resume(ThreadPool* pool);

Future* thread_pool_submit_future(ThreadPool* pool, TaskFunc func, void* arg);
void* future_get(Future* f);

int thread_get_working_count(ThreadPool* pool);
int thread_get_free_count(ThreadPool* pool);
int thread_get_total_count(ThreadPool* pool);

#endif