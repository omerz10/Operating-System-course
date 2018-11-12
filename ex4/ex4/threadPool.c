/*
 * Omer Zucker
 * 200876548
 * Operating Systems- ex4
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <fcntl.h>
#include <memory.h>
#include "threadPool.h"

#define MODE (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)

// Error message while system call failed
char *error = "Error in system call";


ThreadPool *tpCreate(int numOfThreads) {
    int shouldFree = 0;

    // numOfThreads should be greater than 0
    if (numOfThreads < 0) {
        printf("threads number has not difine\n");
        return NULL;
    }
    // allocate thread pool
    ThreadPool *tPool;
    if ((tPool = (ThreadPool *) malloc(sizeof(ThreadPool))) == NULL) {
        printf("allocation error\n");
        shouldFree = 1;
    }
    // init thread pool variables
    tPool->threads_count = numOfThreads;
    tPool->shutdown = 0;
    tPool->queue_counter = 0;

    // allocate threads
    if ((tPool->threads = (pthread_t *) malloc(numOfThreads * sizeof(pthread_t))) == NULL) {
        printf("allocation error\n");
        shouldFree = 1;
    }
    // create queue
    if ((tPool->queue = osCreateQueue()) == NULL) {
        printf("allocation error\n");
        shouldFree = 1;
    }
    // init mutexes
    if ((pthread_mutex_init(&(tPool->lock), NULL) != 0) || (pthread_mutex_init(&(tPool->sd_lock), NULL) != 0) ||
        (pthread_mutex_init(&(tPool->c_lock), NULL) != 0)) {
        printf("allocation error\n");
        shouldFree = 1;
    }
    // init condition thread
    if ((pthread_cond_init(&(tPool->pthread_cond), NULL) != 0)) {
        printf("allocation error\n");
        shouldFree = 1;
    }
    // create all threads
    for (int i = 0; i < numOfThreads; i++) {
        int thread = pthread_create(&(tPool->threads[i]), NULL, (void*)threadEntry, tPool);
        if (thread != 0) {
            printf("allocation error\n");
            shouldFree = 1;
            tpDestroy(tPool, 0);
        }
    }
    if (shouldFree) {
        freeThreadPool(tPool);
        exit(-1);
    }
    // allocation failed

    return tPool;
}

void *threadEntry(ThreadPool *pool) {
    Task_t *task;
    int active = 1;

    // finish when pool is shutdown
    while (active) {
        // lock status of the queue- if its empty and/or shutdown
        pthread_mutex_lock(&pool->lock);
        // run as long as thread pool is active and queue is empty
        if (osIsQueueEmpty(pool->queue) && !pool->shutdown) {
            // wait until new task will pushed to the queue
            if (pthread_cond_wait(&pool->pthread_cond, &pool->lock) != 0) { write(STDERR_FILENO, error, strlen(error));
            } else {
                // queue is empty and pool is active - unlock
                if (pthread_mutex_unlock(&pool->lock) != 0) { write(STDERR_FILENO, error, strlen(error)); }
            }
        }
            // queue is not empty and/or thread pool is shutdown - unlock
        else {
            pthread_mutex_unlock(&pool->lock);
        }
        // lock for dequeue from queue
        pthread_mutex_lock(&pool->lock);
        // there are tasks in queue
        if(!osIsQueueEmpty(pool->queue)) {
            // pull task from queue
            task = osDequeue(pool->queue);
            // for counter
            pthread_mutex_unlock(&(pool->c_lock));
            pool->queue_counter--;
            // for counter
            pthread_mutex_unlock(&(pool->c_lock));
            // unlock the queque
            pthread_mutex_unlock(&pool->lock);
            // execute task
            task->func(task->param);
            free(task);
        }
        else {
            pthread_mutex_unlock(&pool->lock);
        }
        // stops loop
        if (pool->shutdown && pool->queue_counter <= 0) { active = 0; }
    }
    pthread_exit(NULL);
    return NULL;
}

void tpDestroy(ThreadPool *threadPool, int shouldWaitForTasks) {
    int i;
    // lock shutdown
    pthread_mutex_lock(&threadPool->sd_lock);
    //if shutdown already on, exit
    if (threadPool->shutdown == 1) {
        // unlock shutdown
        pthread_mutex_unlock(&threadPool->sd_lock);
        return;
    }
    // just one thread can operate this function
    threadPool->shutdown = 1;
    // unlock if shutdown was 0
    pthread_mutex_unlock(&threadPool->sd_lock);

    /* if variable is 0: turn off only active tasks in threads
       if variable is greater than 0: turn off all tasks
    */
    if (!shouldWaitForTasks) {
        // lock queue counter
        pthread_mutex_lock(&threadPool->c_lock);
        threadPool->queue_counter = 0;
        pthread_mutex_unlock(&threadPool->c_lock);
    }
    do {
        // send signal to all threads
        pthread_cond_broadcast(&threadPool->pthread_cond);
        // join all threads
        for (i = 0; i < threadPool->threads_count; i++) {
            pthread_join(threadPool->threads[i], NULL);
        }
        // free thread pool
    } while (0);
    // free thread pool
    freeThreadPool(threadPool);
}

int tpInsertTask(ThreadPool *threadPool, void (*computeFunc)(void *), void *param) {
    Task_t *task;
    int result = 0;

    if (threadPool->shutdown) { return -1; }
    if (computeFunc == NULL) { return -1; }
    // allocate task
    if ((task = (Task_t *) malloc(sizeof(Task_t))) == NULL) {
        printf("allocation error\n");
        return -1;
    }
    // lock enqueue
    if (pthread_mutex_lock(&(threadPool->lock)) != 0) { return -1; }
    // implement args
    task->func = computeFunc;
    task->param = param;
    // push task to queue
    osEnqueue(threadPool->queue, (void *) task);
    // lock adding to queue counter
    if (pthread_mutex_lock(&(threadPool->c_lock)) != 0) { return -1; }
    // add 1 to queue counter
    threadPool->queue_counter++;
    // unlock adding to queue counter
    if (pthread_mutex_unlock(&(threadPool->c_lock)) != 0) { return -1; }
    // lock enqueue
    if (pthread_mutex_unlock(&(threadPool->lock)) != 0) { return -1; }
    // send signal to thread updating task was pushed into queue
    if (pthread_cond_signal(&(threadPool->pthread_cond)) != 0) {return -1; }

    return result;
}

void freeThreadPool(ThreadPool *pool) {

    // run through tasks in queue
    while (!osIsQueueEmpty(pool->queue)) {
        Task_t *task = osDequeue(pool->queue);
        free(task);
    }
    // free queue
    if (pool->queue != NULL) { osDestroyQueue(pool->queue); }
    // free threads
    if (pool->threads != NULL) { free(pool->threads); }
    // destroy threads mutex
    pthread_mutex_destroy(&(pool->lock));
    // destroy shutdown mutex
    pthread_mutex_destroy(&(pool->sd_lock));
    // destroy queue counter mutex
    pthread_mutex_destroy(&(pool->c_lock));
    // free condition
    pthread_cond_destroy(&(pool->pthread_cond));

    free(pool);
}

