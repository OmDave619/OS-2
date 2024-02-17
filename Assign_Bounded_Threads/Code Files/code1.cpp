#include<bits/stdc++.h>
#include<pthread.h>
#include<sched.h>
#include<sys/time.h>
using namespace std;

int n, k;   //size of matrix(n) and number of threads(k)
int c, bt; //Number of logical cores(c) and number of bounded threads(bt)
int b; //bounded threads per core (k/c)
vector<vector<int>> A; //matrix A
int chunk;  //size of chunks (n/k) (number of rows computed by each thread)
vector<vector<int>> prod_chunk; //product matrix A*A (using chunk method)

typedef struct ComputeArgs {    //struct for thread arguments
    int thread_number;
    int start;  //starting row for each thread
} ComputeArgs;

// Thread function to compute square of matrix using chunk method
void* Compute_chunk(void* arg) {
    ComputeArgs* args = (ComputeArgs*)arg;
    int thread_number = args->thread_number;
    int start = args->start;
    int end = min(start + chunk, n);

    //Set affinity of thread if it is to be bounded
    if (thread_number < bt) {

        cpu_set_t cpuset;
        CPU_ZERO(&cpuset);

        int cpu_id = thread_number / b;

        CPU_SET(cpu_id, &cpuset);
        bool unsuccessfull = pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset);
        if (!unsuccessfull) {
            cout << "Thread " << thread_number << " set to CPU " << cpu_id << "" << endl;
        }
        else {
            cout << "Thread " << thread_number << " failed to set to CPU " << cpu_id << "" << endl;
        }
    }

    // last thread computes remaining rows
    if (thread_number == k) end = n;

    for (int i = start; i < end; i++) {
        //computing row i of product matrix
        for (int j = 0; j < n; j++) {
            for (int k = 0; k < n; k++) {
                prod_chunk[i][j] += A[i][k] * A[k][j];
            }
        }
    }

    free(args);
    pthread_exit(NULL);
}


void print_matrix(vector<vector<int>>& matrix, FILE* out) {
    cout << endl;
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            fprintf(out, "%d ", matrix[i][j]);
            // cout << matrix[i][j] << " ";
        }
        fprintf(out, "\n");
        // cout << endl;
    }
}

//creates and joins threads also prints the contents to output files, and returns time taken to execute
double chunk_method(vector<pthread_t>& threads, FILE* output) {

    fprintf(output, "\n\nChunk method:\n");

    struct timespec start_time, end_time;
    clock_gettime(CLOCK_MONOTONIC, &start_time);

    chunk = max(n / k, 2);  //using minimum chunk size as 2 other wise method would reduce to mixed

    //creating k threads and passing the starting indices to each thread 
    for (int i = 0; i < k; i++) {
        ComputeArgs* args = (ComputeArgs*)malloc(sizeof(ComputeArgs));
        args->thread_number = i + 1;
        args->start = i * chunk;
        pthread_create(&threads[i], NULL, Compute_chunk, (void*)args);
    }

    //joining all threads
    for (int i = 0; i < k; i++) {
        pthread_join(threads[i], NULL);
    }


    clock_gettime(CLOCK_MONOTONIC, &end_time);

    // // printing the product matrix
    fprintf(output, "Product matrix(Chunks):\n");
    // print_matrix(prod_chunk, output);

    double chunk_time = (end_time.tv_sec - start_time.tv_sec) + (1e-9) * (end_time.tv_nsec - start_time.tv_nsec);
    fprintf(output, "\nTotal time taken in chunk method: %f seconds\n", (chunk_time));
    // cout << "Total time taken in chunk method: " << chunk_time << " seconds" << endl;
    return chunk_time;
}

int main() {

    //reading input file
    FILE* input = fopen("../Input Files/input.txt", "r");
    if (input == NULL) {
        cout << "Input file not found" << endl;
        return 1;
    }
    fscanf(input, "%d %d %d %d", &n, &k, &c, &bt);

    b = k / c + (k % c != 0);  //bounded threads per core

    A.resize(n, vector<int>(n, 0));
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            fscanf(input, "%d", &A[i][j]);
        }
    }
    fclose(input);

    FILE* output = fopen("../Output Files/output1.txt", "w");
    if (output == NULL) {
        cout << "Output file not found" << endl;
        return 1;
    }
    fprintf(output, "Size of matrix: %d \nNumber of threads: %d", n, k);

    //resizing matrix according to sizes
    A.resize(n, vector<int>(n, 0));
    prod_chunk.resize(n, vector<int>(n, 0));

    //creating k threads
    vector<pthread_t> threads(k);

    int num_rep = 5; //used while plotting to take average time (taking 5 repititons) 

    // // chunk method
    double chunk_time = 0;
    for (int i = 0; i < num_rep; i++) {
        chunk_time += chunk_method(threads, output);
    }
    chunk_time /= num_rep;

    // time taken 
    // fprintf(output, "\nTotal time taken in chunk method: %f seconds\n", (chunk_time));
    cout << "Total time taken in chunk method: " << chunk_time << " seconds" << endl;
}