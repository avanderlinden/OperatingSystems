#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <inttypes.h>
#include <limits.h>
#include <pthread.h>
#include <ctype.h>



// Number of elements to add to the queue
#define ELEMENTS 100000

pthread_barrier_t bar;


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




/* add_elements Thread function that add elements to a given queue.
 *
 * @param queue The queue where the elements should be added to.
 */
void *add_elements(void* queue){
    struct Queue *q = (struct Queue *) queue;

    pthread_barrier_wait(&bar);

    int i;
    for(i=0; i<ELEMENTS; i++){
        addToQueue(q, 1, 2);
    }

    return EXIT_SUCCESS;
}

/* remove_elements Thread function that removes all elements from
 * a given queue.
 *
 * @param queue The queue where the elements should be removed from.
 */
void *remove_elements(void* queue){
    struct Queue *q = (struct Queue *) queue;

    int *bright = malloc(sizeof *bright);
    int *time = malloc(sizeof *time);

    pthread_barrier_wait(&bar);

    int i;
    for(i=0; i<ELEMENTS; i++){
        removeFromQueue(q, bright, time);
        if(*bright == -1 || *time == -1){
            printf("Error: -1 is returned\n");
        }
    }

    return EXIT_SUCCESS;
}

/*
 * Main
 */
int main( int argc, char *argv[] ) {


    struct Queue* q = malloc(sizeof *q);
    initQueue(q);

    pthread_t add_thread_1;
    pthread_t add_thread_2;
    pthread_t rm_thread_1;
    pthread_t rm_thread_2;

    pthread_barrier_init(&bar, NULL, 3);

    // add elements threads
    pthread_create(&add_thread_1, NULL, add_elements, q);
    pthread_create(&add_thread_2, NULL, add_elements, q);

    pthread_barrier_wait(&bar);

    pthread_join(add_thread_1, NULL);
    pthread_join(add_thread_2, NULL);

    printf("size: %d\n", queueSize(q));

    // remove all elements threads
    pthread_create(&rm_thread_1, NULL, remove_elements, q);
    pthread_create(&rm_thread_2, NULL, remove_elements, q);

    pthread_barrier_wait(&bar);

    pthread_join(rm_thread_1, NULL);
    pthread_join(rm_thread_2, NULL);


    printf("size: %d\n", queueSize(q));

    return EXIT_SUCCESS;
}

