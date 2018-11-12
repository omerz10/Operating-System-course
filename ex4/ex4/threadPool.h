#ifndef __THREAD_POOL__
#define __THREAD_POOL__

#include "osqueue.h"
#include <zconf.h>
#include <malloc.h>
/*
 * Omer Zucker
 * 200876548
 * Operating Systems- ex4
 */


typedef struct thread_pool
{
    pthread_t* threads;
    pthread_mutex_t lock;
    pthread_mutex_t sd_lock;
    pthread_mutex_t c_lock;
    pthread_cond_t pthread_cond;
    struct os_queue* queue;
    int threads_count;
    int queue_counter;
    int shutdown;


} ThreadPool;


typedef  struct task_s
{
    void (*func)(void*);
    void* param;
} Task_t;


ThreadPool* tpCreate(int numOfThreads);

void tpDestroy(ThreadPool* threadPool, int shouldWaitForTasks);

int tpInsertTask(ThreadPool* threadPool, void (*computeFunc) (void *), void* param);

void writeToTxt();

void freeThreadPool(ThreadPool *pool);

void* threadEntry(ThreadPool* pool);

void joinThreads(ThreadPool* threadPool);



#endif
