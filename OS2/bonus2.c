#include <wiringPi.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <inttypes.h>
#include <limits.h>
#include <pthread.h>
#include <ctype.h>
#include <math.h>
#include <time.h>

#define LED1 7
#define LED2 0
#define LED3 2
#define LED4 3

const int LedArray[4] = {LED1, LED2, LED3, LED4};


typedef struct {
    int index;
    int (*sudoku)[9][9];
} parameters;

void led_it_shine(int value){

    int ArrayLength = (int) (sizeof(LedArray) / sizeof(LedArray[0])) -1;

    for(int i = ArrayLength; i>=0; i--) {
        int mask = 0;
        mask = 1 << i;

        if(mask & value) {
            digitalWrite(LedArray[i], HIGH);
            printf("1");
        }
        else {
            digitalWrite(LedArray[i], LOW);
            printf("0");
        }
    }
    printf("\n");
}

void busyWait(int s) {
    clock_t now = clock();
    while (clock() < now + s*(CLOCKS_PER_SEC/1000)) {};
}

void *wait_for_input(void *goal_ptr){
    int *goal = (int *)goal_ptr;
    int run_flag = TRUE;
    int value = 0;
    char *endptr;
    char str_value[1024];


    while(run_flag) {
        printf("goal>");
        fflush(stdout);
        fgets(str_value, 1024, stdin);

        if ((str_value[0] == '-') && (str_value[1] == '1')){
            run_flag = FALSE;
            *goal = -1;
            break;
        }

        int no_digit = FALSE;

        for(int i = 0; i < sizeof(str_value); i++) {
            if(str_value[i]=='\n')
                break;

            if(isdigit(str_value[i]) == 0){
             printf("no digit\n");
             no_digit = TRUE;
            }
        }

        str_value[strcspn(str_value, "\n")] = 0;
        value = (int) strtol(str_value, &endptr, 10);

        if (value < 0 || value > 15 || no_digit){
            printf("Invalid input provided, must be integer between 0 and 15\n");
            continue;
        }
        else {
            *goal = value;
            usleep(1000);
        }
    }
    printf("Ending input thread\n");
    return EXIT_SUCCESS;

}

void *count_towards_goal(void *goal_ptr){
    int run_flag =  TRUE;
    int *goal = (int *)goal_ptr;
    int value = *goal;

    while(run_flag){
        while(value == *goal){}

        if (*goal == -1) {
            run_flag = FALSE;
            printf("Ending led count thread\n");
            break;
        }
        else {
            value = *goal;
            printf("\nNew Goal = %d\n", value);
        }

        for(int i=0; i<=value; i++){
            led_it_shine(i);
            busyWait(500);
            //usleep(500000);

            if(!(value == *goal)) {
                if(*goal == -1){
                    run_flag = FALSE;
                    printf("Ending led count thread\n");
                    break;
                }
                else {
                    printf("\nNew Goal before end = %d\n", *goal);
                    i=0;
                    value = *goal;
                }
            }
        }
    }
    return EXIT_SUCCESS;
}

void *read_and_compute(void *run_flag_ptr){
    int *run_flag = (int *)run_flag_ptr;

    FILE *fp;
    char *line = NULL;
    size_t len = 0;
    ssize_t read;

    double value = 0.0;
    double result = 0.0;

    fp = fopen("./data.txt", "r");
    if (fp == NULL) {
        printf("Error reading file\n");
        *run_flag = FALSE;
    }

    clock_t start_clk = clock();

    while (((read = getline(&line, &len, fp)) != -1) && (*run_flag == TRUE)) {
        value = (double) strtod(line, NULL);
        result += atan(tan(value));
    }

    clock_t clk_end = clock();
    double time = (double)(clk_end - start_clk) / CLOCKS_PER_SEC;


    if(*run_flag){
        printf("\nResult: %2f\n", result);
        printf("Time[ms]: %f1\n", time);
    }
    else {
        printf("\nResult so far: %2f\n", result);
        printf("Time so far [ms]: %f1\n", time);
    }

    fclose(fp);
    if (line)
        free(line);

    return EXIT_SUCCESS;

}


void *check_row(void * args){
    parameters* params = (parameters *) args;
    printf("check_row thread created %d\n", params->index);



    return EXIT_SUCCESS;
}

void *check_colum(void * args){
    printf("check_colum thread created\n");
    return EXIT_SUCCESS;
}


void *check_sub(void * args){
    printf("check_sub thread created\n");
    return EXIT_SUCCESS;
}



int main( int argc, char *argv[] ) {
    FILE *fp;
    char *line = NULL;
    size_t len = 0;
    ssize_t read;

    int sudoku[9][9];
    pthread_t row_threads[9];
    pthread_t colum_threads[9];
    pthread_t sub_threads[9];
    //void * exitCodes[9][9];

    fp = fopen("./sudoku.txt", "r");
    if (fp == NULL) {
        printf("Error reading file\n");
    }


    int row = 0;
    int colum = 0;


    while ((read = getline(&line, &len, fp)) != -1) {
        for (int j = 0; j < strlen(line); j++){
            if(isdigit(line[j])){
                sudoku[row][colum]= ((int)line[j] - 48);
                //printf("[%d][%d]: %d \n",row, colum, sudoku[row][colum]);
                colum++;

            }
        }
        row++;
        colum=0;
    }



    for(int i = 0; i<9; i++){

        //static parameters args;
        //static parameters* args_ptr = &args;

        parameters *params = (parameters *) malloc(sizeof(parameters));
        params->sudoku = &sudoku;
        params->index = i;

        pthread_create(&row_threads[i], NULL, check_row, params);
        pthread_create(&colum_threads[i], NULL, check_colum, params);
        pthread_create(&sub_threads[i], NULL, check_sub, params);
    }


    usleep(1000000);

//    for(int i = 0; i<9; i++){
//        for(int j = 0; j<9; j++){
//            printf("%d ",sudoku[i][j]);
//        }
//        printf("\n");
//    }







//    pthread_t input_thread;
//    pthread_t led_counter_thread;
//
//    exit_code = pthread_create(&input_thread, NULL,
//                               wait_for_input, goal_ptr);
//
//    exit_code = pthread_create(&led_counter_thread, NULL,
//                               count_towards_goal, goal_ptr);
//
//
//
//    exit_code = pthread_join(input_thread, NULL);
//    if(exit_code > 0) {
//        printf("Can't join thread");
//        return EXIT_FAILURE;
//    }
//
//    exit_code = pthread_join(led_counter_thread, NULL);
//    if(exit_code > 0) {
//        printf("Can't join thread");
//        return EXIT_FAILURE;
//    }
//    printf("led counter thread ended\n");
//
//    compute_run_flag = FALSE;
//
//    exit_code = pthread_join(read_and_compute_thread, NULL);
//    if(exit_code > 0) {
//        printf("Can't join thread");
//        return EXIT_FAILURE;
//    }
//    printf("read and compute thread ended\n");

    return EXIT_SUCCESS;
}






