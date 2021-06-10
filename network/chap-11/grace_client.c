# include "lib/common.h"

# define MAXLINE 4096

/**
 *
 * @param argc
 * @param argv
 * @return
 */
int main(int argc, char **argv) {
    if (argc != 2) {
        error(1, 0, "usage: graceclient <IPaddress>");

    }
    /* 创建 TCP 套接字，设置 IPv4 地址（清零、设置地址族，指定端口，转换地址），绑定指定的 IP 和端口 */
    int socket_fd;
    struct sockaddr_in server_addr;
    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERV_PORT);
    inet_pton(AF_INET, argv[1], &server_addr.sin_addr);

    /* 试用套接字连接 server */
    if (connect(socket_fd, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) {
        error(1, errno, "connect failed ");
    }

    char send_line[MAXLINE], recv_line[MAXLINE + 1];
    int n;

    fd_set readmask, allreads;

    /* 为 select 多路复用做准备，初始化描述字集合：同时处理连接套接字和标准输入两个 I/O 对象 */
    FD_ZERO(&allreads);
    FD_SET(0, &allreads);
    FD_SET(socket_fd, &allreads);

    for (;;) {
        readmask = allreads;
        int rc = select(socket_fd + 1, &readmask, NULL, NULL, NULL);
        if (rc <= 0) {
            error(1, errno, "select failed");
        }
        /* 连接套接字上有数据可读，将数据读入到程序缓冲区中 */
        if (FD_ISSET(socket_fd, &readmask)) {
            n = read(socket_fd, recv_line, MAXLINE);
            /* 读取异常，退出 */
            if (n < 0) {
                error(1, errno, "read error");
            }
            /* 读取到服务器发送的 EOF，正常退出 */
            else if (n == 0) {
                error(1, 0, "server terminated \n");
            }
            recv_line[n] = 0;
            fputs(recv_line, stdout);
            fputs("\n", stdout);
        }
        /* 标准输入上有数据可读，读入后判断 */
        if (FD_ISSET(0, &readmask)) {
            if (fgets(send_line, MAXLINE, stdin) != NULL) {

                // 读取到 shutdown，关闭标准输入 I/O 事件感知，调用 shutdown 函数关闭写方向，但此时读方向未关闭，仍然可以接收来自服务端的消息
                // shutdown()：可选择关闭读方向（悄悄丢弃，不告知对方）、写方向（不管引用计数，直接关闭；最后一次发送缓冲区数据和 FIN 报文），或两个方向都关闭
                // 当期望关闭连接其中一个方向时，应该使用 shutdown 函数
                if (strncmp(send_line, "shutdown", 8) == 0) {
                    FD_CLR(0, &allreads);
                    if (shutdown(socket_fd, 1)) {
                        error(1, errno, "shutdown failed");
                    }
                }

                // 读取到 close，调用 close 函数关闭连接（不能收到应答数据），但未必立即关闭数据（只有当引用计数为 0 才立即关闭）
                // close()：通过引用计数判断套接字是否可关闭（为 0 关闭），当多个进程共享套接字时，每调用调用 fork 创建子进程计数 +1，每调用一次 close 计数 -1
                // 在输入方向，系统内核会将该套接字设置为不可读，任何读操作都会返回异常。
                // 在输出方向，系统内核尝试将发送缓冲区的数据发送给对端，最后向对端发送一个 FIN 报文，随后再对该套接字进行写操作会返回异常。
                // 如果对端没有检测到套接字已关闭、还继续发送报文，就会收到一个 RST 报文，告知对端停止发送报文。
                else if (strncmp(send_line, "close", 5) == 0) {
                    FD_CLR(0, &allreads);
                    if (close(socket_fd)) {
                        error(1, errno, "close failed");
                    }
                    sleep(6);
                    exit(0);
                }
                /* 处理正常的输入，将回车符截掉，调用 write 函数，通过套接字将数据发送给服务器端 */
                else {
                    size_t i = strlen(send_line);
                    if (send_line[i - 1] == '\n') {
                        send_line[i - 1] = 0;
                    }
                    printf("now sending %s\n", send_line);
                    size_t rt = write(socket_fd, send_line, strlen(send_line));
                    if (rt < 0) {
                        error(1, errno, "write failed ");
                    }
                    printf("send bytes: %zu \n", rt);
                }
            }
        }
    }
}

