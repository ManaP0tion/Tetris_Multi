#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define MAXFORK 100

int main() {
    pid_t pid;
    char arg[10];

    for (int i = 1; i <= MAXFORK; i++) {
        pid = fork();

        if (pid < 0) {
            perror("Fork failed");
            exit(1);
        }

        if (pid == 0) {
            snprintf(arg, sizeof(arg), "%d", i);
            execl("./s_client", "./s_client", arg, (char *)NULL);
            perror("execl failed");
            exit(1);
        }
    }

    for (int i = 1; i <= MAXFORK; i++) {
        wait(NULL);
    }

    printf("end\n");

    return 0;
}
