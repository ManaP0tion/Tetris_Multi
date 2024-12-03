#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/time.h>

#define PORTNUM 27015
#define BUFFER_SIZE 1024

pthread_mutex_t check_mutex = PTHREAD_MUTEX_INITIALIZER;

volatile int check = 1; // check another connect is ok
int check_loop = 1;


typedef struct {
    int sd;
    FILE* file;
} target_socket;


void generate_matrix(int matrix[10][20]) {
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 20; j++) {
            matrix[i][j] = rand() % 4;
        }
    }
}


void *checkOK(void *arg) {
    
    target_socket *t_socket = (target_socket *)arg; 
    int another = t_socket->sd;
    FILE *file = t_socket->file;
    char buffer[BUFFER_SIZE];
    
    struct timeval t1, t2;

    while (check&&check_loop) {
        //printf("loop\n");
        memset(buffer, 0, BUFFER_SIZE);
        //gettimeofday(&t1, NULL);

        int bytes_received = recv(another, buffer, BUFFER_SIZE,0);

        //gettimeofday(&t2, NULL);
        
        //double performance_time = (t2.tv_sec + t2.tv_usec * 0.000001) - (t1.tv_sec + t1.tv_usec * 0.000001);
        //printf("time: %f\n", performance_time);
        //fprintf(file, "%f\n", performance_time);
        //fflush(file);
        if (bytes_received <= 0) {
            printf("disconect\n");
		pthread_mutex_lock(&check_mutex);
            check = 0;
        pthread_mutex_unlock(&check_mutex);
            break;
        }
        //printf("another: %s", buffer);
        //fflush(stdout);
    }

    return NULL;
    
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
}

void chating(int another, char *filename) {
//void chating(int another) {
    
    int matrix[10][20];
    char message[BUFFER_SIZE];

    char matrix_str[10 * 20 * 2 + 1];
    int inumber = 0;
    FILE *file = fopen(filename, "a");
    //FILE *file = fopen("a.txt", "a");
    if (file == NULL) {
        perror("file");
        return;
    }

    
    target_socket t_socket;
    t_socket.sd = another;
    t_socket.file = file;
    

    pthread_t check_thread;
    pthread_create(&check_thread, NULL, checkOK, &t_socket);

        generate_matrix(matrix);
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 20; j++) {
            inumber += sprintf(&matrix_str[inumber], "%d,", matrix[i][j]);
        }
    }
    matrix_str[strlen(matrix_str) - 1] = '\0';
    
    for (int main_loop = 0; main_loop < 100; main_loop++) {
    
    send(another, matrix_str, strlen(matrix_str), 0);
    //printf("Matrix sent\n");
    sleep(1);
    }
    check_loop = 0;

    close(another);
    
   /*
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
    */
}

void servering(const char *peer_ip, int second_ns, char *filename) {
//void servering(const char *peer_ip, int second_ns) {
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
        perror("connect22");
	    printf("error99\n");
        close(another);
        return;
    }

    printf("connect success\n");

    chating(another, filename);
    //chating(another);
}

void listening(int listen_ns, char *filename) {
//void listening(int listen_ns) {
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

    if (listen(lns, 10) == -1) {
        perror("listen");
        close(lns);
        return;
    }

    //printf("wait\n");

    if ((another = accept(lns, NULL, NULL)) == -1) {
        perror("accept");
        close(lns);
        return;
    }

    printf("connect success\n");

    //chating(another);
    chating(another,filename);
    close(lns);
}


int main(int argc, char* argv[]) {
    if (argc != 2) {
        printf("argc\n");
        exit(1);
    }

    FILE*go_file;
    char go_filename[256];
    char filename[256];
    sprintf(filename, "%s.txt", argv[1]);
    sprintf(go_filename, "%s_go.txt", argv[1]);

    go_file = fopen(go_filename, "a");
    if (go_file == NULL) {
        perror("fopen");
        exit(1);
    }
    

    int ns;
    struct sockaddr_in sin;
    char buffer[BUFFER_SIZE] = {0};
        int rlen;

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
            perror("connect11");
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
            //printf("im listener\n");
			close(ns);
            //servering(anther_ip, second_ns);
            servering(anther_ip, second_ns,filename);
        } else if (sscanf(buffer, "initiator %d", &second_ns) == 1) {
            //printf("im initiator\n");
			close(ns);
           //listening(second_ns);
            listening(second_ns,filename);
        }
    }

    printf("end cli: %s\n",argv[1]);
    return 0;
}
