#include "lib/common.h"

/**
 *
 * @param sockfd
 */
void read_data(int sockfd) {
    ssize_t n;
    char buf[1024];

    int time = 0;
    for (;;) {
        fprintf(stdout, "block in read\n");
        if ((n = readn(sockfd, buf, 1024)) == 0) {
            return;
        }
        time++;
        fprintf(stdout, "1K read for %d \n", time);

        /* 每次读取 1K 数据，之后休眠 1 秒，用来模拟服务器端处理时延 */
        usleep(1000);
    }
}

/**
 *
 * @param argc
 * @param argv
 * @return
 */
int main(int argc, char **argv) {
    /* 创建了 socket 套接字，bind 到对应地址和端口，并开始调用 listen 接口监听 */
    int listenfd, connfd;
    socklen_t clilen;
    struct sockaddr_in cliaddr, servaddr;

    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(12345);

    /* bind 到本地地址，端口为 12345 */
    bind(listenfd, (struct sockaddr *) &servaddr, sizeof(servaddr));
    // listen 的 backlog 为 1024，即未完成连接队列长度为 1024
    // 如果一个连接到达时该队列已满，客户端将会接收一个 ECONNREFUSED 的错误信息，如果支持重传，该请求可能会被忽略，之后会进行一次重传。
    listen(listenfd, 1024);

    /* 循环处理用户请求 */
    for (;;) {
        /* 循环等待连接，通过 accept 获取实际的连接，并开始读取数据 */
        clilen = sizeof(cliaddr);
        connfd = accept(listenfd, (struct sockaddr *) &cliaddr, &clilen);
        /* 读取数据 */
        read_data(connfd);
        /* 关闭连接套接字，注意不是监听套接字*/
        close(connfd);
    }
}

