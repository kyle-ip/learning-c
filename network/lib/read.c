#include "common.h"

/**
 * 从 socketfd 描述字中读取 size 个字节
 *
 * @param fd
 * @param buffer
 * @param size
 * @return
 */
size_t readn(int fd, void *buffer, size_t size) {
    char *buffer_pointer = buffer;
    int length = size;

    while (length > 0) {

        /**
         * @param socketfd  套接字
         * @param buffer    读缓冲区
         * @param size      最多读取 size 个字节
         * @return          返回值为 0 表示 EOF（在网络中表示对端发送了 FIN 包，要处理断连的情况）
         *                  返回值为 -1 表示出错，返回其他表示本次读取字节个数
         */
        int result = read(fd, buffer_pointer, length);

        if (result < 0) {
            /* 考虑非阻塞的情况，这里需要再次调用 read */
            if (errno == EINTR) {
                continue;
            } else {
                return (-1);
            }
        } else if (result == 0) {
            /* EOF(End of File) 表示套接字关闭 */
            break;
        }
        // 计算剩余待读取 length 个字节
        length -= result;

        // 移动缓冲区指针
        buffer_pointer += result;
    }

    /* 返回的是实际读取的字节数 */
    return (size - length);
}

size_t readline_2(int fd, char *buffer, size_t length) {
    char *buf_first = buffer;

    char c;
    while (length > 0 && recv(fd, &c, 1, 0) == 1) {
        *buffer++ = c;
        length--;
        if (c == '\n') {
            *buffer = '\0';
            return buffer - buf_first;
        }
    }

    return -1;

}

/* readline - read a newline terminated record */
/* 123456789\n */
size_t readline(int fd, char *buffer, size_t length) {
    char *buf_first = buffer;
    static char *buffer_pointer;
    int nleft = 0;
    static char read_buffer[512];
    char c;

    while (--length > 0) {
        // nread == 0
        if (nleft <= 0) {
            int nread = recv(fd, read_buffer, sizeof(read_buffer), 0);
            if (nread < 0) {
                if (errno == EINTR) {
                    length++;
                    continue;
                }
                return -1;
            }
            if (nread == 0)
                return 0;
            buffer_pointer = read_buffer;
            nleft = nread;
        }
        c = *buffer_pointer++;
        *buffer++ = c;
        nleft--;
        if (c == '\n') {
            *buffer = '\0';
            return buffer - buf_first;
        }
    }
    return -1;
}

/**
 * 解析报文，报文格式：消息长度 (4bytes) | 消息类型 (4bytes) | 请求正文
 *
 * @param fd
 * @param buffer
 * @param length
 * @return
 */
size_t read_message(int fd, char *buffer, size_t length) {
    u_int32_t msg_length, msg_type;
    int rc;

    /* Retrieve the length of the record */

    /* 读取 4bytes 的消息长度数据  */
    rc = readn(fd, (char *) &msg_length, sizeof(u_int32_t));
    if (rc != sizeof(u_int32_t)) {
        return rc < 0 ? -1 : 0;
    }
    
    /* 读取 4bytes 的消息类型数据 */
    rc = readn(fd, (char *) &msg_type, sizeof(msg_type));
    if (rc != sizeof(u_int32_t)) {
        return rc < 0 ? -1 : 0;
    }

    /* 判断 buffer 是否可以容纳下数据，如果大到本地缓冲区不能容纳，则直接返回错误  */
    msg_length = ntohl(msg_length);
    if (msg_length > length) {
        return -1;
    }

    /* Retrieve the record itself */
    /* 一次性读取已知长度的消息体。 */
    rc = readn(fd, buffer, msg_length);
    if (rc != msg_length) {
        return rc < 0 ? -1 : 0;
    }
    return rc;
}

/**
 * 解析报文，以换行符为边界
 *
 * @param fd
 * @param buf
 * @param size
 * @return
 */
int read_line(int fd, char *buf, int size) {
    int i = 0;
    char c = '\0';
    int n;

    while ((i < size - 1) && (c != '\n')) {

        // 每次尝试读取一个字节
        n = recv(fd, &c, 1, 0);
        if (n > 0) {

            // 如果读取到 \r，则判断下一个是否为换行符，是则读取该换行符
            if (c == '\r') {
                n = recv(fd, &c, 1, MSG_PEEK);
                if ((n > 0) && (c == '\n'))
                    recv(fd, &c, 1, 0);
                else
                    c = '\n';
            }
            // 没有读取到 \r，则把字符放到缓冲区，并移动指针
            buf[i] = c;
            i++;
        } else
            c = '\n';
    }
    buf[i] = '\0';

    return (i);
}
