/**
 * Created by ywh on 02/08/2020.
 */

#ifndef YOLANDA_COMMON_H
#define YOLANDA_COMMON_H

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <signal.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdarg.h>
#include <errno.h>

#define BUFFER_MAX 256
#define LISTENQ 1024
#define MAX_LINE 1024
#define SERV_PORT 10086
#define SERV_ADDR "localhost"

void error(int status, int err, char *fmt, ...);

#endif //YOLANDA_COMMON_H
