#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include <errno.h>

#include "../inc/app/server.h"
#include "../inc/app/thread_pool.h"
#include "../inc/http/events_handler.h"
#include "../lib/log/log.h"

#define PORT 8080
#define REQUEST_BUFFER_SIZE 1024
#define THREAD_POOL_SIZE 7
#define CONN_QUEUE_LEN 1024
#define LOG_USE_COLOR // TODO

static server_t server = NULL;
static thread_pool_t thread_pool = NULL;
static http_events_handler_t events_handler = NULL;

//char** tokenize_string(const char* str, const char* delimiter, int* num_tokens) {
//    char* temp_str = strdup(str);
//    char** tokens = (char**)malloc(strlen(temp_str) * sizeof(char*));
//    char* token = strtok(temp_str, delimiter);
//    int token_count = 0;
//
//    while (token != NULL) {
//        tokens[token_count] = strdup(token);
//        token = strtok(NULL, delimiter);
//        token_count++;
//    }
//
//    free(temp_str);
//    *num_tokens = token_count;
//
//    return tokens;
//}
//
//void handle_http_request(int socket_fd) {
//    char request[REQUEST_BUFFER_SIZE];
//    memset(request, 0, REQUEST_BUFFER_SIZE);
//    log_info("process request from client (fd = %d) ...", socket_fd);
//    ssize_t n = read(socket_fd, request, REQUEST_BUFFER_SIZE - 1);
//    if (n < 0) {
//        log_error("read(): %s", strerror(errno));
//        return;
//    }
//    request[n] = '\0';
//
//    int num_tokens;
//    char** tokens = tokenize_string(request, "\r\n", &num_tokens);
//    for (int i = 0; i < num_tokens; i++) {
//        log_debug("token %d (len %lu): %s", i, strlen(tokens[i]), tokens[i]);
//    }
//
//    char *response = "HTTP/1.1 200 OK\nContent-Type: text/plain\n\nHello, world!";
//    send(socket_fd, response, strlen(response), 0);
//    log_info("reply for client (fd = %d) sent", socket_fd);
//}

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

        handle_http_event(events_handler, client_socket);

        close(client_socket);
        ((task_t *)task)->socket_fd = -1;
        free(task);
    }

    pthread_exit(&rc);
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
    http_events_handler_destroy(&events_handler);

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

int main() {
    log_set_level(LOG_INFO);
    int rc;
    if ((rc = server_create(&server)) != EXIT_SUCCESS) {
        return rc;
    }
    if ((rc = thread_pool_create(&thread_pool, THREAD_POOL_SIZE)) != 0) {
        return rc;
    }
    if ((rc = http_events_handler_create(&events_handler)) != 0) {
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
