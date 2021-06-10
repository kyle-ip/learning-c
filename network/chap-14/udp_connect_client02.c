//
// Created by shengym on 2019-07-12.
//

#include "lib/common.h"


# define NDG 2000    /* datagrams to send */
# define DGLEN 1400    /* length of each datagram */
# define MAXLINE 4096

/**
 * UDP connect 操作：不是通过三次握手连接，而是在应用程序与操作系统内核建立联系（套接字——目标地址+端口）；
 * 当收到一个 ICMP 不可达报文时，操作系统内核可以从映射表中找出拥有该目的地址和端口的套接字，使之可收到“Connection Refused”的信息
 * 如果没有 connect 操作，调用 recvfrom 系统调用也是可以获取客户端的地址和端口信息（UDP 报文中包含）
 * 但 UDP 套接字每次收发数据都会建立套接字连接：连接套接字->发送报文->断开套接字->连接套接字->发送报文...
 * 如果使用 connect，则每次发送成功后都不需要断开、重建连接：连接套接字->发送报文->发送报文->...->最后断开套接字
 *
 * @param argc
 * @param argv
 * @return
 */
int main(int argc, char **argv) {
    if (argc != 2) {
        error(1, 0, "usage: udpclient2 <IPaddress>");
    }

    int socket_fd = socket(AF_INET, SOCK_DGRAM, 0);

    struct sockaddr_in server_addr;
    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERV_PORT);
    inet_pton(AF_INET, argv[1], &server_addr.sin_addr);

    /* 将 UDP 套接字和 IPv4 地址和端口绑定（记录映射关系） */
    // connect 操作可以执行多次，可以重新指定新的 IP 地址和端口号，或者断开一个已连接的套接字（第二次调用 connect 时需要把套接字地址结构的地址族成员设置为 AF_UNSPEC）
    if (connect(socket_fd, (struct sockaddr *) &server_addr, sizeof(server_addr))) {
        error(1, errno, "connect failed");
    }

    char send_line[MAXLINE], recv_line[MAXLINE + 1];
    int n;
    while (fgets(send_line, MAXLINE, stdin) != NULL) {
        int i = strlen(send_line);
        if (send_line[i - 1] == '\n') {
            send_line[i - 1] = 0;
        }

        printf("now sending %s\n", send_line);

        // 使用了 connect 建立连接，需要通过 send 发送（而不是 sendto），同理要通过 recv 接收（而不是 recvfrom）
        size_t rt = send(socket_fd, send_line, strlen(send_line), 0);
        if (rt < 0) {
            error(1, errno, "send failed ");
        }
        printf("send bytes: %zu \n", rt);

        recv_line[0] = 0;
        n = recv(socket_fd, recv_line, MAXLINE, 0);
        if (n < 0)
            error(1, errno, "recv failed");
        recv_line[n] = 0;
        fputs(recv_line, stdout);
        fputs("\n", stdout);
    }

    exit(0);
}


