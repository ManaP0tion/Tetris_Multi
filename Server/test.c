#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>

#define PORT 12056

void receiving(char* data);

int main() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    char buffer[1024] = {0};

    // 소켓 생성
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // 주소 구조체 설정
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // 소켓을 포트에 바인드
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    // 연결 대기
    if (listen(server_fd, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    // 클라이언트 연결 수락
    if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
        perror("accept");
        exit(EXIT_FAILURE);
    }

    // 데이터 읽기
    while (1) {
        memset(buffer, 0, 1024);
        int bytes_received = read(new_socket, buffer, 1024);
        if (bytes_received <= 0) {
            printf("disconect\n");
            break;
        }
        receiving(buffer);
        fflush(stdout);
    }

    close(new_socket);
    close(server_fd);
    return 0;
}

void receiving(char* data){
    char name[1024];
    int value;
    if(sscanf(data, "%1023[^:]:%d", name, &value) != 2){ //문자열 파싱
        printf("INVALID INPUT FORMAT\n");
        return;
    }
    if(!strcmp(name, "CLEAR")){
        printf("Value is %d\n", value);
    }
}