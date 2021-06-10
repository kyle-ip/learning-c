#include <lib/acceptor.h>
#include "lib/common.h"
#include "lib/event_loop.h"
#include "lib/tcp_server.h"

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


// 主线程的 epoll_wait 只处理 acceptor 套接字的事件，表示连接的建立；反应堆子线程的 epoll_wait 主要处理的是已连接套接字的读写事件。
int main(int c, char **v) {
    // 主线程 event_loop
    struct event_loop *eventLoop = event_loop_init();

    // 初始化 acceptor
    struct acceptor *acceptor = acceptor_init(SERV_PORT);

    // 初始 tcp_server，可以指定线程数目，这里线程是 4，说明是一个 acceptor 线程，4 个 I/O 线程，没一个 I/O 线程
    // tcp_server 自己带一个event_loop
    struct TCPserver *tcpServer = tcp_server_init(eventLoop, acceptor, onConnectionCompleted, onMessage, onWriteCompleted, onConnectionClosed, 4);
    tcp_server_start(tcpServer);

    // main thread for acceptor
    event_loop_run(eventLoop);
}