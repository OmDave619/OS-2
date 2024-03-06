#include<bits/stdc++.h>
#include<pthread.h>
#include<semaphore.h>
#include <chrono>
using namespace std;

int nw;     //Number of writer threads
int nr;     //Number of reader threads
int kw;     //Number of times each writer thread tries to enter critical section
int kr;     //Number of times each reader thread tries to enter critical section

// Global pointer to output file
FILE *output;

sem_t mtx , wrt ;   //semaphores for mutual exclusion and writer preference
int readers = 0; //number of active readers 

sem_t printing_lock; //semaphore for printing

void writer_enter() {
    sem_wait(&wrt); //wait if there is a writer in CS
}

void writer_exit() {
    sem_post(&wrt); //signal other writer if it is waiting
}

void reader_enter() {
    sem_wait(&mtx); //to prevent mutual exclusion
    readers++;
    if(readers == 1) {  
        sem_wait(&wrt); //this ensures no writer can enter CS while even one reader is active
    }
    sem_post(&mtx); //signal other readers, as they can enter while this reader is active in CS (Multiple readers criteria)
}

void reader_exit() {
    sem_wait(&mtx); // to prevent mutual exclusion 
    readers--;  //reader wants to leave so decrement the number of active readers
    if(readers == 0) {  
        sem_post(&wrt); //If no reader is left in CS, the waiting writer can enter, hence signal it
    }
    sem_post(&mtx); //readers leaves
}

typedef struct ComputeArgs {    //struct for thread arguments
    int thread_id;
} ComputeArgs;

void* writer(void *arg) {
    ComputeArgs* args = (ComputeArgs*)arg;
    int id = args->thread_id;

    for(int i = 1; i <= kw; i++) {
        
        double reqTime = clock() / (double)CLOCKS_PER_SEC ;
        
        // sem_wait(&printing_lock);
        cout << i << "th CS request by Writer Thread " << id << " at " << reqTime << endl ;
        fprintf(output, "%dth CS request by Writer Thread %d at %f\n", i, id, reqTime);
        // sem_post(&printing_lock);

        writer_enter(); //writer_enter code

        //enters CS
        double enterTime = clock() / (double)CLOCKS_PER_SEC;
        
        // sem_wait(&printing_lock);
        cout << i << "th CS Entry by Writer Thread " << id << " at " << enterTime << endl ;
        fprintf(output, "%dth CS Entry by Writer Thread %d at %f\n", i, id, enterTime);
        // sem_post(&printing_lock);
        
        double randCStime = (double)rand() / RAND_MAX;  //random time (in 0 to 1) to simulate reader/writer executing in Critical Section
        usleep(randCStime * 1000000); //simulate a thread writing in CS

        writer_exit(); //writer_exit code

        double exitTime = clock() / (double)CLOCKS_PER_SEC;

        // sem_wait(&printing_lock);
        cout << i << "th CS Exit by Writer Thread " << id << " at " << exitTime << endl ;
        fprintf(output, "%dth CS Exit by Writer Thread %d at %f\n", i, id, exitTime);
        // sem_post(&printing_lock);
        
        double randRemtime = (double)rand() / RAND_MAX;  //random time (in 0 to 1) to simulate reader/writer executing remainder section
        usleep(randRemtime * 1000000); //simulate a thread executing in Remainder Section
    }

    free(args);
    pthread_exit(NULL);
}

void* reader(void *arg) {
    ComputeArgs* args = (ComputeArgs*)arg;
    int id = args->thread_id;

    for(int i = 1; i <= kr; i++) {

        double reqTime = clock() / (double)CLOCKS_PER_SEC ;

        // sem_wait(&printing_lock);
        cout << i << "th CS request by Reader Thread " << id << " at " << reqTime << endl ;
        fprintf(output, "%dth CS request by Reader Thread %d at %f\n", i, id, reqTime);
        // sem_post(&printing_lock);

        reader_enter(); //reader_enter code

        //enters CS
        double enterTime = clock() / (double)CLOCKS_PER_SEC;

        // sem_wait(&printing_lock);
        cout << i << "th CS Entry by Reader Thread " << id << " at " << enterTime << endl ;
        fprintf(output, "%dth CS Entry by Reader Thread %d at %f\n", i, id, enterTime);
        // sem_post(&printing_lock);
        
        double randCStime = (double)rand() / RAND_MAX;  //random time (in 0 to 1) to simulate reader/writer executing in Critical Section
        usleep(randCStime * 1000000); // simulate a thread reading from CS

        reader_exit(); //reader_exit code

        double exitTime = clock() / (double)CLOCKS_PER_SEC;

        // sem_wait(&printing_lock);
        cout << i << "th CS Exit by Reader Thread " << id << " at " << exitTime << endl ;
        fprintf(output, "%dth CS Exit by Reader Thread %d at %f\n", i, id, exitTime);
        // sem_post(&printing_lock);
        
        double randRemtime = (double)rand() / RAND_MAX;  //random time (in 0 to 1) to simulate reader/writer executing remainder section
        usleep(randRemtime * 1000000); //simulate a thread executing in Remainder Section
    }

    free(args);
    pthread_exit(NULL);
}

int main() {

    //reading input file
    FILE* input = fopen("../Input Files/input.txt", "r");
    if (input == NULL) {
        cout << "Input file not found" << endl;
        return 1;
    }
    fscanf(input, "%d %d %d %d", &nw, &nr, &kw, &kr);
    fclose(input);

    cout << "Number of writer threads: " << nw << endl;
    cout << "Number of reader threads: " << nr << endl;
    cout << "Number of times each writer thread tries to enter critical section: " << kw << endl;
    cout << "Number of times each reader thread tries to enter critical section: " << kr << endl;

    //output file
    output = fopen("../Output Files/output.txt", "w");
    if(output == NULL) {
        cout << "Output file not found" << endl;
        return 1;
    }
    fprintf(output, "Number of writer threads: %d\n", nw);
    fprintf(output, "Number of reader threads: %d\n", nr);
    fprintf(output, "Number of times each writer thread tries to enter critical section: %d\n", kw);
    fprintf(output, "Number of times each reader thread tries to enter critical section: %d\n", kr);

    pthread_t writerThreads[nw];
    pthread_t readerThreads[nr];
    
    sem_init(&mtx, 0, 1);  // Initialize mutex semaphore to 1
    sem_init(&wrt, 0, 1);  // Initialize write semaphore to 1

    //initializing printing lock
    sem_init(&printing_lock, 0, 1);

    //creating writer threads
    for(int i = 0; i < nw; i++) {
        ComputeArgs* args = (ComputeArgs*)malloc(sizeof(ComputeArgs));
        args->thread_id = i+1;
        pthread_create(&writerThreads[i], NULL, writer, (void *)args);
    }

    //creating reader threads
    for(int i = 0; i < nr; i++) {
        ComputeArgs* args = (ComputeArgs*)malloc(sizeof(ComputeArgs));
        args->thread_id = i+1;
        pthread_create(&readerThreads[i], NULL, reader, (void *)args);
    }

    //joining threads
    for(int i = 0; i < nw; i++) {
        pthread_join(writerThreads[i], NULL);
    }

    for(int i = 0; i < nr; i++) {
        pthread_join(readerThreads[i], NULL);
    }

    fclose(output);
}