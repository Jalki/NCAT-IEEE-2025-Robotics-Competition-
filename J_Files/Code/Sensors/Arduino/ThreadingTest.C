#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#define NUM_THREADS 4

void* perform_work(void* arg)
{
    int index = *((int*)arg);
    printf("Thread %d: starting work.... \n", index);
    //perform some work here
    printf("Thread %d: work done. \n", index);
    //Return NULL to indeicate the thread has finished execution
    return NULL;
}

int main()
{
    pthread_t threads[NUM_THREADS];
    int thread_args[NUM_THREADS];
    int rc;

    for (int i = 0; i < NUM_THREADS; ++i){
        thread_args[i] = i;
        rc = pthread_create(&threads[i], NULL, perform_work,(void*)&thread_args[i]);

        if (rc) {
            printf("Error: unable to create thread, %d\n", rc);
            exit(-1);
        }
    }

    for (int i =0; i < NUM_THREADS; ++i){
        pthread_join(threads[i], NULL);
    }
}