#ifndef EVENT_DISPATCHER_H
#define EVENT_DISPATCHER_H

#include "channel.h"

/**
 * 抽象的 event_dispatcher 结构体，对应的实现如 select, poll, epoll 等 I/O 复用，可挂起线程，等待事件发生
 *      是对事件分发机制的一种抽象（接口），可以实现一个基于 poll 的 poll_dispatcher，也可以实现一个基于 epoll 的 epoll_dispatcher
 */
struct event_dispatcher {
    /*  对应实现 */
    const char *name;

    /*  初始化函数 */
    void *(*init)(struct event_loop *eventLoop);

    /* 通知 dispatcher 新增一个 channel 事件*/
    int (*add)(struct event_loop *eventLoop, struct channel *channel);

    /* 通知 dispatcher 删除一个 channel 事件 */
    int (*del)(struct event_loop *eventLoop, struct channel *channel);

    /* 通知 dispatcher 更新 channel 对应的事件 */
    int (*update)(struct event_loop *eventLoop, struct channel *channel);

    /* 实现事件分发，然后调用 event_loop 的 event_activate 方法执行 callback */
    int (*dispatch)(struct event_loop *eventLoop, struct timeval *);

    /* 清除数据 */
    void (*clear)(struct event_loop *eventLoop);
};

#endif