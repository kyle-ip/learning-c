#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include "event_loop.h"
#include "event_loop_thread.h"

/**
 * 线程池，维护了一个 sub-reactor 的线程列表，可以提供给主 reactor 线程使用；
 * 每次有新的连接建立可以从中获取一个线程，以便用于对新连接套接字的 read/write 事件注册，将 I/O 线程和主 reactor 线程分离。
 */
struct thread_pool {
    // 创建 thread_pool 的主线程
    struct event_loop *mainLoop;
    // 是否已经启动
    int started;
    // 线程数目
    int thread_number;
    // 数组指针，指向创建的 event_loop_thread 数组
    struct event_loop_thread *eventLoopThreads;
    // 表示在数组里的位置，用来决定选择哪个 event_loop_thread 服务
    int position;

};

struct thread_pool *thread_pool_new(struct event_loop *mainLoop, int threadNumber);

void thread_pool_start(struct thread_pool *);

struct event_loop *thread_pool_get_loop(struct thread_pool *);

#endif