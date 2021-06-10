#include "lib/common.h"

#define MAX_LINE 4096

char
rot13_char(char c) {
    if ((c >= 'a' && c <= 'm') || (c >= 'A' && c <= 'M'))
        return c + 13;
    else if ((c >= 'n' && c <= 'z') || (c >= 'N' && c <= 'Z'))
        return c - 13;
    else
        return c;
}

void child_run(int fd) {
    char outbuf[MAX_LINE + 1];
    size_t outbuf_used = 0;
    ssize_t result;

    while (1) {
        char ch;
        result = recv(fd, &ch, 1, 0);
        if (result == 0) {
            break;
        } else if (result == -1) {
            perror("read");
            break;
        }

        /* We do this test to keep the user from overflowing the buffer. */
        if (outbuf_used < sizeof(outbuf)) {
            outbuf[outbuf_used++] = rot13_char(ch);
        }

        if (ch == '\n') {
            send(fd, outbuf, outbuf_used, 0);
            outbuf_used = 0;
            continue;
        }
    }
}


void sigchld_handler(int sig) {
    // 循环执行：如果有多个子进程同时结束，内核只会产生一次 SIGCHLD 信号，信号处理函数只会唤醒一次，通过循环取得所有已终止的子进程数据
    // WNOHANG 用来告诉内核，即使还有未终止的子进程也不要阻塞在 waitpid 上；
    // 此处不可使用 wait，wait 函数在有未终止子进程的情况下必须阻塞。
    while (waitpid(-1, 0, WNOHANG) > 0);
    return;
}

int main(int c, char **v) {
    int listener_fd = tcp_server_listen(SERV_PORT);

    // 回收子进程：当一个子进程退出，系统内核还保留了该进程的若干信息。这样的进程如果不回收，就会变成僵尸进程。
    // 
    signal(SIGCHLD, sigchld_handler);

    // 父进程关心监听套接字，当 accept 调用返回新的已连接套接字，父进程 fork 派生出一个子进程、为客户端服务
    while (1) {
        struct sockaddr_storage ss;
        socklen_t slen = sizeof(ss);
        int fd = accept(listener_fd, (struct sockaddr *) &ss, &slen);
        if (fd < 0) {
            error(1, errno, "accept failed");
            exit(1);
        }

        // fork 创建进程，把地址空间、打开的文件描述字、程序计数器、执行代码都会拷贝一份给子进程
        // 对于父进程，fork 返回的是子进程的 id；对于子进程则返回 0
        if (fork() == 0) {
            // 子进程执行代码：不需要关心监听套接字，故而在此关闭掉监听套接字 listen_fd
            close(listener_fd);
            child_run(fd);
            exit(0);
        } else {
            // 父进程执行代码
            close(fd);
        }
    }

    return 0;
}