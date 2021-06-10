#include <lib/acceptor.h>
#include "lib/common.h"
#include "lib/event_loop.h"
#include "lib/tcp_server.h"

// Reactor 模型：
//      1. 存在一个无限循环的事件分发线程，或者叫做 reactor 线程、Event loop 线程，事件分发线程的背后，就是 poll、epoll 等 I/O 分发技术的使用。
//      2. 所有的 I/O 操作都可以抽象成事件，每个事件必须有回调函数来处理。acceptor 连接建立成功、套接字上发送缓冲区空出可写、通信管道 pipe 上有数据可以读，通过事件分发，这些事件都可以被检测，并调用对应的回调函数加以处理。
//      3. 工作线程池可以与反应堆线程池分离，用于处理耗时操作（decode、compute、encode），而反应堆线程池只负责 I/O 相关工作（read、send）
//      4. 主从 reactor 模式：主反应堆线程只负责分发 Acceptor 连接建立，已连接套接字上的 I/O 事件交给 sub-reactor 负责分发。其中 sub-reactor 的数量，可以根据 CPU 的核数来灵活设置。
//      5. 主 - 从 reactor+worker threads 模式：主 - 从 reactor 模式解决了 I/O 分发的高效率问题，还可以利用 work threads 解决业务逻辑和 I/O 分发之间的耦合问题：
//              主线程接收客户端请求，分发到从线程处理 I/O 操作，而对于其他耗时操作，则再由从线程分配给工作线程完成。
// https://time.geekbang.org/column/article/146664
// https://time.geekbang.org/column/article/148148

char rot13_char(char c) {
    if ((c >= 'a' && c <= 'm') || (c >= 'A' && c <= 'M'))
        return c + 13;
    else if ((c >= 'n' && c <= 'z') || (c >= 'N' && c <= 'Z'))
        return c - 13;
    else
        return c;
}

// 连接建立之后的 callback
int onConnectionCompleted(struct tcp_connection *tcpConnection) {
    printf("connection completed\n");
    return 0;
}

// 数据读到 buffer 之后的 callback
int onMessage(struct buffer *input, struct tcp_connection *tcpConnection) {
    printf("get message from tcp connection %s\n", tcpConnection->name);
    printf("%s", input->data);

    struct buffer *output = buffer_new();
    int size = buffer_readable_size(input);
    for (int i = 0; i < size; i++) {
        buffer_append_char(output, rot13_char(buffer_read_char(input)));
    }
    tcp_connection_send_buffer(tcpConnection, output);
    return 0;
}

// 数据通过 buffer 写完之后的 callback
int onWriteCompleted(struct tcp_connection *tcpConnection) {
    printf("write completed\n");
    return 0;
}

// 连接关闭之后的 callback
int onConnectionClosed(struct tcp_connection *tcpConnection) {
    printf("connection closed\n");
    return 0;
}

int main(int c, char **v) {
    // 主线程 event_loop
    struct event_loop *eventLoop = event_loop_init();

    // 初始化 acceptor
    struct acceptor *acceptor = acceptor_init(SERV_PORT);

    // 初始 tcp_server，可以指定线程数目，如果线程是 0，则只有一个线程，既负责 acceptor 也负责I/O
    struct TCPserver *tcpServer = tcp_server_init(eventLoop, acceptor, onConnectionCompleted, onMessage, onWriteCompleted, onConnectionClosed, 0);
    tcp_server_start(tcpServer);

    // main thread for acceptor
    event_loop_run(eventLoop);
}