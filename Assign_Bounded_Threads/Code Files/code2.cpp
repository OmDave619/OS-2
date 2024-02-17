#include<bits/stdc++.h>
#include<pthread.h>
#include<sched.h>
#include<sys/time.h>
using namespace std;

int n, k;   //size of matrix(n) and number of threads(k)
int c, bt; //Number of logical cores(c) and number of bounded threads(bt)
int b; //bounded threads per core (k/c)
vector<vector<int>> A; //matrix A
vector<vector<int>> prod_mixed; //product matrix A*A (using mixed method)

typedef struct ComputeArgs {    //struct for thread arguments
    int thread_number;
    int start;  //starting row for each thread
} ComputeArgs;

// Thread function to compute square of matrix using mixed method
void* Compute_mixed(void* arg) {
    ComputeArgs* args = (ComputeArgs*)arg;
    int thread_number = args->thread_number;
    int start = args->start;
    int end = n;

    // Set affinity of thread if it is to be bounded
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


    for (int i = start; i < end; i += k) {
        //computing row i of product matrix
        for (int j = 0; j < n; j++) {
            for (int k = 0; k < n; k++) {
                prod_mixed[i][j] += A[i][k] * A[k][j];
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
            cout << matrix[i][j] << " ";
        }
        fprintf(out, "\n");
        cout << endl;
    }
}

double mixed_method(vector<pthread_t>& threads, FILE* output) {

    fprintf(output, "\n\nMixed method:\n\n");

    struct timespec start_time, end_time;
    clock_gettime(CLOCK_MONOTONIC, &start_time);

    for (int i = 0; i < k; i++) {
        ComputeArgs* args = (ComputeArgs*)malloc(sizeof(ComputeArgs));
        args->thread_number = i + 1;
        args->start = i;
        pthread_create(&threads[i], NULL, Compute_mixed, (void*)args);
    }

    for (int i = 0; i < k; i++) {
        pthread_join(threads[i], NULL);
    }

    clock_gettime(CLOCK_MONOTONIC, &end_time);

    // //printing the product matrix
    fprintf(output, "Product matrix(Mixed):\n");
    // print_matrix(prod_mixed, output);

    double mixed_time = (end_time.tv_sec - start_time.tv_sec) + (1e-9)*(end_time.tv_nsec - start_time.tv_nsec);
    fprintf(output, "\nTotal time taken in mixed method: %f nanoseconds\n", (mixed_time));

    return mixed_time;
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

    FILE* output = fopen("../Output Files/output2.txt", "w");
    if (output == NULL) {
        cout << "Output file not found" << endl;
        return 1;
    }
    fprintf(output, "Size of matrix: %d \nNumber of threads: %d", n, k);

    //resizing matrix according to sizes
    A.resize(n, vector<int>(n, 0));
    prod_mixed.resize(n, vector<int>(n, 0));

    //creating k threads
    vector<pthread_t> threads(k);
    
    int num_rep = 5; //used while plotting to take average time (taking 5 repititons) 

    // // mixed method
    double mixed_time = 0;
    for (int i = 0; i < num_rep; i++) {
        mixed_time += mixed_method(threads, output);
    }
    mixed_time /= num_rep;


    //time taken 
    // fprintf(output, "Total time taken in mixed method: %lld microseconds\n", (mixed_time) / 1000);
    cout << "Total time taken in mixed method: " << mixed_time << " seconds" << endl;
}