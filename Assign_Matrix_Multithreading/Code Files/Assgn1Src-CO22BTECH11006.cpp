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

typedef struct ComputeArgs {
    int thread_id;
    int start;  //starting row for each thread
} ComputeArgs;

// Computes square of matrix using chunk method
void* Compute_chunk(void* arg) {
    ComputeArgs* args = (ComputeArgs*)arg;
    int thread_id = args->thread_id;
    int start = args->start;
    int end = min(start + chunk,n);

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

// Computes square of matrix using mixed method
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


int main() {


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


    A.resize(n, vector<int>(n, 0));
    prod_chunk.resize(n, vector<int>(n, 0));
    prod_mixed.resize(n, vector<int>(n, 0));
    exec_time_chunks.resize(k + 1, 0);
    exec_time_mixed.resize(k + 1, 0);

    //creating k threads
    pthread_t threads[k];

    // chunk method
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

    //printing the product matrix
    fprintf(output, "Product matrix(Chunks):\n");
    // print_matrix(prod_chunk, output);

    long long chunk_time = (end_time.tv_sec - start_time.tv_sec) * 1e9 + (end_time.tv_nsec - start_time.tv_nsec);
    fprintf(output, "\nTotal time taken in chunk method: %lld nanoseconds\n", (chunk_time));

    for (int i = 0; i < k; i++) {
        fprintf(output, "Time taken by thread %d: %lld nanoseconds\n", i + 1, (exec_time_chunks[i + 1]));
    }

    // mixed method
    fprintf(output, "\n\nMixed method:\n\n");

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

    //printing the product matrix
    fprintf(output, "Product matrix(Mixed):\n");
    // print_matrix(prod_mixed, output);

    long long mixed_time = (end_time.tv_sec - start_time.tv_sec) * 1e9 + (end_time.tv_nsec - start_time.tv_nsec);
    fprintf(output, "\nTotal time taken in mixed method: %lld nanoseconds\n", (mixed_time));

    for (int i = 0; i < k; i++) {
        fprintf(output, "Time taken by thread %d: %lld nanoseconds\n", i + 1, (exec_time_mixed[i + 1]));
    }

}