#ifndef ACCEPTOR_H
#define ACCEPTOR_H

#include "common.h"

/**
 * 服务器端监听器，作为一个 channel 对象注册到 event_loop 上，以进行连接完成的事件分发和检测
 */
struct acceptor {
    /* 监听端口 */
    int listen_port;
    /* 描述字 */
    int listen_fd;
};

struct acceptor *acceptor_init(int port);

#endif