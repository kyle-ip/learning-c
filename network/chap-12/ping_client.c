#include "lib/common.h"
#include "message_objecte.h"

#define    MAXLINE     4096
#define    KEEP_ALIVE_TIME  10
#define    KEEP_ALIVE_INTERVAL  3
#define    KEEP_ALIVE_PROBETIMES  3

/**
 *
 * @param argc
 * @param argv
 * @return
 */
int main(int argc, char **argv) {
    if (argc != 2) {
        error(1, 0, "usage: tcpclient <IPaddress>");
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

    fd_set readmask, allreads;

    // 设置 select 超时时间（保活时间），统计心跳包发送次数
    struct timeval tv;
    int heartbeats = 0;
    tv.tv_sec = KEEP_ALIVE_TIME;
    tv.tv_usec = 0;
    messageObject messageObject;

    FD_ZERO(&allreads);
    FD_SET(0, &allreads);
    FD_SET(socket_fd, &allreads);

    for (;;) {
        // 每次循环重置 readmask
        readmask = allreads;

        // 当 KEEP_ALIVE_TIME 这段时间到达之后，select 函数会返回 0
        int rc = select(socket_fd + 1, &readmask, NULL, NULL, &tv);
        if (rc < 0) {
            error(1, errno, "select failed");
        }

        // 在 KEEP_ALIVE_TIME 这段时间内客户端没有收到任何对当前连接的反馈
        if (rc == 0) {
            // 连续三个检测间隔未收到心跳报文
            if (++heartbeats > KEEP_ALIVE_PROBETIMES) {
                error(1, 0, "connection dead\n");
            }
            printf("sending heartbeat #%d\n", heartbeats);
            messageObject.type = htonl(MSG_PING);

            // 发起 PING 消息，询问服务器端是否存活
            rc = send(socket_fd, (char *) &messageObject, sizeof(messageObject), 0);
            if (rc < 0) {
                error(1, errno, "send failure");
            }
            tv.tv_sec = KEEP_ALIVE_INTERVAL;
            continue;
        }

        // 套接字上有数据返回
        // TODO 需要对报文格式转换和分析，处理 PONG 类型的回应
        if (FD_ISSET(socket_fd, &readmask)) {
            char recv_line[MAXLINE + 1];
            int n = read(socket_fd, recv_line, MAXLINE);
            if (n < 0) {
                error(1, errno, "read error");
            } else if (n == 0) {
                error(1, 0, "server terminated \n");
            }
            printf("received heartbeat, make heartbeats to 0 \n");
            heartbeats = 0;
            tv.tv_sec = KEEP_ALIVE_TIME;
        }
    }
}

