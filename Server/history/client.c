#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#define PORTNUM 12056
#define BUFFER_SIZE 1024

volatile int check = 1; // check another connect is ok

void *checkOK(void *arg) {
    int another = *(int *)arg;
    char buffer[BUFFER_SIZE];

    while (check) {
        memset(buffer, 0, BUFFER_SIZE);
        int bytes_received = read(another, buffer, BUFFER_SIZE);
        if (bytes_received <= 0) {
            printf("disconect\n");
            check = 0;
            break;
        }
        printf("another: %s", buffer);
        fflush(stdout);
    }

    return NULL;
}

void chating(int another) {
    char message[BUFFER_SIZE];

    pthread_t check_thread;
    pthread_create(&check_thread, NULL, checkOK, &another);

    while (check) {
        printf("me: ");
        fflush(stdout);
        fgets(message, BUFFER_SIZE, stdin);

        send(another, message, strlen(message), 0);
    }

    pthread_cancel(check_thread); // Stop the receiving thread
    close(another);
}

void servering(const char *peer_ip, int second_ns) {
    int another;
    struct sockaddr_in another_sin;

    if ((another = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        return;
    }

    another_sin.sin_family = AF_INET;
    another_sin.sin_port = htons(second_ns);
    inet_pton(AF_INET, peer_ip, &another_sin.sin_addr);

    if (connect(another, (struct sockaddr *)&another_sin, sizeof(another_sin)) == -1) {
        perror("connect");
        close(another);
        return;
    }

    printf("connect success\n");

    chating(another);
}

void listening(int listen_ns) {
    int lns, another;
    struct sockaddr_in listen_sin;

    if ((lns = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        return;
    }

    listen_sin.sin_family = AF_INET;
    listen_sin.sin_addr.s_addr = INADDR_ANY;
    listen_sin.sin_port = htons(listen_ns);

    if (bind(lns, (struct sockaddr *)&listen_sin, sizeof(listen_sin)) == -1) {
        perror("bind");
        close(lns);
        return;
    }

    if (listen(lns, 1) == -1) {
        perror("listen");
        close(lns);
        return;
    }

    printf("wait\n");

    if ((another = accept(lns, NULL, NULL)) == -1) {
        perror("accept");
        close(lns);
        return;
    }

    printf("connect success\n");

    chating(another);
    close(lns);
}

int main() {
    int ns;
    struct sockaddr_in sin;
    char buffer[BUFFER_SIZE] = {0};

    while (1) {
        if ((ns = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
            perror("socket");
    			printf("error1");
			exit(1);
            continue;
        }

	memset((char *)&sin, '\0', sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_port = htons(PORTNUM);
	sin.sin_addr.s_addr = inet_addr("127.0.0.1");

        if (connect(ns, (struct sockaddr *)&sin, sizeof(sin)) == -1) {
            perror("connect");
            close(ns);
			printf("error2");
			exit(1);
            continue;
        }

        if (read(ns, buffer, BUFFER_SIZE) <= 0) {
            perror("read");
            close(ns);
        	printf("error3");
			exit(1);
        continue;
        }

		printf("--%s--\n",buffer);

        char anther_ip[INET_ADDRSTRLEN]; //16 server ip length
        int second_ns;

        // decide who make server
        if (sscanf(buffer, "listener %s %d", anther_ip, &second_ns) == 2) {
            printf("im listener\n");
			close(ns);
            servering(anther_ip, second_ns);
        } else if (sscanf(buffer, "initiator %d", &second_ns) == 1) {
            printf("im initiator\n");
			close(ns);
            listening(second_ns);
        }
    }

    return 0;
}

   /*
       int another = *(int *)arg;
    char buffer[BUFFER_SIZE];

    while (check) {
        memset(buffer, 0, BUFFER_SIZE);
        int bytes_received = read(another, buffer, BUFFER_SIZE);
        if (bytes_received <= 0) {
            printf("disconect\n");
            check = 0;
            break;
        }
        printf("another: %s", buffer);
        fflush(stdout);
    }

    return NULL;
    */