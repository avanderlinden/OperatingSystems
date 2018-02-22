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

int exec_cmd(char* cmd, char* result) {
  int exit_code;
  int pipefd[2];
  FILE *cmd_output;
  char buf[1024];
  int status;

  // create a pipe for terminal answer
  exit_code = pipe(pipefd);
  if (exit_code < 0) {
    perror("pipe");
    return -1;
  }

  // forking this process
  exit_code = fork();

  if (exit_code == 0) {
    dup2(pipefd[1], STDOUT_FILENO); // duplicate STDOUT to our pipe
    close(pipefd[0]); // close our pipe
    close(pipefd[1]);

    execl("/bin/bash", "bash", "-c", cmd, NULL); // run command using bash
  }

  //else // forking failed
  //{
  //  perror("fork");
  //  return -1;
  //}

  /* Parent process */
  close(pipefd[1]); /* Close writing end of pipe */

  cmd_output = fdopen(pipefd[0], "r");

  if (fgets(result, sizeof buf, cmd_output)) {
    printf("Data from who command: %s\n", result);
  } else {
    printf("No data received.\n");
  }

  return 0;
}

int main() {
   char cwd[1024];
   if (getcwd(cwd, sizeof(cwd)) != NULL)
       fprintf(stdout, "Current working dir: %s\n", cwd);
   else
   {
       perror("getcwd() error");
   	   return -1;
   }


   //execl("sh", "sh", "-c", "ls -alh", NULL)
   char* result = "";

   exec_cmd("ps -aux | grep \"os_1_2\"", result);

   printf("Result: %s", result);




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

