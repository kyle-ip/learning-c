/**
 * Created by ywh on 02/08/2020.
 */

#include "common.h"

static int count;

/**
 *
 * @param signo
 */
static void sig_int(int signo) {
    printf("\nreceived %d datagrams\n", count);
    exit(0);
}

/**
 *
 * @return
 */
char *pwd() {
    size_t path_size = 128;
    char *ret;
    while (1) {
        if ((ret = (char *) malloc(path_size)) == NULL) {
            return NULL;
        }
        if (getcwd(ret, path_size) != NULL) {
            return ret;
        }
        if (errno != ERANGE) {
            free(ret);
            return NULL;
        } else {
            path_size = path_size * 2;
            free(ret);
        }
        if (path_size == 0) {
            return NULL;
        }
    }
}

/**
 *
 * @return
 */
char *ls() {
    char *data = malloc(16384);
    bzero(data, sizeof(data));
    FILE *fdp;
    char buffer[BUFFER_MAX];
    fdp = popen("ls", "r");
    char *data_index = data;
    if (!fdp) {
        return data;
    }
    while (!feof(fdp)) {
        if (fgets(buffer, BUFFER_MAX, fdp) != NULL) {
            int len = strlen(buffer);
            memcpy(data_index, buffer, len);
            data_index += len;
        }
    }
    pclose(fdp);
    return data;
}

/**
 *
 * @param argc
 * @param argv
 * @return
 */
int main(int argc, char *argv[]) {

    printf("OK!");

    // server socket & addr
    int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in server_addr;
    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(SERV_PORT);

    // addr reuse
    int on = 1;
    setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

    // bind, listen
    if (bind(listen_fd, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) {
        error(1, errno, "bind failed");
    }
    if (listen(listen_fd, LISTENQ) < 0) {
        error(1, errno, "listen failed");
    }

    signal(SIGPIPE, SIG_IGN);

    // client
    int conn_fd;
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);

    char buffer[BUFFER_MAX];
    count = 0;

    // handle multi connections
    while (1) {

        if ((conn_fd = accept(listen_fd, (struct sockaddr *) &client_addr, &client_len)) < 0) {
            error(1, errno, "accept failed");
        }

        while (1) {
            // read error
            bzero(buffer, sizeof(buffer));
            int n = read(conn_fd, buffer, sizeof(buffer));
            if (n < 0) {
                error(1, errno, "error read message");
            } else if (n == 0) {
                printf("client closed \n");
                close(conn_fd);
                break;
            }

            // read
            count++;
            buffer[n] = 0;
            if (strncmp(buffer, "ls", n) == 0) {
                char *ret = ls();
                if (send(conn_fd, ret, strlen(ret), 0) < 0) {
                    return 1;
                }
            } else if (strncmp(buffer, "pwd", n) == 0) {
                char *ret = pwd();
                if (send(conn_fd, pwd, strlen(ret), 0) < 0) {
                    return 1;
                }
            } else if (strncmp(buffer, "cd ", 3) == 0) {
                char target[BUFFER_MAX];
                bzero(target, sizeof(target));
                memcpy(target, buffer + 3, strlen(buffer) - 3);
                if (chdir(target) == -1) {
                    printf("change dir failed, %s\n", target);
                }
            } else {
                char *error = "error: unknown input type";
                if (send(conn_fd, error, strlen(error), 0) < 0) {
                    return 1;
                }
            }
        }
    }
    exit(0);
}