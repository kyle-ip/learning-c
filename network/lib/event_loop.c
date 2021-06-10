#include <assert.h>
#include "event_loop.h"
#include "common.h"
#include "log.h"
#include "event_dispatcher.h"
#include "channel.h"
#include "utils.h"

// in the i/o thread
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

int event_loop_do_channel_event(struct event_loop *eventLoop, int fd, struct channel *channel1, int type) {
    //get the lock
    pthread_mutex_lock(&eventLoop->mutex);
    assert(eventLoop->is_handle_pending == 0);
    event_loop_channel_buffer_nolock(eventLoop, fd, channel1, type);
    //release the lock
    pthread_mutex_unlock(&eventLoop->mutex);
    if (!isInSameThread(eventLoop)) {
        event_loop_wakeup(eventLoop);
    } else {
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

void event_loop_wakeup(struct event_loop *eventLoop) {
    char one = 'a';
    ssize_t n = write(eventLoop->socketPair[0], &one, sizeof one);
    if (n != sizeof one) {
        LOG_ERR("wakeup event loop thread failed");
    }
}

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

    //add the socketfd to event
    eventLoop->owner_thread_id = pthread_self();
    if (socketpair(AF_UNIX, SOCK_STREAM, 0, eventLoop->socketPair) < 0) {
        LOG_ERR("socketpair set fialed");
    }
    eventLoop->is_handle_pending = 0;
    eventLoop->pending_head = NULL;
    eventLoop->pending_tail = NULL;

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


