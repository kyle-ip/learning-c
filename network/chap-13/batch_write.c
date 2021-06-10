#include "lib/common.h"


/* 发送窗口：作为单 TCP 连接、点对点之间的流量控制模型，需要和接收端一起共同协调来调整大小
   拥塞窗口：作为多个 TCP 连接共享带宽的拥塞控制模型，发送端独立地根据网络状况来动态调整 */
int main(int argc, char **argv) {
    if (argc != 2) {
        error(1, 0, "usage: batchwrite <IPaddress>");
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

    /* Nagle 算法：限制大批量的小数据包同时发送，在任何一个时刻，未被确认的小数据包（小于最大报文段长度 MSS 的 TCP 分组）不能超过一个。因此同一个请求得部分报文可能会被延迟发送。 */
    /* 消除 Nagle 算法的副作用（时延敏感应用）：集中写，即在写数据之前，将数据合并到缓冲区，批量发送出去 */
    
    // iovec 数组写入两个字符串，一个是 send_one，另一个通过标准输入读入，合并发出
    char buf[128];
    struct iovec iov[2];
    char *send_one = "hello,";
    iov[0].iov_base = send_one;
    iov[0].iov_len = strlen(send_one);
    iov[1].iov_base = buf;
    while (fgets(buf, sizeof(buf), stdin) != NULL) {
        iov[1].iov_len = strlen(buf);
        int n = htonl(iov[1].iov_len);

        // writev, readv
        if (writev(socket_fd, iov, 2) < 0) {
            error(1, errno, "writev failure");
        }
    }
    exit(0);
}

