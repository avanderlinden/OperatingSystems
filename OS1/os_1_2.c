/*
 ============================================================================
 Name        : os_1_1.c
 Author      : Alex van der Linden
 Version     :
 Description : Creates a folder in app's dir and prints content of app's dir
 Notes       : Compile with: make os_1_2
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

int main() {
    char pwd[1024];
    char cmd[1024];

    // get current working directory
    getcwd(pwd, sizeof(pwd));

    // remove folder
    // sprintf(cmd, "rm -r %s/folder", pwd);
    // exec_cmd(cmd);

    // create folder
    sprintf(cmd, "mkdir %s/folder", pwd);
    exec_cmd(cmd);

    // list dir content
    sprintf(cmd, "ls -alh %s", pwd);
    exec_cmd(cmd);


    printf("Result: %s\n", get_stdout());

    return EXIT_SUCCESS;
}
