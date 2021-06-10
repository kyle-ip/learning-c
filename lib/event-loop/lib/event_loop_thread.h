#ifndef EVENT_LOOP_THREAD_H
#define EVENT_LOOP_THREAD_H

#include <pthread.h>

/**
 * reactor 的线程实现，连接套接字的 read/write 事件检测都在这个线程里完成
 */
struct event_loop_thread {
    struct event_loop *eventLoop;
    pthread_t thread_tid;        /* thread ID */
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    char * thread_name;
    long thread_count;    /* # connections handled */
};

// 初始化已经分配内存的 event_loop_thread
int event_loop_thread_init(struct event_loop_thread *, int);

// 由主线程调用，初始化一个子线程，并且让子线程开始运行 event_loop
struct event_loop *event_loop_thread_start(struct event_loop_thread *);

#endif