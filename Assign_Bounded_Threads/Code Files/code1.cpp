#include<bits/stdc++.h>
#include<pthread.h>
#include<sched.h>
#include<sys/time.h>
using namespace std;

int n, k;   //size of matrix(n) and number of threads(k)
int c, bt; //Number of logical cores(c) and number of bounded threads(bt)
int b; //bounded threads per core (k/c)
int bc; //bounded cores (number of cores which are assigned to bounded threads)
vector<vector<int>> A; //matrix A
int chunk;  //size of chunks (n/k) (number of rows computed by each thread)
vector<vector<int>> prod_chunk; //product matrix A*A (using chunk method)
vector<double> exec_time_bound_threads; //execution time of each bounded thread 
vector<double> exec_time_unbound_threads; //execution time of each bounded thread 
vector<int> cpu_id_for_bounded_thread; //cpu id for each bounded thread

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

        int cpu_id = thread_number % bc;

        CPU_SET(cpu_id, &cpuset);
        bool unsuccessfull = pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset);
        if (!unsuccessfull) {
            cout << "Thread " << thread_number << " set to CPU " << cpu_id << "" << endl;
            cpu_id_for_bounded_thread[thread_number] = cpu_id;
        }
        else {
            cout << "Thread " << thread_number << " failed to set to CPU " << cpu_id << "" << endl;
        }
    }

    // last thread computes remaining rows
    if (thread_number == k) end = n;

    struct timespec start_time, end_time;
    clock_gettime(CLOCK_MONOTONIC, &start_time);

    for (int i = start; i < end; i++) {
        //computing row i of product matrix
        for (int j = 0; j < n; j++) {
            for (int k = 0; k < n; k++) {
                prod_chunk[i][j] += A[i][k] * A[k][j];
            }
        }
    }

    clock_gettime(CLOCK_MONOTONIC, &end_time);
    double time_taken = (end_time.tv_sec - start_time.tv_sec) + (1e-9) * (end_time.tv_nsec - start_time.tv_nsec);
    if (thread_number < bt) exec_time_bound_threads[thread_number] = time_taken;
    else exec_time_unbound_threads[thread_number - bt] = time_taken;

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
        args->thread_number = i;
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

    //printing execution time of each bounded thread
    fprintf(output, "\nExecution time of each bounded thread:\n");
    for (int i = 0; i < bt; i++) {
        fprintf(output, "Thread %d: %f seconds\n", i + 1, exec_time_bound_threads[i]);
    }

    //printing execution time of each unbounded thread
    fprintf(output, "\nExecution time of each unbounded thread:\n");
    for (int i = 0; i < k - bt; i++) {
        fprintf(output, "Thread %d: %f seconds\n", i + 1 + bt, exec_time_unbound_threads[i]);
    }

    //cpu id for each bounded thread
    fprintf(output, "\nCPU id for each bounded thread:\n");
    for (int i = 0; i < bt; i++) {
        fprintf(output, "Thread %d: %d\n", i + 1, cpu_id_for_bounded_thread[i]);
    }

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

    //Number of cores assigned to bounded threads (bounded cores)
    bc = c; //For experiment 1
    // bc = c / 2; //For experiment 2

    b = bt / bc + (bt % bc != 0);  //bounded threads per core
    cout << b << "\n";

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
    cout << "Size of matrix: " << n << "\nNumber of threads: " << k << endl;
    cout << "Bounded threads: " << bt << endl;
    cout << "Approx Bounded threads per core: " << b << "\n\n";

    //resizing matrix according to sizes
    A.resize(n, vector<int>(n, 0));
    prod_chunk.resize(n, vector<int>(n, 0));
    exec_time_bound_threads.resize(bt, 0);
    exec_time_unbound_threads.resize(k - bt, 0);
    cpu_id_for_bounded_thread.resize(bt, 0);

    //creating k threads
    vector<pthread_t> threads(k);

    int num_rep = 1; //used while plotting to take average time (taking 5 repititons) 

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