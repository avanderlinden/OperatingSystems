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
#include <wiringPi.h>


// Number of elements to add to the queue
#define ELEMENTS 100000

pthread_barrier_t bar;

struct Queue* qArray[4];

#define LED1 7
#define LED2 0
#define LED3 2
#define LED4 3

const int LedArray[4] = {LED1, LED2, LED3, LED4};

/* Struct Node
 * brightness: brightness variable
 * duration: duration variable
 * next: pointer to the next node in the queue.
 */
struct Node {
    int brightness;
    int duration; //Stored values
    struct Node* next;
};

/* Struct Queue
 * front: first node in the queue
 * back: last node in the queue
 * mutex: the mutex that should be locked when
 * accessing the queue.
 *
 */
struct Queue {
    int led;
    struct Node* front;
    struct Node* back;
    pthread_mutex_t mutex;
};


/* addToQueue adds a new node to the back of the queue.
 * The node is initialized with given parameters
 * brightness and duration.
 *
 * @param queue The pointer to the queue where the node
 * should be added to.
 * @param brightness The brightness variable to assign to the node.
 * @param duration The duration variable to assign to the node.
 */
void addToQueue(struct Queue* queue, int brightness, int duration) {
    struct Node *node = malloc(sizeof *node);
    node->brightness = brightness;
    node->duration = duration;
    node->next = NULL;

    pthread_mutex_lock(&(queue->mutex));
    if(queue->front == NULL){
        queue->front = node;
        queue->back = node;
    }
    else {
        queue->back->next = node;
        queue->back = node;
    }
    pthread_mutex_unlock(&(queue->mutex));

}

/* removeFromQueue removes node from front of the queue. values of
 * the node are returned via the pointers
 *
 * @param queue The pointer to the queue where the node
 * should be removed from.
 * @param *pBrightness The brightness pointer where the brightness value will
 * be stored in.
 * @param *pDuration The duration pointer where the duration value will
 * be stored in.
 */
void removeFromQueue(struct Queue* queue, int* pBrightness, int* pDuration) {
    pthread_mutex_lock(&(queue->mutex));

    struct Node* node = queue->front;
    if(node == NULL){
        *pBrightness = -1;
        *pDuration = -1;
    }
    else {

        queue->front = node->next;

        // for last node
        if(queue->front == NULL){
            queue->back = NULL;
        }

        *pBrightness = node->brightness;
        *pDuration = node->duration;

        // give back some of that mem
        free(node);
    }
    pthread_mutex_unlock(&(queue->mutex));
}

/* queueSize returns the length of the queue
 *
 * @param queue the pointer to the queue where the size should be
 * calculated from.
 *
 */
int queueSize(struct Queue* queue) {
    int size = 0;
    struct Node* node;
    pthread_mutex_lock(&(queue->mutex));

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

    pthread_mutex_unlock(&(queue->mutex));
    return size;

}

/* initQueue initializes the queue struct and creates a pthread_mutex.
 *
 * @param queue The queue that should be initialized.
 */
void initQueue(struct Queue* queue) {
    queue->back = NULL;
    queue->front = NULL;

    pthread_mutex_t mutex;

    if (pthread_mutex_init(&mutex, NULL) != 0)
    {
        printf("\nError: mutex init failed\n");
    }

    queue->mutex = mutex;
}

void initShow(){
    //FILE *fp;
    char *line = NULL;
    size_t len = 0;
    //ssize_t read;
    int led, brightness, duration;

//    fp = fopen("./lightshow.txt", "r");
//    if (fp == NULL) {
//        printf("Error reading file\n");
//        return;
//    }
//
//    while ((read = getline(&line, &len, fp)) != -1) {
//        sscanf(line, "%d %d %d", &led, &brightness, &duration);
//        addToQueue(qArray[led], brightness, duration);
//    }


    while (getline(&line, &len, stdin) != -1){
        sscanf(line, "%d %d %d", &led, &brightness, &duration);
        addToQueue(qArray[led], brightness, duration);

        printf("%s", line);
    }
}
void *led_thread(void* queue){
    struct Queue *q = (struct Queue *) queue;

    int *pBrightness, *pDuration;

    while(1){
        while(*pBrightness != -1 && *pDuration != -1){
            removeFromQueue(q, pBrightness, pDuration);
            softPwmWrite(LedArray[q->led], pBrightness);
            usleep(pDuration);
        }
        usleep(100);
    }

}


void *input_thread(){
    char *line = NULL;
    //size_t len = 0;
    //int led, brightness, duration;

    char l[1024];

    while(1){
        fgets(l, 1024, stdin);
        printf("l: %s\n", l);
    }
}

/*
 * Main
 */
int main( int argc, char *argv[] ) {
    printf("start\n");
    int status;

    //struct Queue* q = malloc(sizeof *q);
     // = (struct Queue*) malloc(sizeof(struct Queue));

    for(int i=0; i<4; i++){
        struct Queue* q = malloc(sizeof *q);
        initQueue(q);
        qArray[i] = q;
    }

//    for(int i=0; i<4; i++){
//        printf("%p\n", &(qArray[i]->mutex));
//    }


    initShow();





    //pthread_t input_thread_ptr;
   // pthread_create(&input_thread_ptr, NULL, input_thread, NULL);
    //pthread_join(input_thread_ptr, NULL);



//    for(int i=0; i<4; i++){
//
//    }


//
//    pthread_t add_thread_1;
//    pthread_t add_thread_2;
//    pthread_t rm_thread_1;
//    pthread_t rm_thread_2;
//
//    pthread_barrier_init(&bar, NULL, 3);
//
//    // add elements threads
//    pthread_create(&add_thread_1, NULL, add_elements, q);
//    pthread_create(&add_thread_2, NULL, add_elements, q);
//
//    pthread_barrier_wait(&bar);
//
//    pthread_join(add_thread_1, NULL);
//    pthread_join(add_thread_2, NULL);
//
//    printf("size: %d\n", queueSize(q));
//
//    // remove all elements threads
//    pthread_create(&rm_thread_1, NULL, remove_elements, q);
//    pthread_create(&rm_thread_2, NULL, remove_elements, q);
//
//    pthread_barrier_wait(&bar);
//
//    pthread_join(rm_thread_1, NULL);
//    pthread_join(rm_thread_2, NULL);
//
//
//    printf("size: %d\n", queueSize(q));

    return EXIT_SUCCESS;
}

