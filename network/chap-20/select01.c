#include "lib/common.h"

#define MAXLINE 1024

/**
 *
 *
 * @param argc
 * @param argv
 * @return
 */
int main(int argc, char **argv) {
    if (argc != 2) {
        error(1, 0, "usage: select01 <IPaddress>");
    }
    int socket_fd = tcp_client(argv[1], SERV_PORT);

    char recv_line[MAXLINE], send_line[MAXLINE];

    // 描述字集合
    fd_set readmask, allreads;

    // 描述字集合所有元素置 0
    FD_ZERO(&allreads);

    // 把标准输入对应的位置置为 1
    FD_SET(STDIN_FILENO, &allreads);

    // 把 socket_fd 的位置置为 1
    FD_SET(socket_fd, &allreads);

    for (;;) {
        // 每次 select 调用完成后都要重置待测试集合
        readmask = allreads;

        // 待测试的描述字基数，值为最大描述字 + 1，表示为从 0 ~ socket_fd 的 Bitmap：1 为检测、0 为不检测
        // select 函数的返回值：
        //      套接字接收缓冲区有数据可以读，如果使用 read 函数去执行读操作，肯定不会被阻塞，而是会直接读到这部分数据；
        //      对方发送了 FIN，使用 read 函数执行读操作，不会被阻塞，直接返回 0；
        //      针对一个监听套接字而言，有已经完成的连接建立，此时使用 accept 函数去执行不会阻塞，直接返回已经完成的连接；
        //      套接字有错误待处理，使用 read 函数去执行读操作，不阻塞，且返回 -1。
        if (select(socket_fd + 1, &readmask, NULL, NULL, NULL) <= 0) {
            error(1, errno, "select failed");
        }

        // 处理 socket_fd
        if (FD_ISSET(socket_fd, &readmask)) {
            int n = read(socket_fd, recv_line, MAXLINE);
            if (n < 0) {
                error(1, errno, "read error");
            } else if (n == 0) {
                error(1, 0, "server terminated \n");
            }
            recv_line[n] = 0;
            fputs(recv_line, stdout);
            fputs("\n", stdout);
        }

        // 处理标准输入
        if (FD_ISSET(STDIN_FILENO, &readmask)) {
            if (fgets(send_line, MAXLINE, stdin) != NULL) {
                int i = strlen(send_line);
                if (send_line[i - 1] == '\n') {
                    send_line[i - 1] = 0;
                }

                printf("now sending %s\n", send_line);
                ssize_t rt = write(socket_fd, send_line, strlen(send_line));
                if (rt < 0) {
                    error(1, errno, "write failed ");
                }
                printf("send bytes: %zu \n", rt);
            }
        }
    }

}


