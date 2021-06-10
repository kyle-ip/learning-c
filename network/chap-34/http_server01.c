#include <lib/acceptor.h>
#include <lib/http_server.h>
#include "lib/common.h"
#include "lib/event_loop.h"

//数据读到buffer之后的callback
int onRequest(struct http_request *httpRequest, struct http_response *httpResponse) {
    char *url = httpRequest->url;
    char *question = memmem(url, strlen(url), "?", 1);
    char *path = NULL;
    if (question != NULL) {
        path = malloc(question - url);
        strncpy(path, url, question - url);
    } else {
        path = malloc(strlen(url));
        strncpy(path, url, strlen(url));
    }

    if (strcmp(path, "/") == 0) {
        httpResponse->statusCode = OK;
        httpResponse->statusMessage = "OK";
        httpResponse->contentType = "text/html";
        httpResponse->body = "<html><head><title>This is network programming</title></head><body><h1>Hello, network programming</h1></body></html>";
    } else if (strcmp(path, "/network") == 0) {
        httpResponse->statusCode = OK;
        httpResponse->statusMessage = "OK";
        httpResponse->contentType = "text/plain";
        httpResponse->body = "hello, network programming";
    } else {
        httpResponse->statusCode = NotFound;
        httpResponse->statusMessage = "Not Found";
        httpResponse->keep_connected = 1;
    }

    return 0;
}

/**
 * 线程启动顺序：
 *      1. event_loop_init()                            初始化 Accept thread；-> 2
 *      2. event_loop_run()                             acceptor 线程 dispatch block（与 6、7、8 一样）；-> 3
 *      3. handle_connection_established()              连接、创建 tcp_connection 对象、channel 对象；-> 4
 *      4. event_loop_add_channel_event()               从 thread_pool 取出一个线程并唤醒，执行新增连接 event（增加到线程待完成事件中，唤醒线程）；-> 6
 *      5. event_loop_run()                             thread_pool 创建，每个线程 dispatch block；-> 6
 *      6. dispatcher->dispatch()                       进入下一轮 dispatch block；-> 7
 *      7. channel_event_activate()                     查找 channel_map，找到处理函数；-> 8
 *      8. eventReadcallback()/eventWritecallback()     在线程中处理 I/O 事件；-> 9
 *      9. event_loop_handle_pending_channel()          从 dispatch block 中苏醒，执行待完成事件（增/删/改 event）；-> 6
 * @param c
 * @param v
 * @return
 */
int main(int c, char **v) {
    // 主线程 event_loop
    struct event_loop *eventLoop = event_loop_init();

    // 初始 tcp_server，可以指定线程数目，如果线程是 0，就是在这个线程里 acceptor+i/o；如果是 1，有一个 I/O 线程
    // tcp_server 自己带一个 event_loop
    struct http_server *httpServer = http_server_new(eventLoop, SERV_PORT, onRequest, 2);
    http_server_start(httpServer);

    // main thread for acceptor
    // 主 reactor 线程（acceptor），用于监听请求，分发处理
    // 当使用多个 reactor 线程时，主线程会创建多个子线程（将新创建的已连接套接字对应的读写事件交给一个），并在创建后按照主线程指定的启动函数立即运行并进行初始化；
    // 在设置了多个线程的情况下，需要 sub-reactor 线程处理，因此从 thread_pool 中取出一个线程，通知有新的事件加入；
    // 子线程是一个 event_loop 线程，阻塞在 dispatch 上，事件发生时会查找 channel_map，找到对应的处理函数并执行它，之后它就会增加、删除或修改 pending 事件，再次进入下一轮的 dispatch
    event_loop_run(eventLoop);
}