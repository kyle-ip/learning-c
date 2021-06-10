#include "lib/common.h"

#define    MAXLINE     4096
#define    KEEP_ALIVE_TIME  10
#define    KEEP_ALIVE_INTERVAL  3
#define    KEEP_ALIVE_PROBETIMES  3

/**
 * 向 fd 写入 n 字节数
 *
 * @param fd
 * @param data
 * @param n
 * @return
 */
ssize_t writen(int fd, const void *data, size_t n) {
    size_t nleft;
    ssize_t nwritten;
    const char *ptr;

    ptr = data;
    nleft = n;
    //如果还有数据没被拷贝完成，就一直循环
    while (nleft > 0) {
        if ((nwritten = write(fd, ptr, nleft)) <= 0) {
            /* 这里 EAGAIN 是非阻塞 non-blocking 情况下，通知我们再次调用 write() */
            if (nwritten < 0 && errno == EAGAIN)
                nwritten = 0;
            else {
                return -1;         /* 出错退出 */
            }
        }
        /* 指针增大，剩下字节数变小*/
        nleft -= nwritten;
        ptr += nwritten;
    }
    return n;
}

/**
 *
 * @param argc
 * @param argv
 * @return
 */
int main(int argc, char **argv) {
    if (argc != 2) {
        error(1, 0, "usage: nonblockingclient <IPaddress>");
    }

    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in server_addr;
    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERV_PORT);
    inet_pton(AF_INET, argv[1], &server_addr.sin_addr);

    if (connect(socket_fd, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) {
        error(1, errno, "connect failed ");
    }

    // SO_LINGER 套接字选项，连接被关闭时 TCP 套接字上将会发送一个 RST
    struct linger ling;
    ling.l_onoff = 1;
    ling.l_linger = 0;
    setsockopt(socket_fd, SOL_SOCKET, SO_LINGER, &ling, sizeof(ling));
    close(socket_fd);
    exit(0);
}

