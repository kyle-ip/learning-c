#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>     /* htons */

/**
 *
 * @param port
 * @return
 */
int make_socket(uint16_t port) {
    int sock;
    struct sockaddr_in name;

    /*
     * 创建字节流类型的 IPV4 socket.
     * @param __family      协议族，PF_INET、PF_INET6、PF_LOCAL 等
     * @param __type        套接字类型
     *                          SOCK_STREAM：字节流，对应 TCP；
     *                          SOCK_DGRAM：数据包，对应 UDP；
     *                          SOCK_RAW：原始套接字
     * @param __protocol    通信协议，已废弃，填 0 即可
     */
    sock = socket(PF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    /* 绑定到 port 和 ip. */
    name.sin_family = AF_INET;                  /* IPV4 */
    name.sin_port = htons(port);                /* 指定端口 */
    name.sin_addr.s_addr = htonl(INADDR_ANY);   /* 通配地址：应用程序不知道自己被部署到那台机器上 */

    /*
     * 把 IPV4 地址转换成通用地址格式，同时传递长度
     */
    if (bind(sock, (struct sockaddr *) &name, sizeof(name)) < 0) {
        perror("bind");
        exit(EXIT_FAILURE);
    }

    return sock;
}

int main(int argc, char **argv) {
    int sockfd = make_socket(12345);
    exit(0);
}