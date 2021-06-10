#include "lib/common.h"

#define  THREAD_NUMBER      4
#define  BLOCK_QUEUE_SIZE   100

extern void loop_echo(int);

typedef struct {
    pthread_t thread_tid;        /* thread ID */
    long thread_count;    /* # connections handled */
} Thread;

Thread *thread_array;

// 队列
typedef struct {
    int number;                     // 队列里的描述字最大个数
    int *fd;                        // 数组指针
    int front;                      // 当前队列的头位置
    int rear;                       // 当前队列的尾位置
    int count;                      // 队列中当前元素数目
    pthread_mutex_t mutex;          // 锁
    // pthread_cond_t cond;            // 条件变量
    pthread_cond_t not_empty;       // 条件变量：未空
    pthread_cond_t not_full;        // 条件变量：未满
} block_queue;

// 初始化
void block_queue_init(block_queue *blockQueue, int number) {
    blockQueue->number = number;
    blockQueue->fd = calloc(number, sizeof(int));
    blockQueue->count = blockQueue->front = blockQueue->rear = 0;
    pthread_mutex_init(&blockQueue->mutex, NULL);
    pthread_cond_init(&blockQueue->not_empty, NULL);
    pthread_cond_init(&blockQueue->not_full, NULL);
}

// 往队列里放置一个描述字 fd
void block_queue_push(block_queue *blockQueue, int fd) {
    // 一定要先加锁，因为有多个线程需要读写队列
    pthread_mutex_lock(&blockQueue->mutex);
    
    // 队列已满
    while (blockQueue->count == blockQueue->number) { 
        pthread_cond_wait(&blockQueue->not_full, &blockQueue->mutex);
    }
    
    // 将描述字放到队列尾的位置
    blockQueue->fd[blockQueue->rear] = fd;
    // 如果已经到最后，重置尾的位置
    if (++blockQueue->rear == blockQueue->number) {
        blockQueue->rear = 0;
    }
    blockQueue->count++;

    // 通知其他等待读的线程，有新的连接字等待处理
     pthread_cond_signal(&blockQueue->not_empty);

    // 解锁
    pthread_mutex_unlock(&blockQueue->mutex);
}

// 从队列里读出描述字进行处理
int block_queue_pop(block_queue *blockQueue) {
    // 加锁
    pthread_mutex_lock(&blockQueue->mutex);
    // 判断队列里没有新的连接字可以处理，就一直条件等待，直到有新的连接字入队列
    // 在 while 循环中执行：为了确保被 pthread_cond_wait 唤醒之后的线程，确实可以满足继续往下执行的条件（如果没有 while 循环的再次确认，可能直接就往下执行了）。
    while (blockQueue->front == blockQueue->rear) {
        pthread_cond_wait(&blockQueue->not_empty, &blockQueue->mutex);
    }
    // 取出队列头的连接字
    int fd = blockQueue->fd[blockQueue->front];
    // 如果已经到最后，重置头的位置
    if (++blockQueue->front == blockQueue->number) {
        blockQueue->front = 0;
    }
    blockQueue->count--;
    pthread_cond_signal(&blockQueue->not_full);
    // 解锁
    pthread_mutex_unlock(&blockQueue->mutex);
    // 返回连接字
    return fd;
}  

void thread_run(void *arg) {
    pthread_t tid = pthread_self();
    pthread_detach(tid);

    block_queue *blockQueue = (block_queue *) arg;
    while (1) {
        int fd = block_queue_pop(blockQueue);
        printf("get fd in thread, fd==%d, tid == %d", fd, tid);
        loop_echo(fd);
    }
}

int main(int c, char **v) {
    int listener_fd = tcp_server_listen(SERV_PORT);

    block_queue blockQueue;
    block_queue_init(&blockQueue, BLOCK_QUEUE_SIZE);

    thread_array = calloc(THREAD_NUMBER, sizeof(Thread));

    // 预创建线程，组成线程池（thread_run 会被阻塞，因为 blockQueue 中没有描述字）
    for (int i = 0; i < THREAD_NUMBER; i++) {
        pthread_create(&(thread_array[i].thread_tid), NULL, &thread_run, (void *) &blockQueue);
    }

    while (1) {
        struct sockaddr_storage ss;
        socklen_t slen = sizeof(ss);
        int fd = accept(listener_fd, (struct sockaddr *) &ss, &slen);
        if (fd < 0) {
            error(1, errno, "accept failed");
        } else {
            block_queue_push(&blockQueue, fd);
        }
    }

    return 0;
}