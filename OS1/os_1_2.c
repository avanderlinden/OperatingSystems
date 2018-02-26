/*
 ============================================================================
 Name        : os_1_1.c
 Author      : Alex van der Linden
 Version     :
 Description :
 ============================================================================
 */

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>

int exec_cmd(char* cmd, char result[]) {
	int pid;
	int pipefd[2];
	FILE *cmd_output;
	char buf[1024];
	int status;

	// create a pipe for terminal answer
	pid = pipe(pipefd);
	if (pid < 0) {
	perror("pipe");
	return -1;
	}

	// forking this process
	pid = fork();
	if(pid < 0) {
	  perror("fork");
	  exit(-1);
	}

	// run command
	if (pid == 0) {
	dup2(pipefd[1], STDOUT_FILENO); // duplicate STDOUT to our pipe
	close(pipefd[0]); // close our pipe
	close(pipefd[1]);

	execl("/bin/bash", "bash", "-c", cmd, NULL); // run command using bash
	}

	close(pipefd[1]); /* Close writing end of pipe */
	cmd_output = fdopen(pipefd[0], "r");

	fgets(result, sizeof buf, cmd_output); // write file to result

	return 0;
}

int main() {
    char pwd[1024];
    char cmd[1024];
    char result[] = "";

    // get current working directory
    getcwd(pwd, sizeof(pwd));

    sprintf(cmd, "mkdir %s/folder", pwd);

    exec_cmd(cmd, result); // create folder
    //result = "";
    exec_cmd(cmd, result); // create folder
    //printf("Result: %s\n", result);




    return EXIT_SUCCESS;
}


//int cmd_quem(void) {
//  int result;
//  int pipefd[2];
//  FILE *cmd_output;
//  char buf[1024];
//  int status;
//
//  result = pipe(pipefd);
//  if (result < 0) {
//    perror("pipe");
//    exit(-1);
//  }
//
//  result = fork();
//  if(result < 0) {
//    exit(-1);
//  }
//
//  if (result == 0) {
//    dup2(pipefd[1], STDOUT_FILENO); /* Duplicate writing end to stdout */
//    close(pipefd[0]);
//    close(pipefd[1]);
//
//    execl("/usr/bin/who", "who", NULL);
//    _exit(1);
//  }
//
//  /* Parent process */
//  close(pipefd[1]); /* Close writing end of pipe */
//
//  cmd_output = fdopen(pipefd[0], "r");
//
//  if (fgets(buf, sizeof buf, cmd_output)) {
//    printf("Data from who command: %s\n", buf);
//  } else {
//    printf("No data received.\n");
//  }

