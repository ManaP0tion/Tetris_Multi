#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#define PORT 27015

int cli_socket[2];
pthread_mutex_t lock;

void* cli_to_cli(void* socket) {
    int ans = *(int*)socket;
    char buffer[1024];
    int rlen;

    /* receive data */
        printf("cli_to_cli start: %d\n", cli_from);
    while ((rlen = recv(ans, buffer, sizeof(buffer) - 1, 0)) {
        buffer[rlen] = '\0';
        printf("receive: %s\n", buffer);

        /* send data to all other clients */
        pthread_mutex_lock(&lock);
        for (int i = 0; i < 2; i++) {
            if (cli_socket[i] != 0 && (cli_socket[i] != ans)) {
                send(cli_socket[i], buffer, strlen(buffer), 0);
                printf("another send %d: %s\n", i, buffer);
            }
        }
        pthread_mutex_unlock(&lock);
    }

    /* When disconnected */
    printf("disconnect\n");
    close(ans);
    pthread_mutex_lock(&lock);

    /*chk ns, ns is to 0*/
    for (int i = 0; i < 2; i++) {
        if (cli_socket[i] == ans) {
            cli_socket[i] = 0;
            break;
        }
    }
    pthread_mutex_unlock(&lock);
    free(socket);
    return NULL;
}

int main() {
    int sd, ns, * ns_thread;
    struct sockaddr_in sin, csin;

    socklen_t lns = sizeof(csin);

    memset(cli_socket, 0, sizeof(cli_socket));

    pthread_mutex_init(&lock, NULL);

    // basic server setting
    if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        exit(1);
    }

    memset((char*)&sin, '\0', sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = INADDR_ANY;
    sin.sin_port = htons(PORT);

    if (bind(sd, (struct sockaddr*)&sin, sizeof(sin))) {
        perror("bind");
        exit(1);
    }

    if (listen(sd, 3)) {
        perror("listen");
        exit(1);
    }

    while ((ns = accept(sd, (struct sockaddr*)&csin, &lns)) >= 0) {
        printf("accept!\n");

        pthread_mutex_lock(&lock);

        /*chk all socket*/
        int chk = 0;
        for (int i = 0; i < 2; i++) {
            if (cli_socket[i] == 0) {
                cli_socket[i] = ns;
                chk = 1;
                break;
            }
        }
        pthread_mutex_unlock(&lock);

        if (chk ==0) {
            printf("slot full\n");
            close(ns);
            continue;
        }

        pthread_t thread_id;
        ns_thread = malloc(sizeof(ns));
        *ns_thread = ns;
        if (pthread_create(&thread_id, NULL, cli_to_cli, (void*)ns_thread) != 0) {
            perror("pthread_create");
            return 1;
        }

        pthread_detach(thread_id);
    }

    close(sd);
    pthread_mutex_destroy(&lock);
    return 0;
}
