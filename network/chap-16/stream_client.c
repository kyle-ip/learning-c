#include "lib/common.h"

int main(int argc, char **argv) {
    if (argc != 2) {
        error(1, 0, "usage: tcpclient <IPaddress>");
    }

    int socket_fd;
    socket_fd = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in server_addr;
    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERV_PORT);
    inet_pton(AF_INET, argv[1], &server_addr.sin_addr);

    socklen_t server_len = sizeof(server_addr);
    if (connect(socket_fd, (struct sockaddr *) &server_addr, server_len) < 0) {
        error(1, errno, "connect failed ");
    }

    // 报文格式转化为结构体
    struct {
        // 4bytes
        u_int32_t message_length;
        // 4bytes
        u_int32_t message_type;
        char data[128];
    } message;

    int n;
    while (fgets(message.data, sizeof(message.data), stdin) != NULL) {
        n = strlen(message.data);
        // 转换网络字节序
        message.message_length = htonl(n);
        message.message_type = 1;
        if (send(socket_fd, (char *) &message, sizeof(message.message_length) + sizeof(message.message_type) + n, 0) <0)
            error(1, errno, "send failure");
    }
    exit(0);
}

