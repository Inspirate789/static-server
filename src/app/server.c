#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "server.h"
#include "log.h"

struct server {
    int server_socket_fd;
    bool is_running;
};

int server_create(server_t *server) {
    server_t tmp_server = malloc(sizeof(struct server));
    if (tmp_server == NULL) {
        log_error("server_init malloc(): %s", strerror(errno));
        return errno;
    }

    if ((tmp_server->server_socket_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        log_error("socket(): %s", strerror(errno));
        free(*server);
        return EXIT_FAILURE;
    }

    int opt = IP_PMTUDISC_WANT;
    if (setsockopt(tmp_server->server_socket_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        log_error("setsockopt(): %s", strerror(errno));
        free(tmp_server);
        return EXIT_FAILURE;
    }

    tmp_server->is_running = false;
    *server = tmp_server;

    return EXIT_SUCCESS;
}

int server_run(server_t server, int port, int conn_queue_len, void(*handle_request)(int)) {
    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    if (bind(server->server_socket_fd, (struct sockaddr*)&address, sizeof(address)) == -1) {
        log_error("bind(): %s", strerror(errno));
        return errno;
    }

    if (listen(server->server_socket_fd, conn_queue_len) == -1) {
        log_error("listen(): %s", strerror(errno));
        return errno;
    }

    log_info("server started on port %d; wait for connections...", port);

    server->is_running = true;
    fd_set client_fds;
    while (server->is_running) {
        FD_ZERO(&client_fds);
        FD_SET(server->server_socket_fd, &client_fds);

        if (select(server->server_socket_fd + 1, &client_fds, NULL, NULL, NULL) == -1) {
            log_error("select(): %s", strerror(errno));
            return errno;
        }

        int client_socket_fd = -1;
        if (FD_ISSET(server->server_socket_fd, &client_fds)) {
            if ((client_socket_fd = accept(server->server_socket_fd, (struct sockaddr*)NULL, NULL)) == -1) {
                log_error("accept(): %s", strerror(errno));
                return errno;
            }

            handle_request(client_socket_fd);
        }
    }

    return EXIT_SUCCESS;
}

void server_stop(server_t server) {
    if (server == NULL) {
        return;
    }
    log_info("waiting for processing all requests...");
    server->is_running = false;
    log_info("request processing finished");
}

void server_destroy(server_t *server) {
    if (server == NULL || *server == NULL) {
        return;
    }
    close((*server)->server_socket_fd);
    (*server)->server_socket_fd = -1;
    free(*server);
    *server = NULL;
}
