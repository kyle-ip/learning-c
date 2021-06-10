#include <assert.h>
#include "event_loop.h"
#include "utils.h"

/**
 * in the i/o thread
 * 遍历当前 event loop 里 pending 的 channel event 列表，将其和 event_dispatcher 关联，从而修改感兴趣的事件集合
 *
 * @param eventLoop
 * @return
 */
int event_loop_handle_pending_channel(struct event_loop *eventLoop) {
    // get the lock
    pthread_mutex_lock(&eventLoop->mutex);
    eventLoop->is_handle_pending = 1;

    struct channel_element *channelElement = eventLoop->pending_head;
    while (channelElement != NULL) {
        //save into event_map
        struct channel *channel = channelElement->channel;
        int fd = channel->fd;
        if (channelElement->type == 1) {
            event_loop_handle_pending_add(eventLoop, fd, channel);
        } else if (channelElement->type == 2) {
            event_loop_handle_pending_remove(eventLoop, fd, channel);
        } else if (channelElement->type == 3) {
            event_loop_handle_pending_update(eventLoop, fd, channel);
        }
        channelElement = channelElement->next;
    }

    eventLoop->pending_head = eventLoop->pending_tail = NULL;
    eventLoop->is_handle_pending = 0;

    //release the lock
    pthread_mutex_unlock(&eventLoop->mutex);

    return 0;
}

/**
 *
 * @param eventLoop
 * @param fd
 * @param channel1
 * @param type
 */
void event_loop_channel_buffer_nolock(struct event_loop *eventLoop, int fd, struct channel *channel1, int type) {
    //add channel into the pending list
    struct channel_element *channelElement = malloc(sizeof(struct channel_element));
    channelElement->channel = channel1;
    channelElement->type = type;
    channelElement->next = NULL;
    //第一个元素
    if (eventLoop->pending_head == NULL) {
        eventLoop->pending_head = eventLoop->pending_tail = channelElement;
    } else {
        eventLoop->pending_tail->next = channelElement;
        eventLoop->pending_tail = channelElement;
    }
}

/**
 *
 * @param eventLoop
 * @param fd
 * @param channel1
 * @param type
 * @return
 */
int event_loop_do_channel_event(struct event_loop *eventLoop, int fd, struct channel *channel1, int type) {

    // event loop 线程得到活动事件之后，会回调事件处理函数（onMessage 等）
    // 如果这里的业务逻辑过于复杂，就会导致 event_loop_handle_pending_channel 执行的时间偏后，从而影响 I/O 的检测

    // get the lock
    pthread_mutex_lock(&eventLoop->mutex);
    assert(eventLoop->is_handle_pending == 0);
    // 获取锁后主线程调用往子线程的数据中增加需要处理的 channel event 对象
    event_loop_channel_buffer_nolock(eventLoop, fd, channel1, type);
    // release the lock
    pthread_mutex_unlock(&eventLoop->mutex);

    // 如果是主线程发起操作，则调用 event_loop_wakeup 唤醒子线程
    if (!isInSameThread(eventLoop)) {
        event_loop_wakeup(eventLoop);
    }
    // 如果是子线程自己，则直接可以操作
    else {
        event_loop_handle_pending_channel(eventLoop);
    }
    return 0;
}

/**
 * 新增 channel event 事件
 *
 * @param eventLoop
 * @param fd
 * @param channel1
 * @return
 */
int event_loop_add_channel_event(struct event_loop *eventLoop, int fd, struct channel *channel1) {
    return event_loop_do_channel_event(eventLoop, fd, channel1, 1);
}

/**
 * 删除 channel event 事件
 *
 * @param eventLoop
 * @param fd
 * @param channel1
 * @return
 */
int event_loop_remove_channel_event(struct event_loop *eventLoop, int fd, struct channel *channel1) {
    return event_loop_do_channel_event(eventLoop, fd, channel1, 2);
}

/**
 * 修改 channel event 事件
 *
 * @param eventLoop
 * @param fd
 * @param channel1
 * @return
 */
int event_loop_update_channel_event(struct event_loop *eventLoop, int fd, struct channel *channel1) {
    return event_loop_do_channel_event(eventLoop, fd, channel1, 3);
}

/**
 * 新增 channel event 事件
 *
 * in the i/o thread
 *
 * @param eventLoop
 * @param fd
 * @param channel
 * @return
 */
int event_loop_handle_pending_add(struct event_loop *eventLoop, int fd, struct channel *channel) {
    yolanda_msgx("add channel fd == %d, %s", fd, eventLoop->thread_name);
    struct channel_map *map = eventLoop->channelMap;
    if (fd < 0) {
        return 0;
    }
    if (fd >= map->nentries) {
        if (map_make_space(map, fd, sizeof(struct channel *)) == -1)
            return (-1);
    }
    if ((map)->entries[fd] == NULL) {
        map->entries[fd] = channel;
        struct event_dispatcher *eventDispatcher = eventLoop->eventDispatcher;
        eventDispatcher->add(eventLoop, channel);
        return 1;
    }
    return 0;
}

/**
 * 删除 channel event 事件
 *
 * in the i/o thread
 *
 * @param eventLoop
 * @param fd
 * @param channel1
 * @return
 */
int event_loop_handle_pending_remove(struct event_loop *eventLoop, int fd, struct channel *channel1) {
    struct channel_map *map = eventLoop->channelMap;
    assert(fd == channel1->fd);

    if (fd < 0)
        return 0;

    if (fd >= map->nentries)
        return (-1);

    struct channel *channel2 = map->entries[fd];

    //update dispatcher(multi-thread)here
    struct event_dispatcher *eventDispatcher = eventLoop->eventDispatcher;

    int retval = 0;
    if (eventDispatcher->del(eventLoop, channel2) == -1) {
        retval = -1;
    } else {
        retval = 1;
    }

    map->entries[fd] = NULL;
    return retval;
}

/**
 * 修改 channel event 事件
 *
 * in the i/o thread
 *
 * @param eventLoop
 * @param fd
 * @param channel
 * @return
 */
int event_loop_handle_pending_update(struct event_loop *eventLoop, int fd, struct channel *channel) {
    yolanda_msgx("update channel fd == %d, %s", fd, eventLoop->thread_name);
    struct channel_map *map = eventLoop->channelMap;

    if (fd < 0)
        return 0;

    if ((map)->entries[fd] == NULL) {
        return (-1);
    }

    //update channel
    struct event_dispatcher *eventDispatcher = eventLoop->eventDispatcher;
    eventDispatcher->update(eventLoop, channel);
}

/**
 * 找到了对应的 channel 对象后根据事件类型，回调读函数或者写函数
 *
 * @param eventLoop
 * @param fd
 * @param revents
 * @return
 */
int channel_event_activate(struct event_loop *eventLoop, int fd, int revents) {
    struct channel_map *map = eventLoop->channelMap;
    yolanda_msgx("activate channel fd == %d, revents=%d, %s", fd, revents, eventLoop->thread_name);

    // fd 范围越界
    if (fd < 0) {
        return 0;
    }
    if (fd >= map->nentries) {
        return (-1);
    }

    // 从 map 中获取 channel 对象，并根据事件类型调用回调函数
    struct channel *channel = map->entries[fd];
    assert(fd == channel->fd);

    if (revents & (EVENT_READ)) {
        if (channel->eventReadCallback) {
            channel->eventReadCallback(channel->data);
        }
    }
    if (revents & (EVENT_WRITE)) {
        if (channel->eventWriteCallback) {
            channel->eventWriteCallback(channel->data);
        }
    }

    return 0;

}

/**
 *
 *
 * @param eventLoop
 */
void event_loop_wakeup(struct event_loop *eventLoop) {
    // socketPair[0] 上写一个字节即可把 event_loop 子线程唤醒（因为 event_loop 已经注册了 socketPair[1]的可读事件）
    char one = 'a';
    ssize_t n = write(eventLoop->socketPair[0], &one, sizeof one);
    if (n != sizeof one) {
        LOG_ERR("wakeup event loop thread failed");
    }
}

/**
 * 让子线程从 dispatch 的阻塞中苏醒
 *
 * @param data
 * @return
 */
int handleWakeup(void *data) {
    struct event_loop *eventLoop = (struct event_loop *) data;
    char one;
    ssize_t n = read(eventLoop->socketPair[1], &one, sizeof one);
    if (n != sizeof one) {
        LOG_ERR("handleWakeup  failed");
    }
    yolanda_msgx("wakeup, %s", eventLoop->thread_name);
}

struct event_loop *event_loop_init() {
    return event_loop_init_with_name(NULL);
}

/**
 * 由于 sub-reactor 线程是一个无限循环的 event loop 执行体，在没有已注册事件发生的情况下，会阻塞在 event_dispatcher 的 dispatch 上；
 *
 * 1. 构建一个类似管道的描述字，让 event_dispatcher 注册该管道描述字；
 * 2. 往管道发送消息，使 sub-reactor 线程从 event_dispatcher 的 dispatch 上返回；
 * 3. sub-reactor 线程返回后把新的已连接套接字事件注册上，主线程把已连接套接字交给 sub-reactor 子线程
 *
 * @param thread_name
 * @return
 */
struct event_loop *event_loop_init_with_name(char *thread_name) {
    struct event_loop *eventLoop = malloc(sizeof(struct event_loop));
    pthread_mutex_init(&eventLoop->mutex, NULL);
    pthread_cond_init(&eventLoop->cond, NULL);

    if (thread_name != NULL) {
        eventLoop->thread_name = thread_name;
    } else {
        eventLoop->thread_name = "main thread";
    }

    eventLoop->quit = 0;
    eventLoop->channelMap = malloc(sizeof(struct channel_map));
    map_init(eventLoop->channelMap);

    // 动态选择使用 epoll 或 poll
#ifdef EPOLL_ENABLE
    yolanda_msgx("set epoll as dispatcher, %s", eventLoop->thread_name);
    eventLoop->eventDispatcher = &epoll_dispatcher;
#else
    yolanda_msgx("set poll as dispatcher, %s", eventLoop->thread_name);
    eventLoop->eventDispatcher = &poll_dispatcher;
#endif
    eventLoop->event_dispatcher_data = eventLoop->eventDispatcher->init(eventLoop);

    // add the socketfd to event 创建的是套接字对，目的是唤醒子线程
    eventLoop->owner_thread_id = pthread_self();
    if (socketpair(AF_UNIX, SOCK_STREAM, 0, eventLoop->socketPair) < 0) {
        LOG_ERR("socketpair set fialed");
    }
    eventLoop->is_handle_pending = 0;
    eventLoop->pending_head = NULL;
    eventLoop->pending_tail = NULL;

    // 告诉 event_loop 注册了 socketPair[1] 描述字上的 READ 事件；如果有 READ 事件发生，就调用 handleWakeup 函数来完成事件处理。
    struct channel *channel = channel_new(eventLoop->socketPair[1], EVENT_READ, handleWakeup, NULL, eventLoop);
    event_loop_add_channel_event(eventLoop, eventLoop->socketPair[1], channel);

    return eventLoop;
}

/**
 * event_loop 启动入口
 *
 * event_loop 在不退出的情况下，一直在循环，调用 dispatcher 对象的 dispatch 方法来等待事件的发生
 *
 * 1. 参数验证
 * 2. 调用 dispatcher 来进行事件分发，分发完回调事件处理函数
 */
int event_loop_run(struct event_loop *eventLoop) {

    // 线程 id 不能为空，event_loop 对象不能为空
    assert(eventLoop != NULL);
    struct event_dispatcher *dispatcher = eventLoop->eventDispatcher;
    if (eventLoop->owner_thread_id != pthread_self()) {
        exit(1);
    }

    yolanda_msgx("event loop run, %s", eventLoop->thread_name);

    struct timeval timeval;
    timeval.tv_sec = 1;

    while (!eventLoop->quit) {
        // block here to wait I/O event, and get active channels
        // 阻塞等待事件发生：创建连接对象 tcp_connection、channel 对象等
        dispatcher->dispatch(eventLoop, &timeval);

        // handle the pending channel
        // 处理等待的 channel
        event_loop_handle_pending_channel(eventLoop);
    }

    yolanda_msgx("event loop end, %s", eventLoop->thread_name);
    return 0;
}


