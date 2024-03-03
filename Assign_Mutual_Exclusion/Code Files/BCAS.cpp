#include<bits/stdc++.h>
#include<pthread.h>
#include<sched.h>
#include<sys/time.h>
#include<atomic>
using namespace std;

int n, k;   //size of matrix(n) and number of threads(k)
int rowInc; //number of rows computed by each thread in one go (Similarly to chunk size)
vector<vector<int>> A; //matrix A
vector<vector<int>> prod; //product matrix A*A 
int C; // Shared counter to keep track of the number of threads that have completed their work
atomic<bool> lock_(false); // BCAS lock initialized to false
vector<bool> waiting; // vector to keep track of threads waiting in Bounded CAS

typedef struct ComputeArgs {    //struct for thread arguments
    int thread_id;
} ComputeArgs;

// Thread function to compute the certain rows (rowInc) of the square matrix 
void* Compute_BCAS(void* arg) {
    ComputeArgs* args = (ComputeArgs*)arg;
    int thread_id = args->thread_id;

    while (true) {

        if (C > n) break; // All rows of product matrix have been computed

        waiting[thread_id]=true; // Thread is waiting
        while(waiting[thread_id]) { //spin untill thread is not waiting
            bool expected = false;
            if(lock_.compare_exchange_strong(expected, true)) {
                break; //lock acquired, break out from this loop and enter critical section
            }
        }
        waiting[thread_id]=false; // Current thread can enter critical section, it is not waiting any more

        /* Critical Section */ 

        //Getting the start index and manually incrementing the shared counter C
        int start = C; //returns the previous value of C
        C+=rowInc; //Increment the counter
        int end = min(start + rowInc, n);

        cout << C << " " << start << " " << end << " Thread: " << thread_id << "\n";
        
        // CRITICAL SECTION ENDS
        
        //cyclically search for next waiting thread
        int j = (thread_id + 1) % k;
        while(j != thread_id && !waiting[j]) {
            j = (j + 1) % k;            
        }

        if(j == thread_id) {
            lock_.store(false); //No waiting thread found, release the lock 
        }
        else {
            waiting[j]=false; //Found a waiting thread, set it to false so it can entere critical section now
        }

        /* Remainder Section */
        
        for (int i = start; i < end; i++) {
            //computing row i of product matrix
            for (int j = 0; j < n; j++) {
                for (int k = 0; k < n; k++) {
                    prod[i][j] += A[i][k] * A[k][j];
                }
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

//Bounded Compare and swap method to execute mutual exclusion
double BCAS(vector<pthread_t>& threads, FILE* output) {

    fprintf(output, "Mutual Exclusion using BCAS:\n");

    struct timespec start_time, end_time;
    clock_gettime(CLOCK_MONOTONIC, &start_time);

    //creating k threads
    for (int i = 0; i < k; i++) {
        ComputeArgs* args = (ComputeArgs*)malloc(sizeof(ComputeArgs));
        args->thread_id = i;
        pthread_create(&threads[i], NULL, Compute_BCAS, (void*)args);
    }

    //joining all threads
    for (int i = 0; i < k; i++) {
        pthread_join(threads[i], NULL);
    }

    clock_gettime(CLOCK_MONOTONIC, &end_time);

    double BCAS_time = (end_time.tv_sec - start_time.tv_sec) + (1e-9) * (end_time.tv_nsec - start_time.tv_nsec);
    
    fprintf(output, "\nTotal time taken using BCAS: %f seconds\n", (BCAS_time));
    cout << "Total time taken using BCAS: " << BCAS_time << " seconds" << endl;
    
    // // printing the product matrix
    fprintf(output, "Product matrix(BCAS):\n");
    print_matrix(prod, output);

    return BCAS_time;
}

int main() {

    //reading input file
    FILE* input = fopen("../Input Files/input.txt", "r");
    if (input == NULL) {
        cout << "Input file not found" << endl;
        return 1;
    }
    fscanf(input, "%d %d %d", &n, &k, &rowInc);

    A.resize(n, vector<int>(n, 0));
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            if(fscanf(input, "%d", &A[i][j])!=1) {
                cout << "Invalid input" << endl;
                return 1;
            }
        }
    }
    fclose(input);

    //Resizing the waiting vector and setting all elements to false
    waiting.resize(k,false);

    FILE* output = fopen("../Output Files/output_Bcas.txt", "w");
    if (output == NULL) {
        cout << "Output file not found" << endl;
        return 1;
    }

    fprintf(output, "Size of matrix: %d \nNumber of threads: %d \nRow increment: %d \n", n, k, rowInc);
    //resizing matrix according to sizes

    prod.resize(n, vector<int>(n, 0));

    //creating k threads
    vector<pthread_t> threads(k);
    
    //BCAS Method
    int num_rep = 1;
    double BCAS_time=0;
    for(int i = 0; i < num_rep; i++) {
        BCAS_time += BCAS(threads, output);
        C=0;
    }
    BCAS_time/=num_rep;
    cout << "Average BCAS time: " << BCAS_time << "\n";

    fclose(output);
}