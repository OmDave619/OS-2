#include<bits/stdc++.h>
#include<pthread.h>
#include<sys/time.h>
using namespace std;

int n, k;   //size of matrix(n) and number of threads(k)
vector<vector<int>> A; //matrix A
int chunk;  //size of chunks (n/k) (number of rows computed by each thread)
vector<vector<int>> prod_chunk; //product matrix A*A (using chunk method)
vector<vector<int>> prod_mixed; //product matrix A*A (using mixed method)
vector<long long> exec_time_chunks; //execution time of each thread (using chunk method)
vector<long long> exec_time_mixed;  //execution time of each thread (using mixed method)


//For mixed-chunk method
vector<vector<int>> prod_mixed_chunks;
vector<long long> exec_time_mixed_chunks;
int chunk_for_mixed_chunks;

typedef struct ComputeArgs {    //struct for thread arguments
    int thread_id;
    int start;  //starting row for each thread
} ComputeArgs;

// Thread function to compute square of matrix using chunk method
void* Compute_chunk(void* arg) {
    ComputeArgs* args = (ComputeArgs*)arg;
    int thread_id = args->thread_id;
    int start = args->start;
    int end = min(start + chunk, n);

    // last thread computes remaining rows
    if (thread_id == k) end = n;

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
    double time_taken = (end_time.tv_sec - start_time.tv_sec) * 1e9 + (end_time.tv_nsec - start_time.tv_nsec);
    exec_time_chunks[thread_id] = time_taken;

    free(args);
    pthread_exit(NULL);
}

// Thread function to compute square of matrix using mixed method
void* Compute_mixed(void* arg) {
    ComputeArgs* args = (ComputeArgs*)arg;
    int thread_id = args->thread_id;
    int start = args->start;
    int end = n;

    struct timespec start_time, end_time;
    clock_gettime(CLOCK_MONOTONIC, &start_time);

    for (int i = start; i < end; i += k) {
        //computing row i of product matrix
        for (int j = 0; j < n; j++) {
            for (int k = 0; k < n; k++) {
                prod_mixed[i][j] += A[i][k] * A[k][j];
            }
        }
    }

    clock_gettime(CLOCK_MONOTONIC, &end_time);
    double time_taken = (end_time.tv_sec - start_time.tv_sec) * 1e9 + (end_time.tv_nsec - start_time.tv_nsec);
    exec_time_mixed[thread_id] = time_taken;

    free(args);
    pthread_exit(NULL);
}

// Thread function to compute square of matrix using mixed-chunks method
void* Compute_mixed_chunks(void* arg) {
    ComputeArgs* args = (ComputeArgs*)arg;
    int thread_id = args->thread_id;
    int start = args->start;
    int end = n;

    struct timespec start_time, end_time;
    clock_gettime(CLOCK_MONOTONIC, &start_time);

    for (int p = start; p < end; p += k * chunk_for_mixed_chunks) {
        for (int i = p; i < min(p + chunk_for_mixed_chunks, n); i++) {
            for (int j = 0; j < n; j++) {
                for (int k = 0; k < n; k++) {
                    prod_mixed_chunks[i][j] += A[i][k] * A[k][j];
                }
            }
        }
    }

    clock_gettime(CLOCK_MONOTONIC, &end_time);
    double time_taken = (end_time.tv_sec - start_time.tv_sec) * 1e9 + (end_time.tv_nsec - start_time.tv_nsec);
    exec_time_mixed_chunks[thread_id] = time_taken;

    free(args);
    pthread_exit(NULL);
}

void print_matrix(vector<vector<int>>& matrix, FILE* out) {
    cout << endl;
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            fprintf(out, "%d ", matrix[i][j]);
            cout << matrix[i][j] << " ";
        }
        fprintf(out, "\n");
        cout << endl;
    }
}

long long chunk_method(vector<pthread_t>& threads, FILE* output) {
    
    fprintf(output, "\n\nChunk method:\n");

    struct timespec start_time, end_time;
    clock_gettime(CLOCK_MONOTONIC, &start_time);

    chunk = max(n / k, 2);

    for (int i = 0; i < k; i++) {
        ComputeArgs* args = (ComputeArgs*)malloc(sizeof(ComputeArgs));
        args->thread_id = i + 1;
        args->start = i * chunk;
        pthread_create(&threads[i], NULL, Compute_chunk, (void*)args);
    }

    for (int i = 0; i < k; i++) {
        pthread_join(threads[i], NULL);
    }


    clock_gettime(CLOCK_MONOTONIC, &end_time);

    // // printing the product matrix
    fprintf(output, "Product matrix(Chunks):\n");
    print_matrix(prod_chunk, output);

    long long chunk_time = (end_time.tv_sec - start_time.tv_sec) * 1e9 + (end_time.tv_nsec - start_time.tv_nsec);
    fprintf(output, "\nTotal time taken in chunk method: %lld nanoseconds\n", (chunk_time));

    for (int i = 0; i < k; i++) {
        fprintf(output, "Time taken by thread %d: %lld nanoseconds\n", i + 1, (exec_time_chunks[i + 1]));
    }

    return chunk_time;
}

long long mixed_method(vector<pthread_t>& threads, FILE* output) {
    
    fprintf(output, "\n\nMixed method:\n\n");
    
    struct timespec start_time, end_time;
    clock_gettime(CLOCK_MONOTONIC, &start_time);

    for (int i = 0; i < k; i++) {
        ComputeArgs* args = (ComputeArgs*)malloc(sizeof(ComputeArgs));
        args->thread_id = i + 1;
        args->start = i;
        pthread_create(&threads[i], NULL, Compute_mixed, (void*)args);
    }

    for (int i = 0; i < k; i++) {
        pthread_join(threads[i], NULL);
    }

    clock_gettime(CLOCK_MONOTONIC, &end_time);

    // //printing the product matrix
    fprintf(output, "Product matrix(Mixed):\n");
    print_matrix(prod_mixed, output);

    long long mixed_time = (end_time.tv_sec - start_time.tv_sec) * 1e9 + (end_time.tv_nsec - start_time.tv_nsec);
    fprintf(output, "\nTotal time taken in mixed method: %lld nanoseconds\n", (mixed_time));

    for (int i = 0; i < k; i++) {
        fprintf(output, "Time taken by thread %d: %lld nanoseconds\n", i + 1, (exec_time_mixed[i + 1]));
    }
    
    return mixed_time;
}

long long mixed_chunks_method(vector<pthread_t>& threads, FILE* output) {

    fprintf(output, "\n\nMixed-Chunks method:\n\n");
    
    struct timespec start_time, end_time;
    clock_gettime(CLOCK_MONOTONIC, &start_time);

    //determine size of chunk in mixed-chunks method
    chunk_for_mixed_chunks = max(((n / k) / k) * 2, 2);
    fprintf(output, "\n\nSize of block/chunk in mixed-chunk method: %d\n\n", chunk_for_mixed_chunks);

    for (int i = 0; i < k; i++) {
        ComputeArgs* args = (ComputeArgs*)malloc(sizeof(ComputeArgs));
        args->thread_id = i + 1;
        args->start = i * chunk_for_mixed_chunks;
        pthread_create(&threads[i], NULL, Compute_mixed_chunks, (void*)args);
    }

    for (int i = 0; i < k; i++) {
        pthread_join(threads[i], NULL);
    }

    clock_gettime(CLOCK_MONOTONIC, &end_time);

    // //printing the product matrix
    fprintf(output, "Product matrix(Mixed-chunks):\n");
    print_matrix(prod_mixed_chunks, output);

    long long mixed_chunks_time = (end_time.tv_sec - start_time.tv_sec) * 1e9 + (end_time.tv_nsec - start_time.tv_nsec);
    fprintf(output, "\nTotal time taken in mixed-chunks method: %lld nanoseconds\n", (mixed_chunks_time));

    for (int i = 0; i < k; i++) {
        fprintf(output, "Time taken by thread %d: %lld nanoseconds\n", i + 1, (exec_time_mixed_chunks[i + 1]));
    }

    return mixed_chunks_time;
}

int main() {

    //reading input file
    FILE* input = fopen("../Input Files/input.txt", "r");
    if (input == NULL) {
        cout << "Input file not found" << endl;
        return 1;
    }
    fscanf(input, "%d %d", &n, &k);
    A.resize(n, vector<int>(n, 0));
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            fscanf(input, "%d", &A[i][j]);
        }
    }
    fclose(input);

    FILE* output = fopen("../Output Files/output.txt", "w");
    if (output == NULL) {
        cout << "Output file not found" << endl;
        return 1;
    }
    fprintf(output, "Size of matrix: %d \nNumber of threads: %d", n, k);

    //resizing matrix according to sizes
    A.resize(n, vector<int>(n, 0));
    prod_chunk.resize(n, vector<int>(n, 0));
    prod_mixed.resize(n, vector<int>(n, 0));
    prod_mixed_chunks.resize(n, vector<int>(n, 0));
    exec_time_mixed_chunks.resize(k + 1, 0);
    exec_time_chunks.resize(k + 1, 0);
    exec_time_mixed.resize(k + 1, 0);


    //creating k threads
    vector<pthread_t> threads(k);

    int num_rep=1; //used while plotting to take average time (taking 5 repititons) 

    // // chunk method
    long long chunk_time=0;
    for(int i = 0; i < num_rep; i++) {
        chunk_time += chunk_method(threads,output);
    }
    chunk_time/=num_rep;

    // // mixed method
    long long mixed_time=0;
    for(int i = 0; i < num_rep; i++) {
        mixed_time += mixed_method(threads,output);
    }
    mixed_time/=num_rep;
    
    //mixed-chunks method
    long long mixed_chunks_time=0;
    for(int i = 0; i < num_rep; i++) {
        mixed_chunks_time += mixed_chunks_method(threads,output);
    }
    mixed_chunks_time/=num_rep;

    //time taken in all methods
    fprintf(output, "\nTotal time taken in chunk method: %lld microseconds\n", (chunk_time) / 1000);
    fprintf(output, "Total time taken in mixed method: %lld microseconds\n", (mixed_time) / 1000);
    fprintf(output, "Total time taken in mixed-chunks method: %lld microseconds\n", (mixed_chunks_time) / 1000);
}