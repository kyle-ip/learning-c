//
// Created by shengym on 2019-07-07.
//

#include "lib/common.h"
#include "message_objecte.h"

static int count;

/**
 *
 * @param signo
 */
static void sig_int(int signo) {
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
    if (argc != 2) {
        error(1, 0, "usage: tcpsever <sleepingtime>");
    }
    int sleepingTime = atoi(argv[1]);
    int listenfd = socket(AF_INET, SOCK_STREAM, 0), connfd;
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);


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
    signal(SIGINT, sig_int);
    signal(SIGPIPE, SIG_IGN);

    messageObject message;
    count = 0;
    for (;;) {
        int n = read(connfd, (char *) &message, sizeof(messageObject));
        if (n < 0) {
            error(1, errno, "error read");
        } else if (n == 0) {
            error(1, 0, "client closed \n");
        }

        printf("received %d bytes\n", n);
        count++;

        switch (ntohl(message.type)) {
            case MSG_TYPE1 :
                printf("process MSG_TYPE1 \n");
                break;
            case MSG_TYPE2 :
                printf("process MSG_TYPE2 \n");
                break;
            // 处理来自 client 的 PING 消息，并休眠模拟响应处理时间，再响应 PONG 报文
            case MSG_PING: {
                messageObject pong_message;
                pong_message.type = MSG_PONG;

                /* 接收到 ping 报文后，暂停 sleepingTime 后返回 pong */
                sleep(sleepingTime);
                ssize_t rc = send(connfd, (char *) &pong_message, sizeof(pong_message), 0);
                if (rc < 0) {
                    error(1, errno, "send failure");
                }
                break;
            }
            // 消息格式无法识别
            default :
                error(1, 0, "unknown message type (%d)\n", ntohl(message.type));
        }

    }

}


