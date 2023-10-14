#ifndef SERVER_H
#define SERVER_H // TODO: #ifdef __cplusplus

typedef struct server *server_t;

int server_create(server_t *server);
int server_run(server_t server, int port, int conn_queue_len, void(*handle_request)(int));
void server_stop(server_t server);
void server_destroy(server_t *server);

#endif //SERVER_H
