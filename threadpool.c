#include <stdlib.h>
#include "uthread.h"
#include "uthread_mutex_cond.h"
#include "threadpool.h"

typedef struct task_t {
  void (*func)(void*);
  void* arg;
  struct task_t* next;
} task_t;

struct tpool {
    // TODO done
    unsigned int num_threads;
    unsigned int num_tasks;
    unsigned int max_threads;
    unsigned int num_active_threads;
    uthread_t *threads;
    task_t *tasks;
    uthread_mutex_t mutex;
    uthread_cond_t notify;
    uthread_cond_t notify_idle;
    volatile int sleep;
    // volatile int is_alive;
};



/**
 * Base procedure of every worker thread.  Calls available tasks
 * or blocks until a task becomes available.
 */
void *worker_thread(void *pool_v) {
  tpool_t pool = pool_v;

  // TODO done

      while (1) {
        uthread_mutex_lock(&pool->mutex);
        while (pool->num_tasks == 0 && !pool->sleep) {
            uthread_cond_wait(&pool->notify);
        }
        if (pool->sleep) {
            uthread_mutex_unlock(&pool->mutex);
            return NULL;
        }
        task_t *task = pool->tasks;
        if (task != NULL) {
            pool->tasks = task->next;
            pool->num_tasks--;
            // uthread_mutex_unlock(&pool->queue_mutex);
            // task->func(pool, task->arg);
            // free(task);
        } 
        uthread_mutex_unlock(&pool->mutex);

        if(task != NULL) {
            task->func(task->arg);
            free(task);
        }
        
    }
    return NULL;
}

/**
 * Create a new thread pool with max_threads thread-count limit.
 */
tpool_t tpool_create(unsigned int max_threads) {
    // TODO 

    // if(pool == NULL) {
    //     return NULL;
    // }
    tpool_t pool = (tpool_t)malloc(sizeof(struct tpool));
    // pool->threads = (uthread_t*)malloc(max_threads * sizeof(uthread_t));
    if(pool->threads == NULL) {
        free(pool);
        return NULL;
    }

    pool->max_threads = max_threads;
    pool->num_active_threads = 0;
    pool->num_threads = 0;
    pool->tasks = NULL;
    pool->num_tasks = 0;
    pool->sleep = 0;


    for (unsigned int i = 0; i < max_threads; i++) {
        if(uthread_create(worker_thread, pool) == NULL) {
            pool->num_threads++;
        }
        return NULL;
    }

    return pool;
}

/**
 * Sechedule task f(arg) to be executed.
 */
void tpool_schedule_task(tpool_t pool, void (*f)(tpool_t, void *), void *arg) {
    // TODO

    task_t *task = (task_t*)malloc(sizeof(task_t));
    task->func = f;
    task->arg = arg;
    task->next = NULL;

    uthread_mutex_lock(&pool->mutex);

    if (pool->tasks == NULL) {
        pool->tasks = task;
        uthread_cond_signal(&pool->notify);
    } else {
        task_t *last = pool->tasks;
        while (last->next != NULL) {
            last = last->next;
        }
        last->next = task;
    }
    pool->num_tasks++;

    // Notify a waiting worker thread
    uthread_cond_signal(&pool->notify);

    uthread_mutex_unlock(&pool->mutex);
}

/**
 * Wait (by blocking) until all tasks have completed and thread pool is thus idle
 */
void tpool_join(tpool_t pool) {
    // TODO

    uthread_mutex_lock(&pool->mutex);
    while (pool->num_tasks > 0 || pool->num_active_threads > 0) {
        uthread_cond_wait(&pool->notify_idle);
    }
    uthread_mutex_unlock(&pool->mutex);
    
}
