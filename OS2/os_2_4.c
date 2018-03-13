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
    int old_value = 0;

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

        int counting_flag = TRUE;
        int break_flag = FALSE;
        while(counting_flag){
        		if (value > old_value){
				for(int i=old_value; i<=value; i++){
					led_it_shine(i);
					old_value = i;

					usleep(200000);

					if(!(value == *goal)) {
						if(*goal == -1){
							run_flag = FALSE;
							printf("Ending led count thread\n");
							break;
						}
						else {
							printf("\nNew Goal before end = %d\n", *goal);
							//i=0;
							value = *goal;
							break_flag = TRUE;
							break;
						}
					}
				}
        		}

			else if(value < old_value){
				for(int i=old_value; i>=value; i--){

				led_it_shine(i);
				old_value = i;

				usleep(200000);

				if(!(value == *goal)) {
					if(*goal == -1){
						run_flag = FALSE;
						printf("Ending led count thread\n");
						break;
					}
					else {
						printf("\nNew Goal before end = %d\n", *goal);
						//i=0;
						value = *goal;
						break_flag = TRUE;
						break;
					}
				}
			  }
            }
        		if (!break_flag){
  			  //printf("stop count\n");
  			  counting_flag = FALSE;
        		}
        		else{
        			break_flag=FALSE;
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


int main( int argc, char *argv[] ) {

    wiringPiSetup();

    pinMode(LED1, OUTPUT);
    pinMode(LED2, OUTPUT);
    pinMode(LED3, OUTPUT);
    pinMode(LED4, OUTPUT);


    int exit_code;
    int goal = 0;
    int *goal_ptr = &goal;
    int compute_run_flag = TRUE;
    int *compute_run_flag_ptr = &compute_run_flag;



    pthread_t read_and_compute_thread;
    pthread_attr_t attr;
    pthread_attr_init(&attr); //Initialize your thread attributes
    //Makes sure the thread does not inherit the attributes of its parent:
    pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
    //Sets the scheduling policy to Round Robin:
    pthread_attr_setschedpolicy(&attr, SCHED_RR);

    struct sched_param param;

    //printf("%d, %d, %d, %d\n", EAGAIN, EAGAIN, EINVAL, EPERM);

    param.sched_priority = 20; //Sets the priority of the thread
    exit_code = pthread_attr_setschedparam(&attr, &param);
    exit_code = pthread_create(&read_and_compute_thread, &attr,
                               read_and_compute, compute_run_flag_ptr);
    if(exit_code == EPERM){
        printf("Error EPERM: No permission to set the scheduling "
                "policy and parameters specified in attr\n");
    }

    pthread_t input_thread;
    exit_code = pthread_create(&input_thread, NULL,
                               wait_for_input, goal_ptr);



    pthread_t led_counter_thread;
    param.sched_priority = 10; //Sets the priority of the thread
    exit_code = pthread_attr_setschedparam(&attr, &param);
    exit_code = pthread_create(&led_counter_thread, &attr,
                               count_towards_goal, goal_ptr);
    if(exit_code == EPERM){
		printf("Error EPERM: No permission to set the scheduling "
				"policy and parameters specified in attr\n");
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

