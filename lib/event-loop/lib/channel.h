#ifndef CHANNEL_H
#define CHANNEL_H

#include "common.h"
#include "event_loop.h"
#include "buffer.h"

#define EVENT_TIMEOUT 0x01

/* Wait for a socket or FD to become readable */
#define EVENT_READ 0x02

/* Wait for a socket or FD to become writeable */
#define EVENT_WRITE 0x04

/* Wait for a POSIX signal to be raised*/
#define EVENT_SIGNAL 0x08


typedef int (*event_read_callback)(void *data);

typedef int (*event_write_callback)(void *data);

/**
 * 对各种注册到 event_loop 上的对象的抽象（例如注册到 event_loop 上的监听事件，注册到 event_loop 上的套接字读写事件等）；
 * 主要用于和 event_dispatcher 进行交互，一个 channel 对应一个描述字（ READ/WRITE 事件）
 */
struct channel {

    int fd;

    /* event 类型 */
    int events;

    /* 读回调 */
    event_read_callback eventReadCallback;

    /* 写回调 */
    event_write_callback eventWriteCallback;

    /* callback data, 可能是 event_loop，也可能是 tcp_server 或者 tcp_connection */
    void *data;
};


struct channel *
channel_new(int fd, int events, event_read_callback eventReadCallback, event_write_callback eventWriteCallback,
            void *data);

int channel_write_event_is_enabled(struct channel *channel);

int channel_write_event_enable(struct channel *channel);

int channel_write_event_disable(struct channel *channel);


#endif