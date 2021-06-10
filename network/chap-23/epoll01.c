#include  <sys/epoll.h>
#include "lib/common.h"

#define MAXEVENTS 128

char rot13_char(char c) {
    if ((c >= 'a' && c <= 'm') || (c >= 'A' && c <= 'M'))
        return c + 13;
    else if ((c >= 'n' && c <= 'z') || (c >= 'N' && c <= 'Z'))
        return c - 13;
    else
        return c;
}

/**
 * epoll 的性能优势：
 *      1. 避免了用户态-内核态频繁的数据拷贝：使用 poll 或 select 之前，都需要准备一个感兴趣的事件集合，系统内核拿到事件集合，进行分析并在内核空间构建相应的数据结构来完成对事件集合的注册。
 *          而使用 epoll，则维护了一个全局的事件集合，通过 epoll 句柄，可以操纵这个事件集合，增加、删除或修改这个事件集合里的某个元素。因此操纵系统内核就不需要每次重新扫描事件集合（事件变化集合没有这么大），构建内核空间数据结构。
 *      2. 在使用 poll 或者 select 之后（返回事件就绪的个数），应用程序都需要扫描整个感兴趣的事件集合，从中找出真正活动的事件。随着列表增长，每次扫描耗时也会变大（哪怕只有几个活动事件）。
 *          而使用 epoll，返回的直接就是活动的事件列表，应用程序减少了大量的扫描时间。
 *      3. epoll 支持边缘触发，比条件触发性能更优（select/poll），但同时程序处理要更小心；
 *          条件触发：只要满足事件的条件，就一直不断地把这个事件传递给用户；
 *          边缘触发：只有第一次满足条件的时候才触发，之后就不会再传递同样的事件。
 *          比如有数据需要读，只要一直不调用 read，对于 epoll_wait，前者只 wake 一次，后者会不断触发 wake
 *          由于边缘触发只通知一次，所以应用程序必须把数据读取完，否则会一直收到可读事件。
 * 
 * @param argc
 * @param argv
 * @return
 */
int main(int argc, char **argv) {
    int listen_fd, socket_fd;
    int n, i;
    int efd;

    // 注册的事件类型（设置用户需要的数据）
    struct epoll_event event;

    // 返回给用户空间需要处理的 I/O 事件，数组的大小由 epoll_wait 的返回值决定，每个元素都是一个需要待处理的 I/O 事件，
    // events 表示具体的事件类型，取值和 epoll_ctl 可设置的值一样，epoll_event 里的 data 是在 epoll_ctl 设置的 data，即用户空间和内核空间调用时需要的数据。
    struct epoll_event *events;

    // 创建非阻塞服务端实例
    listen_fd = tcp_nonblocking_server_listen(SERV_PORT);

    // 创建一个 epoll 实例（可增加如 EPOLL_CLOEXEC 的额外参数）
    efd = epoll_create1(0);
    if (efd == -1) {
        error(1, errno, "epoll create failed");
    }

    event.data.fd = listen_fd;

    // 使用 edge-triggered（边缘触发），默认则为 EPOLLLT（水平触发），select 和 poll 都是水平触发
    // https://github.com/YoungYo/yolanda/blob/master/chap-20/select02.c
    // https://github.com/YoungYo/yolanda/blob/master/chap-21/pollserver03.c
    event.events = EPOLLIN | EPOLLET;

    // 将监听套接字对应的 I/O 事件注册，在有新的连接建立之后就可以感知到
    //      EPOLL_CTL_ADD： 向 epoll 实例注册文件描述字对应的事件；
    //      EPOLL_CTL_DEL：向 epoll 实例删除文件描述字对应的事件；
    //      EPOLL_CTL_MOD： 修改文件描述字对应的事件。
    if (epoll_ctl(efd, EPOLL_CTL_ADD, listen_fd, &event) == -1) {
        error(1, errno, "epoll_ctl add listen fd failed");
    }

    /* Buffer where events are returned */
    // 为 events 事件数组分配内存
    events = calloc(MAXEVENTS, sizeof(event));

    // 主循环调用 epoll_wait 函数分发 I/O 事件，epoll_wait 成功返回时通过遍历返回的 event 数组，就知道发生的 I/O 事件
    // 由于 epoll 直接返回有事件发生的数组，相比起 poll 返回的是准备好的描述字个数，每次返回都要遍历注册的描述字数组（尤其是数量越大遍历次数越多），即使抛开阻塞和阻塞 I/O 层面，性能也有明显差异
    while (1) {

        // epoll_wait 阻塞调用的超时值设置为 -1 表示不超时（如果设置为 0 则立即返回，即使没有任何 I/O 事件发生）
        n = epoll_wait(efd, events, MAXEVENTS, -1);
        printf("epoll_wait wakeup\n");
        
        // 遍历 events 数组，获知发生的 I/O 事件
        for (i = 0; i < n; i++) {

            // 处理异常
            if ((events[i].events & EPOLLERR) || (events[i].events & EPOLLHUP) || (!(events[i].events & EPOLLIN))) {
                fprintf(stderr, "epoll error\n");
                close(events[i].data.fd);
                continue;
            }
            // 监听套接字上有事件发生（有新的连接）
            else if (listen_fd == events[i].data.fd) {
                struct sockaddr_storage ss;
                socklen_t slen = sizeof(ss);
                // 获取已建立连接
                int fd = accept(listen_fd, (struct sockaddr *) &ss, &slen);
                if (fd < 0) {
                    error(1, errno, "accept failed");
                } else {
                    // 将该连接设置为非阻塞
                    make_nonblocking(fd);
                    event.data.fd = fd;
                    event.events = EPOLLIN | EPOLLET; //edge-triggered
                    // 把已连接套接字的可读事件注册到 epoll 中：使用 event_data 的 fd 字段，将连接套接字存储其中
                    if (epoll_ctl(efd, EPOLL_CTL_ADD, fd, &event) == -1) {
                        error(1, errno, "epoll_ctl add connection fd failed");
                    }
                }
                continue;
            }
            // 处理已连接套接字上的可读事件，读取字节流，编码后回应给客户端
            else {
                socket_fd = events[i].data.fd;
                printf("get event on socket fd == %d \n", socket_fd);
                while (1) {
                    char buf[512];
                    // 由于前面设置了 make_nonblocking，socket_fd 没有数据可读会直接返回 -1，且 errno = EAGAIN
                    if ((n = read(socket_fd, buf, sizeof(buf))) < 0) {
                        if (errno != EAGAIN) {
                            error(1, errno, "read error");
                            close(socket_fd);
                        }
                        // 此处跳出了 while(1) 循环
                        break;
                    } else if (n == 0) {
                        close(socket_fd);
                        break;
                    } else {
                        for (i = 0; i < n; ++i) {
                            buf[i] = rot13_char(buf[i]);
                        }
                        if (write(socket_fd, buf, n) < 0) {
                            error(1, errno, "write error");
                        }
                    }
                }
            }
        }
    }

    free(events);
    close(listen_fd);
}
