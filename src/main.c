#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include <errno.h>

#include "server.h"
#include "thread_pool.h"
#include "events_handler.h"
#include "log.h"

#define PORT 8080
#define REQUEST_BUFFER_SIZE 1024
#define THREAD_POOL_SIZE 7
#define CONN_QUEUE_LEN 1024

static server_t server = NULL;
static thread_pool_t thread_pool = NULL;

typedef struct task {
    int socket_fd;
} task_t;

void *worker_thread(void *arg) {
    thread_pool_t pool = (thread_pool_t)arg;
    int rc = EXIT_SUCCESS;
    while (1) {
        void *task = NULL;
        if ((rc = thread_pool_take_task(&task, pool)) != EXIT_SUCCESS) {
            log_error("thread_pool_take_task(): %s", strerror(errno));
            pthread_exit(&rc);
        }
        int client_socket = ((task_t *)task)->socket_fd;

        handle_http_event(client_socket);

        close(client_socket);
        ((task_t *)task)->socket_fd = -1;
        free(task);
    }
}

void handle_request(int socket_fd) {
    task_t *task = malloc(sizeof(task_t));
    if (task == NULL) {
        log_error("handle_request(fd = %d) malloc (): %s", socket_fd, strerror(errno));
        log_info("request(fd = %d) cannot be handled; skip", socket_fd);
        return;
    }
    task->socket_fd = socket_fd;
    thread_pool_submit(thread_pool, task);
}

void server_shutdown(server_t s)
{
    log_info("shutdown server...");

    server_stop(s);
    server_destroy(&s);

    thread_pool_destroy(&thread_pool);

    log_info("server stopped");
    exit(EXIT_SUCCESS);
}

void signal_handler(int signum)
{
    switch (signum) {
        case SIGINT:
            log_debug("Signal SIGINT received");
            server_shutdown(server);
            break;
        case SIGTERM:
            log_debug("Signal SIGTERM received");
            server_shutdown(server);
            break;
        default:
            log_debug("unknown signal received: %d", signum);
    }
}

int main(void) {
    // log_set_level(LOG_INFO);
    int rc;
    if ((rc = server_create(&server)) != EXIT_SUCCESS) {
        return rc;
    }
    if ((rc = thread_pool_create(&thread_pool, THREAD_POOL_SIZE)) != 0) {
        return rc;
    }

    if ((rc = thread_pool_start(thread_pool, worker_thread)) != EXIT_SUCCESS) {
        return rc;
    }

    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    if ((rc = server_run(server, PORT, CONN_QUEUE_LEN, handle_request)) != EXIT_SUCCESS) {
        return rc;
    }

    return EXIT_SUCCESS;
}
