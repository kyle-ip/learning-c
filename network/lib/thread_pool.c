#include <assert.h>
#include "utils.h"
#include "thread_pool.h"

struct thread_pool *thread_pool_new(struct event_loop *mainLoop, int threadNumber) {

    struct thread_pool *threadPool = malloc(sizeof(struct thread_pool));
    threadPool->mainLoop = mainLoop;
    threadPool->position = 0;
    threadPool->thread_number = threadNumber;
    threadPool->started = 0;
    threadPool->eventLoopThreads = NULL;
    return threadPool;
}

/**
 * （由主线程）启动线程池
 *
 * @param threadPool
 */
void thread_pool_start(struct thread_pool *threadPool) {
    assert(!threadPool->started);
    assertInSameThread(threadPool->mainLoop);

    threadPool->started = 1;
    void *tmp;

    // 如果线程池大小为 0 则直接返回，acceptor 和 I/O 事件都会在同一个主线程里处理，退化为单 reactor 模式
    if (threadPool->thread_number <= 0) {
        return;
    }

    threadPool->eventLoopThreads = malloc(threadPool->thread_number * sizeof(struct event_loop_thread));
    for (int i = 0; i < threadPool->thread_number; ++i) {
        // 对每个子线程初始化并启动
        event_loop_thread_init(&threadPool->eventLoopThreads[i], i);
        event_loop_thread_start(&threadPool->eventLoopThreads[i]);
    }
}

//一定是main thread中选择
struct event_loop *thread_pool_get_loop(struct thread_pool *threadPool) {
    assert(threadPool->started);
    assertInSameThread(threadPool->mainLoop);

    //优先选择当前主线程
    struct event_loop *selected = threadPool->mainLoop;

    //从线程池中按照顺序挑选出一个线程
    if (threadPool->thread_number > 0) {
        selected = threadPool->eventLoopThreads[threadPool->position].eventLoop;
        if (++threadPool->position >= threadPool->thread_number) {
            threadPool->position = 0;
        }
    }

    return selected;
}
