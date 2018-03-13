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



struct Node {
    int brightness;
    int duration; //Stored values
    struct Node* next;
};
struct Queue {
    struct Node* front;
    struct Node* back;
    pthread_mutex_t* mutex;
};

//First node in the queue
//Last node in the queue
//Used for mutual exclusion
//Adds a new node to the back of the queue
void addToQueue(struct Queue* queue, int brightness, int duration) {
    struct Node *node = malloc(sizeof *node);
    node->brightness = brightness;
    node->duration = duration;
    node->next = NULL;

    if(queue->front == NULL){
        queue->front = node;
        queue->back = node;
    }
    else {
        queue->back->next = node;
        queue->back = node;
    }

}
//Removes a node from the front of the queue
//The memory of the removed node has to be freed
//Returns the values of the removed node via pointers
//If the queue is empty return -1 for both the brightness and the duration
void removeFromQueue(struct Queue* queue, int* pBrightness, int* pDuration) {
// Implement
}
//Returns the length of the queue
int queueSize(struct Queue* queue) {
    int size = 0;
    struct Node* node;
    //pthread_mutex_lock(queue->mutex);

    if(queue->back == NULL){
        size = 0;
    }
    else {
        size++;
        node = queue->front;
        while(node->next != NULL){
            size++;
            node = node->next;
        }
    }

    //pthread_mutex_unlock(queue->mutex);
    return size;

}
//Initializes the values of an already allocated queue struct
void initQueue(struct Queue* queue) {
    printf("init q\n");

    queue->back = NULL;
    queue->front = NULL;
    pthread_mutex_t mutex;
    pthread_mutex_t* mutex_ptr = &mutex;
    queue->mutex = mutex_ptr;

    //printf("%#010x\n", mutex_ptr);

    if (pthread_mutex_init(mutex_ptr, NULL) != 0)
    {
        printf("\n mutex init failed\n");
    }
    //pthread_mutex_unlock(mutex_ptr);
}







//void *fade_out(){
//
//    pthread_barrier_wait(&bar);
//
//    for(int i=RANGE; i>=0; --i){
//        softPwmWrite(LED1, i);
//        softPwmWrite(LED2, i);
//        softPwmWrite(LED3, i);
//        softPwmWrite(LED4, i);
//        usleep(100);
//    }
//
//    return EXIT_SUCCESS;
//}
//
//void *fade_in(){
//
//    pthread_barrier_wait(&bar);
//
//    for(int i=0; i<=RANGE; i++){
//        softPwmWrite(LED1, i);
//        softPwmWrite(LED2, i);
//        softPwmWrite(LED3, i);
//        softPwmWrite(LED4, i);
//        usleep(100);
//    }
//
//    return EXIT_SUCCESS;
//}


int main( int argc, char *argv[] ) {

//    wiringPiSetup();
//
//    pinMode(LED1, OUTPUT);
//    pinMode(LED2, OUTPUT);
//    pinMode(LED3, OUTPUT);
//    pinMode(LED4, OUTPUT);
//
//    softPwmCreate(LED1, 0, 100);
//    softPwmCreate(LED2, 0, 100);
//    softPwmCreate(LED3, 0, 100);
//    softPwmCreate(LED4, 0, 100);

    struct Queue* q = malloc(sizeof *q);
    initQueue(q);
    printf("q size: %d\n",queueSize(q));

    addToQueue(q, 5, 10);
    printf("q size: %d\n",queueSize(q));
    addToQueue(q, 5, 10);
    printf("q size: %d\n",queueSize(q));
    addToQueue(q, 5, 10);
    addToQueue(q, 5, 10);
    addToQueue(q, 5, 10);
    printf("q size: %d\n",queueSize(q));

    printf("stop\n");

    //


//    pthread_t fade_out_thread;
//    pthread_t fade_in_thread;
//    int exit_code;
//
//    pthread_barrier_init(&bar, NULL, 3);
//
//    exit_code = pthread_create(&fade_out_thread, NULL, fade_out, NULL);
//    if(exit_code > 0) {
//        printf("Can't create thread\n");
//        return EXIT_FAILURE;
//    }
//    else {
//        printf("Created new fade_out_thread\n");
//    }
//
//    exit_code = pthread_create(&fade_in_thread, NULL, fade_in, NULL);
//    if(exit_code > 0) {
//        printf("Can't create thread\n");
//        return EXIT_FAILURE;
//    }
//    else {
//        printf("Created new fade_in_thread\n");
//    }
//
//
//    pthread_barrier_wait(&bar);
//
//    // joining threads
//    exit_code = pthread_join(fade_out_thread, NULL);
//    if(exit_code > 0) {
//        printf("Can't join thread");
//        return EXIT_FAILURE;
//    }
//    printf("fade_out_thread ended\n");
//
//    exit_code = pthread_join(fade_in_thread, NULL);
//    if(exit_code > 0) {
//        printf("Can't join thread");
//        return EXIT_FAILURE;
//    }
//    printf("fade_in_thread ended\n");


    return EXIT_SUCCESS;
}

