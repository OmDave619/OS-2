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


double totalWaitTimeWriters = 0.0;
double totalWaitTimeReaders = 0.0;
int totalWriterAttempts = 0;
int totalReaderAttempts = 0;
double worstWaitTimeWriters = 0.0;
double worstWaitTimeReaders = 0.0;

sem_t printing_mtx; //semaphore for mutual exclusion for printing
int readers = 0; // Number of active readers
int waitingWriters = 0; // Number of writers waiting to enter the critical section
sem_t mtx;  // Semaphore for mutual exclusion for the readers/waitingwriters count
sem_t wrt;  // Semaphore to ensure mutual exclusion in the critical section for writers
sem_t canRead; // Semaphore to control readers' access based on writer priority

void writer_enter() {
    sem_wait(&mtx); // Protect the waitingWriters count
    waitingWriters++;
    if (waitingWriters == 1)
        sem_wait(&canRead); // Block readers from starting if this is the first writer waiting
    sem_post(&mtx);
    sem_wait(&wrt); // Wait for access to the critical section
}

void writer_exit() {
    sem_post(&wrt); // Release the critical section for the next writer or reader
    sem_wait(&mtx); // Protect the waitingWriters count
    waitingWriters--;
    if (waitingWriters == 0)
        sem_post(&canRead); // Allow readers to start if no writers are waiting
    sem_post(&mtx);
}

void reader_enter() {
    sem_wait(&canRead); // Check if readers are allowed to read
    sem_wait(&mtx); // Mutual exclusion for the readers count
    readers++;
    if (readers == 1)
        sem_wait(&wrt); // If this is the first reader, wait for the critical section to be free
    sem_post(&mtx);
    sem_post(&canRead); // Allow other readers or writers to check their conditions
}

void reader_exit() {
    sem_wait(&mtx); // Mutual exclusion for the readers count
    readers--;
    if (readers == 0)
        sem_post(&wrt); // If this is the last reader, allow a writer to enter the critical section
    sem_post(&mtx);
}

typedef struct ComputeArgs {    //struct for thread arguments
    int thread_id;
} ComputeArgs;

void* writer(void* arg) {
    ComputeArgs* args = (ComputeArgs*)arg;
    int id = args->thread_id;

    for (int i = 1; i <= kw; i++) {
        
        sem_wait(&printing_mtx);
        double reqTime = clock() / (double)CLOCKS_PER_SEC;
        // cout << i << "th CS request by Writer Thread " << id << " at " << reqTime << endl;
        fprintf(output, "%dth CS request by Writer Thread %d at %f\n", i, id, reqTime);
        sem_post(&printing_mtx);

        writer_enter(); //writer_enter code

        //enters CS       
        sem_wait(&printing_mtx); 
        double enterTime = clock() / (double)CLOCKS_PER_SEC;

        totalWaitTimeWriters += (enterTime - reqTime);
        worstWaitTimeWriters = max(worstWaitTimeWriters, (enterTime - reqTime));
        totalWriterAttempts++;

        // cout << i << "th CS Entry by Writer Thread " << id << " at " << enterTime << endl;
        fprintf(output, "%dth CS Entry by Writer Thread %d at %f\n", i, id, enterTime);
        sem_post(&printing_mtx);

        double randCStime = distribution_cs(generator);
        usleep(randCStime * 1000); //simulate a thread writing in CS

        writer_exit(); //writer_exit code

        sem_wait(&printing_mtx);
        double exitTime = clock() / (double)CLOCKS_PER_SEC;
        // cout << i << "th CS Exit by Writer Thread " << id << " at " << exitTime << endl;
        fprintf(output, "%dth CS Exit by Writer Thread %d at %f\n", i, id, exitTime);
        sem_post(&printing_mtx);

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
        
        sem_wait(&printing_mtx);
        double reqTime = clock() / (double)CLOCKS_PER_SEC;
        // cout << i << "th CS request by Reader Thread " << id << " at " << reqTime << endl;
        fprintf(output, "%dth CS request by Reader Thread %d at %f\n", i, id, reqTime);
        sem_post(&printing_mtx);
        reader_enter(); //reader_enter code

        //enters CS
        sem_wait(&printing_mtx);
        double enterTime = clock() / (double)CLOCKS_PER_SEC;

        totalWaitTimeReaders += (enterTime - reqTime);
        worstWaitTimeReaders = max(worstWaitTimeReaders, (enterTime - reqTime));
        totalReaderAttempts++;

        // cout << i << "th CS Entry by Reader Thread " << id << " at " << enterTime << endl;
        fprintf(output, "%dth CS Entry by Reader Thread %d at %f\n", i, id, enterTime);
        sem_post(&printing_mtx);

        double randCStime = distribution_cs(generator); // in ms
        usleep(randCStime * 1000); // simulate a thread reading from CS


        reader_exit(); //reader_exit code

        sem_wait(&printing_mtx);
        double exitTime = clock() / (double)CLOCKS_PER_SEC;
        // cout << i << "th CS Exit by Reader Thread " << id << " at " << exitTime << endl;
        fprintf(output, "%dth CS Exit by Reader Thread %d at %f\n", i, id, exitTime);
        sem_post(&printing_mtx);

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
    output = fopen("../Output Files/RW-log.txt", "w");
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
    sem_init(&canRead, 0, 1); // Initialize Can read semaphore to 1
    sem_init(&printing_mtx, 0, 1); // Initialize printing semaphore to 1

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

    FILE *output2 = fopen("../Output Files/RW-Average_time.txt", "w");

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