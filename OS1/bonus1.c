#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <string.h>

#define MAX_LINE 80 /* The maximum length command */

char history[10][1024];
int arrayCnt = 0;


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

    int status;
    waitpid(0, &status, 0);

    close(filefd);

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
    fread(file_contents, sizeof(char), input_file_size-1, input_file);
    fclose(input_file);

    return file_contents;
}

void memorize_cmd(char* cmd)
{
    if (arrayCnt < 10)
    {
        strcpy(history[arrayCnt], cmd);
        arrayCnt++;
    }
    else
    {
        for (int i=0; i<10; i++)
        {
            strcpy(history[i], history[i+1]);
        }
        strcpy(history[9], cmd);
    }

//    for (int j=0; j<arrayCnt-1; j++)
//    {
//       printf("%s; ", history[j]);
//    }
//    printf("\n");
}

void print_history(void)
{
    for (int j=0; j<arrayCnt; j++)
    {
       printf("%d\t%s\n", j, history[j]);
    }
}

int main(void) {
    char *args[MAX_LINE/2 +1]; /* command line arguments */
    int should_run = 1; /* flag to determine when to exit program */

    char cmd[1024];

    while (should_run) {
        printf("osh>");
        fflush(stdout);
        fgets(cmd, 1024, stdin);
        cmd[strcspn(cmd, "\n")] = 0;

        //printf("cmd: %s\n", cmd);

        if(strcmp(cmd,"history") == 0){
            //memorize_cmd(cmd);
            print_history();
        }
        else if((char)cmd[0]=='!' && (char)cmd[1]=='!') {
            if (arrayCnt == 0) {
                printf("No such command\n");
            }
            else {
                strcpy(cmd, history[arrayCnt-1]);
                memorize_cmd(cmd);
                exec_cmd(cmd);
                printf("%s\n",get_stdout());
            }

        }
        else if((char)cmd[0]=='!' && (int)cmd[1] > 47 && (int)cmd[1] < 58) {
            int index = (int)cmd[1] - 48;

            if (index > arrayCnt-1) {
                printf("No such command\n");
            }
            else {
                strcpy(cmd, history[index]);
                memorize_cmd(cmd);
                exec_cmd(cmd);
                printf("%s\n",get_stdout());
            }
        }
        else {
            memorize_cmd(cmd);
            exec_cmd(cmd);
            printf("%s\n",get_stdout());
        }

        /**
        * After reading user input, the steps are:
        * (1) fork a child process using fork()
        * (2) the child process will invoke execvp()
        * (3) if command did not include &, parent will invoke wait()
        */
    }

    return 0;
}

