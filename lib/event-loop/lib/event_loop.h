#ifndef EVENT_LOOP_H
#define EVENT_LOOP_H

#include <pthread.h>
#include "channel.h"
#include "event_dispatcher.h"
#include "common.h"

extern const struct event_dispatcher poll_dispatcher;
extern const struct event_dispatcher epoll_dispatcher;


struct channel_element {
    // 1: add  2: delete
    int type;

    struct channel *channel;

    struct channel_element *next;
};

/**
 * event_loop 即和一个线程绑定的无限事件循环，用于执行事件分发，一旦有事件发生，它就会回调预先定义好的回调函数，完成事件的处理。
 * 使用 epoll 或 poll 将一个线程阻塞，监听等待各种 I/O 事件的发生；
 * event_loop 的运行：
 *      1. 当 event_loop_run 完成之后线程进入循环；
 *      2. 执行 dispatch 事件分发；
 *      3. 一旦有事件发生，调用 channel_event_activate 函数；
 *      4. 在 channel_event_activate 中完成事件回调函数 eventReadcallback 和 eventWritecallback 的调用；
 *      5. 最后再进行 event_loop_handle_pending_channel，修改当前监听的事件列表；
 *      6. 完成以上流程后，又进入了事件分发循环 event_loop_run。
 *
 *      event_loop_run() => dispatcher->dispatch() => channel_event_activate() => eventReadcallback()/eventWritecallback() =>
 *      event_loop_handle_pending_channel() =>
 *          dispatcher->add()
 *          dispatcher->del()
 *          dispatcher->update()
 *      event_loop_run()
 */
struct event_loop {

    int quit;
    /* 事件分发器（poll 或 epoll），可以让线程挂起，等待事件发生 */
    const struct event_dispatcher *eventDispatcher;

    /* 对应的 event_dispatcher 的数据 */
    void *event_dispatcher_data;

    struct channel_map *channelMap;

    int is_handle_pending;

    /* 保留在子线程内的需要处理的新事件 */
    struct channel_element *pending_head;
    struct channel_element *pending_tail;

    /* event_loop 线程 id */
    pthread_t owner_thread_id;

    /* 锁 */
    pthread_mutex_t mutex;

    /* 条件变量 */
    pthread_cond_t cond;

    /* 用于父线程通知子线程有新的事件需要处理 */
    int socketPair[2];

    char *thread_name;
};

struct event_loop *event_loop_init();

struct event_loop *event_loop_init_with_name(char * thread_name);

int event_loop_run(struct event_loop *eventLoop);

void event_loop_wakeup(struct event_loop *eventLoop);

int event_loop_add_channel_event(struct event_loop *eventLoop, int fd, struct channel *channel1);

int event_loop_remove_channel_event(struct event_loop *eventLoop, int fd, struct channel *channel1);

int event_loop_update_channel_event(struct event_loop *eventLoop, int fd, struct channel *channel1);

int event_loop_handle_pending_add(struct event_loop *eventLoop, int fd, struct channel *channel);

int event_loop_handle_pending_remove(struct event_loop *eventLoop, int fd, struct channel *channel);

int event_loop_handle_pending_update(struct event_loop *eventLoop, int fd, struct channel *channel);

// dispather 派发完事件之后，调用该方法通知 event_loop 执行对应事件的相关 callback 方法
// res：EVENT_READ | EVENT_READ 等
int channel_event_activate(struct event_loop *eventLoop, int fd, int res);

#endif