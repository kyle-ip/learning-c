#include <assert.h>
#include "event_loop_thread.h"
#include "event_loop.h"

/**
 * （由子线程调用）启动 event_loop，并完成线程初始化工作
 *
 * @param arg
 * @return
 */
void *event_loop_thread_run(void *arg) {
    struct event_loop_thread *eventLoopThread = (struct event_loop_thread *) arg;

    pthread_mutex_lock(&eventLoopThread->mutex);

    // 初始化 eventLoop（初始化前为 NULL）
    eventLoopThread->eventLoop = event_loop_init_with_name(eventLoopThread->thread_name);
    yolanda_msgx("event loop thread init and signal, %s", eventLoopThread->thread_name);

    // 初始化成功后才通知主线程从 wait 中苏醒，继续往下执行
    pthread_cond_signal(&eventLoopThread->cond);
    pthread_mutex_unlock(&eventLoopThread->mutex);

    // 子线程 event loop run
    event_loop_run(eventLoopThread->eventLoop);

    return NULL;
}

/**
 * 初始化已经分配内存的 event_loop_thread
 *
 * @param eventLoopThread
 * @param i
 * @return
 */
int event_loop_thread_init(struct event_loop_thread *eventLoopThread, int i) {
    pthread_mutex_init(&eventLoopThread->mutex, NULL);
    pthread_cond_init(&eventLoopThread->cond, NULL);
    eventLoopThread->eventLoop = NULL;
    eventLoopThread->thread_count = 0;
    eventLoopThread->thread_tid = 0;

    char *buf = malloc(16);
    sprintf(buf, "Thread-%d\0", i + 1);
    eventLoopThread->thread_name = buf;

    return 0;
}


/**
 * （由主线程调用）初始化一个子线程，并且让子线程开始运行 event_loop
 *
 * @param eventLoopThread
 * @return
 */
struct event_loop *event_loop_thread_start(struct event_loop_thread *eventLoopThread) {

    // 子线程创建，立即执行 event_loop_thread_run
    pthread_create(&eventLoopThread->thread_tid, NULL, &event_loop_thread_run, eventLoopThread);

    // eventLoopThread->eventLoop 对象由主线程和子线程共享，因此需要加锁
    assert(pthread_mutex_lock(&eventLoopThread->mutex) == 0);

    while (eventLoopThread->eventLoop == NULL) {
        // 守候 eventLoopThread 中的 eventLoop 的变量
        assert(pthread_cond_wait(&eventLoopThread->cond, &eventLoopThread->mutex) == 0);
    }

    // 解锁
    assert(pthread_mutex_unlock(&eventLoopThread->mutex) == 0);

    yolanda_msgx("event loop thread started, %s", eventLoopThread->thread_name);
    return eventLoopThread->eventLoop;
}