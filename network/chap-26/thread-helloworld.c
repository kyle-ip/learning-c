#include "lib/common.h"

int another_shared = 0;

void * thread_run(void *arg) {
    int *calculator = (int *) arg;

    // 返回线程 id
    printf("hello, world, tid == %d \n", pthread_self());

    // 此处不是线程安全的：从内存读取 i 的值到寄存器 -> 操作寄存器加 1 -> 将寄存器值写到 i 内存
    // 多个线程去操作同一个全局变量的时候，可能某个线程在第二步的时候切换到另一个线程，这样就导致少加。
    for (int i = 0; i < 1000; i++) {
        *calculator += 1;
        another_shared += 1;
    }
}

int main(int c, char **v) {
    int calculator;
    pthread_t tid1, tid2;

    pthread_create(&tid1, NULL, thread_run, &calculator);
    pthread_create(&tid2, NULL, thread_run, &calculator);

    // 主线程阻塞，直到对应 tid 的子线程自然终止（和 pthread_cancel 不同，不会强迫子线程终止）
    pthread_join(tid1, NULL);
    pthread_join(tid2, NULL);

    printf("calculator is %d \n", calculator);
    printf("another_shared is %d \n", another_shared);
}