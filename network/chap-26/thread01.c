#include "lib/common.h"

extern void loop_echo(int);

void thread_run(void *arg) {

    // 线程分离，每个子线程把自己设置为分离，就能在它终止后自动回收相关的线程资源，不需要主线程调用 pthread_join 函数
    pthread_detach(pthread_self());
    int fd = (int) arg;
    loop_echo(fd);
}

int main(int c, char **v) {
    int listener_fd = tcp_server_listen(SERV_PORT);
    pthread_t tid;

    while (1) {
        struct sockaddr_storage ss;
        socklen_t slen = sizeof(ss);

        // 阻塞，直到新连接建立，阻塞调用返回，再 pthread_create 创建一个子线程处理连接
        int fd = accept(listener_fd, (struct sockaddr *) &ss, &slen);
        if (fd < 0) {
            error(1, errno, "accept failed");
        } else {
            // 第三个参数为线程入口函数，第四个参数为函数的参数（多个参数需要使用结构体封装），转换为 void * 类型，
            pthread_create(&tid, NULL, &thread_run, (void *) fd);
        }
    }

    return 0;
}