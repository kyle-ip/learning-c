//
// Created by shengym on 2019-07-07.
//

#include "lib/common.h"

static int count;

static void recvfrom_int(int signo) {
    printf("\nreceived %d datagrams\n", count);
    exit(0);
}


int main(int argc, char **argv) {
    int socket_fd;
    socket_fd = socket(AF_INET, SOCK_DGRAM, 0);

    struct sockaddr_in server_addr;
    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(SERV_PORT);

    bind(socket_fd, (struct sockaddr *) &server_addr, sizeof(server_addr));

    char message[MAXLINE];
    count = 0;

    // 处理 SIGINT 信号：在响应“Ctrl+C”退出时，打印出收到的报文总数。
    signal(SIGINT, recvfrom_int);

    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    for (;;) {
        // 不需要像 TCP 那样通过 accept 函数获取描述字来确定对端信息，直接 recvfrom 即可获取数据（报文之间没有上下文）
        int n = recvfrom(socket_fd, message, MAXLINE, 0, (struct sockaddr *) &client_addr, &client_len);
        message[n] = 0;
        printf("received %d bytes: %s\n", n, message);

        char send_line[MAXLINE];
        sprintf(send_line, "Hi, %s", message);
        sendto(socket_fd, send_line, strlen(send_line), 0, (struct sockaddr *) &client_addr, client_len);

        count++;
    }

}


