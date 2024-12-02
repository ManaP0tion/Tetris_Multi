#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#define PORTNUM 12056
#define BUFFER_SIZE 1024

typedef struct {
    int socket;
    struct sockaddr_in address;
} cli;

pthread_mutex_t client_mutex = PTHREAD_MUTEX_INITIALIZER; // use mutex
pthread_mutex_t check_mutex = PTHREAD_MUTEX_INITIALIZER; // use mutex
cli *first_cli = NULL; // save first client
int cli_port=PORTNUM+1;			

void *connect_cli(void *arg) {
    cli *client = (cli *)arg;
    char buffer[BUFFER_SIZE];
    int client_socket = client->socket;

    pthread_mutex_lock(&client_mutex);

    if (first_cli == NULL) {
        // first client
        printf("wait another\n");
        first_cli = client;
        pthread_mutex_unlock(&client_mutex);

        // wait to another user
        while (1) {
            pthread_mutex_lock(&client_mutex);
            if (first_cli != client) {
                pthread_mutex_unlock(&client_mutex);
                break;
            }
            pthread_mutex_unlock(&client_mutex);
            sleep(1); // avoid busy wait
        }
    } else {
        // second client
        cli *one_cli= first_cli;
        first_cli = NULL;

        printf("matching\n");
		pthread_mutex_lock(&check_mutex);
		int listen_port; // port +1
		listen_port=cli_port++;
	if(cli_port>30000){
		cli_port=PORTNUM+1;
	}
		pthread_mutex_unlock(&check_mutex);

	/* can error!! when fisrt listener!!, and PORTNUM must up one by one!!*/

        // decide who client-client
        //int listen_port = PORTNUM + 1; // port +1
        sprintf(buffer, "initiator %d", listen_port);
        send(client->socket, buffer, strlen(buffer), 0);


        // decide who client-server
        sprintf(buffer, "listener %s %d", inet_ntoa(client->address.sin_addr), listen_port);
        send(one_cli->socket, buffer, strlen(buffer), 0);


        pthread_mutex_unlock(&client_mutex);
		
        close(one_cli->socket);
        //free(one_cli);

        close(client_socket);
        //free(client);
		
        return NULL;
    }

    close(client_socket);
    free(client);

    return NULL;
}

int main() {
    int sd;
    struct sockaddr_in sin;

    // Basic server setting
    if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        exit(1);
    }
    
    int optvalue=1;
    setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &optvalue, sizeof(optvalue));


    memset((char *)&sin, '\0', sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_port = htons(PORTNUM);
    sin.sin_addr.s_addr = inet_addr("0.0.0.0");

    if (bind(sd, (struct sockaddr *)&sin, sizeof(sin))) {
        perror("bind");
        exit(1);
    }

    if (listen(sd, 10)) {
        perror("listen");
        exit(1);
    }

    while (1) {
        // client buffer, use malloc to use thread
        cli *client = malloc(sizeof(cli));
        socklen_t addrlen = sizeof(client->address);

        // accept client
        if ((client->socket = accept(sd,(struct sockaddr *)&client->address, &addrlen))==-1) {
            perror("accept");
            free(client);
			//exit(1);
            continue;
        }

        // Create thread to handle client
        pthread_t thread_id;
        pthread_create(&thread_id, NULL, connect_cli, (void *)client);
        pthread_detach(thread_id);
    }

    close(sd);
    return 0;
}

