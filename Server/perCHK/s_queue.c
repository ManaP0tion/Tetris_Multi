#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#define PORT 27015
#define MAX_CLIENTS 1000

/*thread는 accpet를 한 결과를 queue에 저장하고 main은 room만 생성*/

int wait_cli[MAX_CLIENTS];
int wait_count = 0;
pthread_mutex_t lock;

typedef struct {
    int cli1;
    int cli2;
} pair;

void enqueue(int cli_socket) {
    //printf("enqueue\n");
    if (wait_count < MAX_CLIENTS) {
        wait_cli[wait_count++] = cli_socket;
    }
    else {
        printf("queue is full\n");
    }
}

int dequeue() {
    //printf("dequeue\n");
    if (wait_count == 0) {
        printf("something error mutex\n");
        return -1;
    }

    int cli_socket = wait_cli[0];

    for (int i = 1; i < wait_count; i++) {
        wait_cli[i - 1] = wait_cli[i];
    }

    wait_count--;
    return cli_socket;
}

void* cli_to_cli(void* cli_socket) {
    pair* clients = (pair*)cli_socket;
    int cli_from = clients->cli1;
    int cli_to = clients->cli2;
    char buffer[1024];
    int rlen;

    printf("cli_to_cli start: %d\n", cli_from);
    while (1) {
        if ((rlen = recv(cli_from, buffer, sizeof(buffer) - 1, 0))) {
            buffer[rlen] = '\0';
            //printf("cli %d : %s\n", cli_from, buffer);

            send(cli_to, buffer, strlen(buffer), 0);
        }

        // check cli is ok
        if (rlen <= 0) {
            printf("cli %d disconect.\n", cli_from);
            close(cli_from);
            close(cli_to);
            break;
        }
    }

    free(cli_socket);
    return NULL;
}

void make_room(int cli1, int cli2) {
    pthread_t thread1, thread2;

    char *gs_msg = "GAMESTART\n";

    //printf("room create\n");

    // game start
    send(cli1, gs_msg , strlen(gs_msg ), 0);
    send(cli2, gs_msg , strlen(gs_msg ), 0);

    // cli1 to cli2
    pair* cli_pair1 = malloc(sizeof(pair));
    cli_pair1->cli1 = cli1;
    cli_pair1->cli2 = cli2;

    if (pthread_create(&thread1, NULL, cli_to_cli, (void*)cli_pair1) != 0) {
        perror("cli1 to cli2 error");
        exit(1);
    }

    // cli2 to cli1
    pair* cli_pair2 = malloc(sizeof(pair));
    cli_pair2->cli1 = cli2;
    cli_pair2->cli2 = cli1;

    if (pthread_create(&thread2, NULL, cli_to_cli, (void*)cli_pair2) != 0) {
        perror("cli2 to cli1 error");
        exit(1);
    }

    pthread_detach(thread1);
    pthread_detach(thread2);
}

void* accept_clients(void* socket_sd) {
    int sd = *(int*)socket_sd;
    int ns;
    struct sockaddr_in csin;
    socklen_t lns = sizeof(csin);

    while ((ns = accept(sd, (struct sockaddr*)&csin, &lns)) >= 0) {
        printf("accept IP=%s,  %d\n",  inet_ntoa(csin.sin_addr),ns);

        pthread_mutex_lock(&lock);

        enqueue(ns);

        pthread_mutex_unlock(&lock);
    }

    return NULL;
}

int main() {
    int sd, ns;
    struct sockaddr_in sin, csin;
    socklen_t lns = sizeof(csin);

    memset(wait_cli, 0, sizeof(wait_cli));

    pthread_mutex_init(&lock, NULL);

    // basic server setting
    if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket");
        exit(1);
    }

    memset((char*)&sin, '\0', sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = INADDR_ANY;
    sin.sin_port = htons(PORT);

    if (bind(sd, (struct sockaddr*)&sin, sizeof(sin)) < 0) {
        perror("bind");
        exit(1);
    }

    if (listen(sd, 100) < 0) {
        perror("listen");
        exit(1);
    }

    //only accept
    pthread_t accept_thread;
    if (pthread_create(&accept_thread, NULL, accept_clients, (void*)&sd) != 0) {
        perror("pthread_create");
        exit(1);
    }

    while (1) {
        pthread_mutex_lock(&lock);
        if (wait_count >= 2) {
            int cli1 = dequeue();
            int cli2 = dequeue();

            pthread_mutex_unlock(&lock);
            make_room(cli1, cli2);
        }
        else {
            pthread_mutex_unlock(&lock);
        }

        sleep(1); // avoid busy wating
    }

    close(sd);
    pthread_mutex_destroy(&lock);
    return 0;
}
