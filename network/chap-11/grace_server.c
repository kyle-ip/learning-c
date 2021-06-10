//
// Created by shengym on 2019-07-07.
//
#include "lib/common.h"

static int count;

/**
 *
 * @param signo
 */
static void sig_pipe(int signo) {
    printf("\nreceived %d datagrams\n", count);
    exit(0);
}

/**
 *
 * @param argc
 * @param argv
 * @return
 */
int main(int argc, char **argv) {

    /* 创建了一个 TCP 套接字 */
    int listenfd, connfd;
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    listenfd = socket(AF_INET, SOCK_STREAM, 0);

    /* 设置了本地服务器 IPv4 地址，绑定到 ANY 地址和指定的端口，监听、接收客户端请求 */
    struct sockaddr_in server_addr;
    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(SERV_PORT);

    if (bind(listenfd, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) {
        error(1, errno, "bind failed ");
    }
    if (listen(listenfd, LISTENQ) < 0) {
        error(1, errno, "listen failed ");
    }
    if ((connfd = accept(listenfd, (struct sockaddr *) &client_addr, &client_len)) < 0) {
        error(1, errno, "bind failed ");
    }

    signal(SIGINT, sig_pipe);
    signal(SIGPIPE, SIG_DFL);

    char message[MAXLINE], send_line[MAXLINE];
    count = 0;
    for (;;) {
        int n = read(connfd, message, MAXLINE);
        if (n < 0) {
            error(1, errno, "error read");
        } else if (n == 0) {
            error(1, 0, "client closed \n");
        }

        /* 显示收到的字符串，对原字符串进行重新格式化，休眠 5s 模拟服务端处理时间，调用 send 函数将数据发送给客户端 */
        message[n] = 0;
        printf("received %d bytes: %s\n", n, message);
        count++;
        sprintf(send_line, "Hi, %s", message);

        sleep(5);
        int write_nc = send(connfd, send_line, strlen(send_line), 0);
        printf("send bytes: %d \n", write_nc);
        if (write_nc < 0) {
            error(1, errno, "error write");
        }
    }

}


