#include "lib/common.h"

#define INIT_SIZE 128

/**
 * 相较于 select，poll 的改进：
 *      可以突破 select 1024 个描述字的限制，在高并发场景下更占优势
 *
 * @param argc
 * @param argv
 * @return
 */
int main(int argc, char **argv) {
    int listen_fd, connected_fd, ready_number;
    ssize_t n;
    char buf[MAXLINE];
    struct sockaddr_in client_addr;

    listen_fd = tcp_server_listen(SERV_PORT);

    // 初始化 pollfd 数组（pollfd：描述字、检测事件、返回事件），数组的第一个元素是 listen_fd，其余的用来记录将要连接的 connect_fd
    // 可改进为动态分配数组的方式（见 poll_server02），内存不够进行 realloc 或者申请一块更大的内存，把原数组拷贝过来
    struct pollfd event_set[INIT_SIZE];
    event_set[0].fd = listen_fd;

    // 系统内核检测监听套接字上的连接建立完成事件
    event_set[0].events = POLLRDNORM;

    // 用 -1 表示这个数组位置未被占用
    int i;
    for (i = 1; i < INIT_SIZE; i++) {
        event_set[i].fd = -1;
    }

    for (;;) {
        // 调用 poll，返回这次 I/O 事件描述字的个数（自动忽略 fd 为 -1 的 pollfd）
        if ((ready_number = poll(event_set, INIT_SIZE, -1)) < 0) {
            error(1, errno, "poll failed ");
        }

        // 位与操作，判断该描述字发生 POLLRDNORM 事件
        if (event_set[0].revents & POLLRDNORM) {
            socklen_t client_len = sizeof(client_addr);
            connected_fd = accept(listen_fd, (struct sockaddr *) &client_addr, &client_len);

            // 在数组中找到一个可用于记录该连接套接字的位置
            for (i = 1; i < INIT_SIZE; i++) {
                if (event_set[i].fd < 0) {
                    event_set[i].fd = connected_fd;
                    event_set[i].events = POLLRDNORM;
                    break;
                }
            }
            // 找不到（event_set 已经被连接填满），报错
            if (i == INIT_SIZE) {
                error(1, errno, "can not hold so many clients");
            }

            // 如果处理完监听套接字之后，就已经完成了这次 I/O 复用所要处理的事情，则跳过后面的处理，再次进入 poll 调用
            if (--ready_number <= 0) {
                continue;
            }
        }

        // 循环检查 event_set 里面其他的事件
        for (i = 1; i < INIT_SIZE; i++) {
            int socket_fd;
            if ((socket_fd = event_set[i].fd) < 0) {
                continue;
            }
            //  revents 的事件类型是 POLLRDNORM 或者 POLLERR，可以进行读操作
            if (event_set[i].revents & (POLLRDNORM | POLLERR)) {
                if ((n = read(socket_fd, buf, MAXLINE)) > 0) {
                    if (write(socket_fd, buf, n) < 0) {
                        error(1, errno, "write error");
                    }
                }
                // 如果读到 EOF 或者是连接重置，则关闭这个连接，并且把 event_set 对应的 pollfd 重置
                else if (n == 0 || errno == ECONNRESET) {
                    close(socket_fd);
                    event_set[i].fd = -1;
                }
                // 读取数据失败
                else {
                    error(1, errno, "read error");
                }

                if (--ready_number <= 0)
                    break;
            }
        }
    }
}
