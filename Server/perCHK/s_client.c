#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/time.h>

#define PORT 27015

int check_loop = 1;

typedef struct {
    int sd;
    FILE* file;
} target_socket;

void* receive_message(void* targeting) {
    target_socket* ts = (target_socket*)targeting;
    int sd = ts->sd;
    char buffer[1024];
    int rlen;

    FILE* file = ts->file;
    struct timeval t1, t2;

    while (check_loop) {
        gettimeofday(&t1, NULL);
        rlen = recv(sd, buffer, sizeof(buffer) - 1, 0);
        buffer[rlen] = '\0';
        gettimeofday(&t2, NULL);

        if (rlen > 0) {
            //printf("received: %s\n", buffer);
            double performance_time = (t2.tv_sec + t2.tv_usec * 0.000001) - (t1.tv_sec + t1.tv_usec * 0.000001);
            //printf("time: %f\n", performance_time);
            fprintf(file, "%f\n", performance_time);
            fflush(file);
        }
        //printf("something error\n");
        sleep(1);
    }

    return NULL;
}

/*
void* receive_message(void* socket) {
    int sd = *(int*)socket;
    char buffer[1024];
    int rlen;


    printf("recieve start\n");
    while (1) {
        rlen = recv(sd, buffer, sizeof(buffer) - 1, 0);
        buffer[rlen] = '\0';
        printf("received: %s\n", buffer);
        printf("recieve loop\n");

    if (rlen == 0) {
        printf("rlen 0\n");
        break;
    }

    if (rlen < 0) {
        perror("rlen < 0\n");
        break;
    }
    }

    return NULL;
}
*/

void generate_matrix(int matrix[10][20]) {
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 20; j++) {
            matrix[i][j] = rand() % 4;
        }
    }
}

void send_matrix(int sd, int matrix[10][20]) {
    
    char matrix_str[10 * 20 * 2 + 1];
    int inumber = 0;

    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 20; j++) {
            inumber += sprintf(&matrix_str[inumber], "%d,", matrix[i][j]);
        }
    }
    matrix_str[strlen(matrix_str) - 1] = '\0';

    send(sd, matrix_str, strlen(matrix_str), 0);
    //printf("Matrix sent\n");
    
   /*
   char buffer[1024];
    printf("me: ");
    scanf("%s", buffer);

    send(sd, buffer, strlen(buffer), 0);
    */
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        printf("argc\n");
        exit(1);
    }

    FILE* file, *go_file;
    char filename[256];
    char go_filename[256];

    sprintf(filename, "%s.txt", argv[1]);
    sprintf(go_filename, "%s_go.txt", argv[1]);

    file = fopen(filename, "a");
    if (file == NULL) {
        perror("fopen");
        exit(1);
    }
    go_file = fopen(go_filename, "a");
    if (go_file == NULL) {
        perror("fopen");
        exit(1);
    }

    int sd;
    struct sockaddr_in sin;
    pthread_t thread1;
    int matrix[10][20];


    if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        exit(1);
    }

    memset((char*)&sin, '\0', sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_port = htons(PORT);
    sin.sin_addr.s_addr = inet_addr("127.0.0.1");

    struct timeval t1, t2;
    double connect_time;

    gettimeofday(&t1, NULL);
    if (connect(sd, (struct sockaddr*)&sin, sizeof(sin))) {
        perror("connect");
        exit(1);
    }

    gettimeofday(&t2, NULL);

    connect_time = (t2.tv_sec + t2.tv_usec * 0.000001) - (t1.tv_sec + t1.tv_usec * 0.000001);
    fprintf(go_file, "%f\n", connect_time);
    fclose(go_file);

    target_socket ts;
    ts.sd = sd;
    ts.file = file;

    char buffer[1024];
    int rlen;
    while (1) {
        rlen = recv(sd, buffer, sizeof(buffer) - 1, 0);
        if (rlen > 0) {
            buffer[rlen] = '\0';         
            if (strcmp(buffer, "GAMESTART") == 0) {
                //printf("game start\n");
                break;
            }
            //printf("GAMESTART loop\n");
        }
    }
    /*
    if (pthread_create(&thread1, NULL, receive_message, (void*)&sd) != 0) {
        perror("pthread_create");
        exit(1);
    }
    */
    
    if (pthread_create(&thread1, NULL, receive_message, (void*)&ts) != 0) {
        perror("pthread_create");
        exit(1);
    }

    for (int main_loop = 0; main_loop < 10; main_loop++) {
        sleep(1);
        //printf("loop\n");
        generate_matrix(matrix);
        send_matrix(sd, matrix);
    }

    check_loop = 0;


    pthread_join(thread1, NULL);

    close(sd);
    fclose(file);

    printf("end cli: %s\n",argv[1]);
    return 0;
}