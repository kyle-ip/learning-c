/**
 * Created by ywh on 02/08/2020.
 */


#include "common.h"

/**
 *
 * @param address
 * @param port
 * @return
 */
int tcp_client(char *address, int port) {
    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in server_addr;
    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    inet_pton(AF_INET, address, &server_addr.sin_addr);

    socklen_t server_len = sizeof(server_addr);
    if (connect(socket_fd, (struct sockaddr *) &server_addr, server_len) < 0) {
        error(1, errno, "connect failed ");
    }
    return socket_fd;
}

/**
 *
 * @param argc
 * @param argv
 * @return
 */
int main(int argc, char *argv[]) {
    int socket_fd = tcp_client(SERV_ADDR, SERV_PORT);
    char recv_line[MAX_LINE], send_line[MAX_LINE];
    fd_set readmask, allreads;
    FD_ZERO(&allreads);
    FD_SET(0, &allreads);
    FD_SET(socket_fd, &allreads);

    for (;;) {

        // select
        readmask = allreads;
        if (select(socket_fd + 1, &readmask, NULL, NULL, NULL) <= 0) {
            error(1, errno, "select failed");
        }

        // socket
        if (FD_ISSET(socket_fd, &readmask)) {
            int n = read(socket_fd, recv_line, MAX_LINE);
            if (n < 0) {
                error(1, errno, "read error");
            } else if (n == 0) {
                printf("server closed \n");
                break;
            }
            recv_line[n] = 0;
            fputs(recv_line, stdout);
            fputs("\n", stdout);
        }

        /* stdin */
        if (FD_ISSET(STDIN_FILENO, &readmask)) {
            if (fgets(send_line, MAX_LINE, stdin) != NULL) {
                int i = strlen(send_line);
                if (send_line[i - 1] == '\n') {
                    send_line[i - 1] = 0;
                }
                if (strncmp(send_line, "quit", strlen(send_line)) == 0) {
                    // gracefully shutdown
                    if (shutdown(socket_fd, 1)) {
                        error(1, errno, "shutdown failed");
                    }
                }
                if (write(socket_fd, send_line, strlen(send_line)) < 0) {
                    error(1, errno, "write failed ");
                }
            }
        }

    }
    exit(0);
}