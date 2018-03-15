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
#include <softPwm.h>

#define LED1 7
#define LED2 0
#define LED3 2
#define LED4 3

#define RANGE 100

pthread_barrier_t bar;

void *fade_out(){

    pthread_barrier_wait(&bar);

    for(int i=RANGE; i>=0; --i){
        softPwmWrite(LED1, i);
        //softPwmWrite(LED2, i);
        //softPwmWrite(LED3, i);
        //softPwmWrite(LED4, i);

        //printf("fout %d\n", i);
        usleep(100000);
    }

    return EXIT_SUCCESS;
}

void *fade_in(){

    pthread_barrier_wait(&bar);

    for(int i=0; i<=RANGE; i++){
        softPwmWrite(LED1, i);
        //softPwmWrite(LED2, i);
        //softPwmWrite(LED3, i);
        //softPwmWrite(LED4, i);

        //printf("fin %d\n", i);
        usleep(100000);
    }

    return EXIT_SUCCESS;
}


int main( int argc, char *argv[] ) {

    wiringPiSetup();

    pinMode(LED1, OUTPUT);
    pinMode(LED2, OUTPUT);
    pinMode(LED3, OUTPUT);
    pinMode(LED4, OUTPUT);

    softPwmCreate(LED1, 0, 100);
    softPwmCreate(LED2, 0, 100);
    softPwmCreate(LED3, 0, 100);
    softPwmCreate(LED4, 0, 100);


    pthread_t fade_out_thread;
    pthread_t fade_in_thread;
    int exit_code;

    pthread_barrier_init(&bar, NULL, 3);

    exit_code = pthread_create(&fade_out_thread, NULL, fade_out, NULL);
    if(exit_code > 0) {
        printf("Can't create thread\n");
        return EXIT_FAILURE;
    }
    else {
        printf("Created new fade_out_thread\n");
    }

    exit_code = pthread_create(&fade_in_thread, NULL, fade_in, NULL);
    if(exit_code > 0) {
        printf("Can't create thread\n");
        return EXIT_FAILURE;
    }
    else {
        printf("Created new fade_in_thread\n");
    }


    pthread_barrier_wait(&bar);

    // joining threads
    exit_code = pthread_join(fade_out_thread, NULL);
    if(exit_code > 0) {
        printf("Can't join thread");
        return EXIT_FAILURE;
    }
    printf("fade_out_thread ended\n");

    exit_code = pthread_join(fade_in_thread, NULL);
    if(exit_code > 0) {
        printf("Can't join thread");
        return EXIT_FAILURE;
    }
    printf("fade_in_thread ended\n");


    return EXIT_SUCCESS;
}

