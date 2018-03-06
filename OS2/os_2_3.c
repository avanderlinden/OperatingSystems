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

void led_it_shine(int value){

    int ArrayLength = (int) (sizeof(LedArray) / sizeof(LedArray[0])) -1;
    //printf("3\\ .. /0\n  ");

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
            //printf("%d", i);
            led_it_shine(i);
            usleep(500000);

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
        printf("Result: %2f\n", result);
        printf("Time[ms]: %f1\n", time);
    }
    else {
        printf("Result so far: %2f\n", result);
        printf("Time so far [ms]: %f1\n", time);
    }

    fclose(fp);
    if (line)
        free(line);

    return EXIT_SUCCESS;

}


int main( int argc, char *argv[] ) {

    wiringPiSetup();

    pinMode(LED1, OUTPUT);
    pinMode(LED2, OUTPUT);
    pinMode(LED3, OUTPUT);
    pinMode(LED4, OUTPUT);


    pthread_t input_thread;
    pthread_t led_counter_thread;
    pthread_t read_and_compute_thread;
    int exit_code;
    int goal = 0;
    int *goal_ptr = &goal;

    exit_code = pthread_create(&input_thread, NULL, wait_for_input, goal_ptr);
    if(exit_code > 0) {
        printf("Can't create thread");
        return EXIT_FAILURE;
    }
    else {
        printf("Created new input thread\n");
    }

    exit_code = pthread_create(&led_counter_thread, NULL,
                               count_towards_goal, goal_ptr);
    if(exit_code > 0) {
        printf("Can't create thread");
        return EXIT_FAILURE;
    }
    else {
        printf("Created new led counter thread\n");
    }

    int compute_run_flag = TRUE;
    int *compute_run_flag_ptr = &compute_run_flag;

    exit_code = pthread_create(&read_and_compute_thread, NULL,
                               read_and_compute, compute_run_flag_ptr);
    if(exit_code > 0) {
        printf("Can't create thread");
        return EXIT_FAILURE;
    }
    else {
        printf("Created new compute thread\n");
    }




    exit_code = pthread_join(input_thread, NULL);
    if(exit_code > 0) {
        printf("Can't join thread");
        return EXIT_FAILURE;
    }

    exit_code = pthread_join(led_counter_thread, NULL);
    if(exit_code > 0) {
        printf("Can't join thread");
        return EXIT_FAILURE;
    }
    printf("led counter thread ended\n");

    compute_run_flag = FALSE;

    exit_code = pthread_join(read_and_compute_thread, NULL);
    if(exit_code > 0) {
        printf("Can't join thread");
        return EXIT_FAILURE;
    }
    printf("read and compute thread ended\n");

    return EXIT_SUCCESS;
}

