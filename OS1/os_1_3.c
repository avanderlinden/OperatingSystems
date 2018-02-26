/*
 ============================================================================
 Name        : os_1_1.c
 Author      : Alex van der Linden
 Version     :
 Description : Prints Student name and number, and process ID.
 ============================================================================
 */
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>

int exec_cmd(char* cmd) {
    int filefd = open("stdout.txt", O_WRONLY|O_CREAT, 0666);

    if(!fork()) {
        close(1); // close stout
        //close(2); // close stderr
        dup(filefd);
        execlp("/bin/bash", "bash", "-c", cmd, NULL);
    }
    else {
        close(filefd);
        wait(NULL);
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

char* get_stdout(void) {
    char *file_contents;
    long input_file_size;

    FILE *input_file = fopen("stdout.txt", "rb");
    fseek(input_file, 0, SEEK_END);
    input_file_size = ftell(input_file);
    rewind(input_file);
    file_contents = malloc(input_file_size * (sizeof(char)));
    fread(file_contents, sizeof(char), input_file_size, input_file);
    fclose(input_file);

    return file_contents;
}

int main(void) {
    int counter = 0;
    int pid = fork();
    int *status;

    if (pid == 0) { // child code
        char pwd[1024];
        char cmd[1024];

        getcwd(pwd, sizeof(pwd));
        sprintf(cmd, "ls -alh %s", pwd);
        exec_cmd(cmd);
        printf("Result: %s\n", get_stdout());

    }
    else if (pid > 0) { // parent code
        waitpid(0, status, 0);
        printf("Child died; status %d \n", status);
    }
    else {
        printf("fail\n");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
