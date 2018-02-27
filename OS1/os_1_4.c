/*
 ============================================================================
 Name        : os_1_1.c
 Author      : Alex van der Linden
 Version     :
 Description :
 Notes       : Compile with: gcc os_1_4.c -o os_1_4 -lpthread
 ============================================================================
 */
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <pthread.h>

int exec_cmd(char* cmd) {
    int filefd = open("stdout.txt", O_RDONLY | O_WRONLY | O_TRUNC, 0666);

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

void *show_files() {
    char pwd[1024];
    char cmd[1024];

    sleep(1);

    getcwd(pwd, sizeof(pwd));
    sprintf(cmd, "ls -alh %s", pwd);
    exec_cmd(cmd);
    printf("Result: %s\n", get_stdout());
}

int main(void) {
    int exit_code;
    int pid = (int) getpid();
    char cmd[1024];
    pthread_t show_files_thread;

    sprintf(cmd, "ps -T -p %d ", pid);

    // show Threats before Thread is created
    exec_cmd(cmd);
    printf("ps -T -p %d :\n%s\n\n", pid, get_stdout());

    // Create a thread
    exit_code = pthread_create(&show_files_thread, NULL, show_files, NULL);
    if(exit_code > 0) {
        printf("Can't create thread");
        return EXIT_FAILURE;
    }
    else {
        printf("Created new thread\n\n");
    }

    // show Threats after Thread is created
    exec_cmd(cmd);
    printf("ps -T -p %d :\n%s\n\n", pid, get_stdout());

    // wait for the thread to finish
    exit_code = pthread_join(show_files_thread, NULL);
    if(exit_code > 0) {
        printf("Can't join thread");
        return EXIT_FAILURE;
    }

    /* Thread VS Fork
     *
     * A Thread shares the following with it parent
     * - Process instructions
     * - Most data
     * - open files (descriptors)
     * - signals and signal handlers
     * - current working directory
     * - User and group id
     *
     * With Fork the child does not share this with the parent
     * but a copy is made.
     *
     */

    return EXIT_SUCCESS;
}
