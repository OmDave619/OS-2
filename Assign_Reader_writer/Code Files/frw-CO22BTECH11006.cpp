#include<bits/stdc++.h>
#include<pthread.h>
#include<ctime>
#include<iostream>   
#include<iomanip> 
#include<semaphore.h>
#include<cstdio>
#include<fstream>
#include<chrono>
using namespace std;

int nw;     //Number of writer threads
int nr;     //Number of reader threads
int kw;     //Number of times each writer thread tries to enter critical section
int kr;     //Number of times each reader thread tries to enter critical section
double muCS;   //avg time delays in critical section (exponential distribution)
double muRem;  //avg time delays in Remainder section (exponential distribution)

exponential_distribution<double> distribution_cs;   //exponential distribution for critical section delay
exponential_distribution<double> distribution_rem; //exponential distribution for remainder section delay

// Global random engine, seeded with current time
std::default_random_engine generator(std::chrono::system_clock::now().time_since_epoch().count());

// Global pointer to output file
FILE* output;

int readers = 0; //number of active readers 
sem_t mtx;  //semaphore for mutual exclusion for shared variables
sem_t wrt;   //semaphore for writer preference
sem_t rd;  //semaphore for reader 
sem_t printing_mtx; //semaphore for mutual exclusion for printing

double totalWaitTimeWriters = 0.0;
double totalWaitTimeReaders = 0.0;
int totalWriterAttempts = 0;
int totalReaderAttempts = 0;
double worstWaitTimeWriters = 0.0;
double worstWaitTimeReaders = 0.0;

void writer_enter() {
    sem_wait(&rd); //Prioritize writer over reader, make the reader wait 
    sem_wait(&wrt); //wait if there is a writer in CS
}

void writer_exit() {
    sem_post(&rd);  //signal reader if it is waiting
    sem_post(&wrt); //signal other writer if it is waiting
}

void reader_enter() {
    sem_wait(&rd); //To prevent mutual exclusion among readers
    sem_wait(&mtx); //to prevent mutual exclusion for readers shared variable
    readers++;
    if (readers == 1) {
        sem_wait(&wrt); //this ensures no writer can enter CS while even one reader is active
    }
    sem_post(&mtx); //signal other readers, as they can enter while this reader is active in CS (Multiple readers criteria)
    sem_post(&rd);
}

void reader_exit() {
    sem_wait(&mtx); // to prevent mutual exclusion for readers shared variable 
    readers--;  //reader wants to leave so decrement the number of active readers
    if (readers == 0) {
        sem_post(&wrt); //If no reader is left in CS, the waiting writer can enter, hence signal it
    }
    sem_post(&mtx); //readers leaves
}

typedef struct ComputeArgs {    //struct for thread arguments
    int thread_id;
} ComputeArgs;

void* writer(void* arg) {
    ComputeArgs* args = (ComputeArgs*)arg;
    int id = args->thread_id;

    for (int i = 1; i <= kw; i++) {

        double reqTime = clock() / (double)CLOCKS_PER_SEC;

        // cout << i << "th CS request by Writer Thread " << id << " at " << reqTime << endl;
        fprintf(output, "%dth CS request by Writer Thread %d at %f\n", i, id, reqTime);

        writer_enter(); //writer_enter code

        //enters CS        
        double enterTime = clock() / (double)CLOCKS_PER_SEC;

        totalWaitTimeWriters += (enterTime - reqTime);
        worstWaitTimeWriters = max(worstWaitTimeWriters, (enterTime - reqTime));
        totalWriterAttempts++;

        // cout << i << "th CS Entry by Writer Thread " << id << " at " << enterTime << endl;
        fprintf(output, "%dth CS Entry by Writer Thread %d at %f\n", i, id, enterTime);

        double randCStime = distribution_cs(generator);
        usleep(randCStime * 1000); //simulate a thread writing in CS

        writer_exit(); //writer_exit code

        double exitTime = clock() / (double)CLOCKS_PER_SEC;
        // cout << i << "th CS Exit by Writer Thread " << id << " at " << exitTime << endl;
        fprintf(output, "%dth CS Exit by Writer Thread %d at %f\n", i, id, exitTime);

        double randRemtime = distribution_rem(generator);
        usleep(randRemtime * 1000); //simulate a thread executing in Remainder Section
    }

    free(args);
    pthread_exit(NULL);
}

void* reader(void* arg) {
    ComputeArgs* args = (ComputeArgs*)arg;
    int id = args->thread_id;

    for (int i = 1; i <= kr; i++) {

        double reqTime = clock() / (double)CLOCKS_PER_SEC;

        // cout << i << "th CS request by Reader Thread " << id << " at " << reqTime << endl;
        fprintf(output, "%dth CS request by Reader Thread %d at %f\n", i, id, reqTime);

        reader_enter(); //reader_enter code

        //enters CS
        double enterTime = clock() / (double)CLOCKS_PER_SEC;

        totalWaitTimeReaders += (enterTime - reqTime);
        worstWaitTimeReaders = max(worstWaitTimeReaders, (enterTime - reqTime));
        totalReaderAttempts++;

        // cout << i << "th CS Entry by Reader Thread " << id << " at " << enterTime << endl;
        fprintf(output, "%dth CS Entry by Reader Thread %d at %f\n", i, id, enterTime);

        double randCStime = distribution_cs(generator);
        usleep(randCStime * 1000); // simulate a thread reading from CS

        reader_exit(); //reader_exit code

        double exitTime = clock() / (double)CLOCKS_PER_SEC;

        // cout << i << "th CS Exit by Reader Thread " << id << " at " << exitTime << endl;
        fprintf(output, "%dth CS Exit by Reader Thread %d at %f\n", i, id, exitTime);

        double randRemtime = distribution_rem(generator);
        // cout << "Remainder Section Time: " << randRemtime << endl;
        usleep(randRemtime * 1000); //simulate a thread executing in Remainder Section
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
    fscanf(input, "%d %d %d %d %lf %lf", &nw, &nr, &kw, &kr, &muCS, &muRem);
    fclose(input);

    // cout << "Number of writer threads: " << nw << endl;
    // cout << "Number of reader threads: " << nr << endl;
    // cout << "Number of times each writer thread tries to enter critical section: " << kw << endl;
    // cout << "Number of times each reader thread tries to enter critical section: " << kr << endl;

    //output file
    output = fopen("../Output Files/FairRW-log.txt", "w");
    if (output == NULL) {
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
    sem_init(&rd, 0, 1); // Initialize read semaphore to 1

    distribution_cs = exponential_distribution<double>(1.0 / muCS);
    distribution_rem = exponential_distribution<double>(1.0 / muRem);

    //creating writer threads
    for (int i = 0; i < nw; i++) {
        ComputeArgs* args = (ComputeArgs*)malloc(sizeof(ComputeArgs));
        args->thread_id = i + 1;
        pthread_create(&writerThreads[i], NULL, writer, (void*)args);
    }

    //creating reader threads
    for (int i = 0; i < nr; i++) {
        ComputeArgs* args = (ComputeArgs*)malloc(sizeof(ComputeArgs));
        args->thread_id = i + 1;
        pthread_create(&readerThreads[i], NULL, reader, (void*)args);
    }
    


    //joining threads
    for (int i = 0; i < nr; i++) {
        pthread_join(readerThreads[i], NULL);
    }
    for (int i = 0; i < nw; i++) {
        pthread_join(writerThreads[i], NULL);
    }

    fclose(output);

    FILE* output2 = fopen("../Output Files/FairRW-Average_time.txt", "w");

    double avgWaitTimeWriter = totalWriterAttempts > 0 ? totalWaitTimeWriters / totalWriterAttempts : 0;
    double avgWaitTimeReader = totalReaderAttempts > 0 ? totalWaitTimeReaders / totalReaderAttempts : 0;

    // cout << "Average time/writer threads: " << avgWaitTimeWriter << " seconds." << endl;
    fprintf(output2, "Average time/writer threads: %f seconds.\n", avgWaitTimeWriter);
    // cout << "Number of writer attempts: " << totalWriterAttempts << endl;

    // cout << "Average time/reader threads: " << avgWaitTimeReader << " seconds." << endl;
    fprintf(output2, "Average time/reader threads: %f seconds.\n", avgWaitTimeReader);
    // cout << "Number of reader attempts: " << totalReaderAttempts << endl;

    // cout << "Worst case/writer threads: " << worstWaitTimeWriters << " seconds." << endl;
    fprintf(output2, "Worst case/writer threads: %f seconds.\n", worstWaitTimeWriters);
    // cout << "Worst case/reader threads: " << worstWaitTimeReaders << " seconds." << endl;
    fprintf(output2, "Worst case/reader threads: %f seconds.\n", worstWaitTimeReaders);
    
    fclose(output2);
    return 0;
}