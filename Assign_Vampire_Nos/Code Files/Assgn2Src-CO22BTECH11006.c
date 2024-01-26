#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>
#include<limits.h>
// #include<time.h>
#include<sys/time.h>
#include<stdbool.h>
#include<math.h>
#include<string.h>

bool *checked;      // Array to keep track of numbers that have been checked
long long count=0;  // Number of Vampire numbers found
long *execution_times; // Array to keep track of execution times of each thread


// Recursive function to go through all permutations to find if it is vampire number
//permut will store the permutations of str
void search(char *str, int i, bool *isVampire, char *permut, int n, int *chosen, long long num) {
    
    if (*isVampire) return; // Stop recursing if isVampire is true

    if (i == n) {
        permut[n] = '\0'; // Ensure null termination

        // Splitting permuation into 2 parts
        //eg permut=[1,2,3,4,5,6]
        //temp1=[1,2,3] and temp2=[4,5,6]
       
        char temp1[n/2 + 1], temp2[n/2 + 1];
        strncpy(temp1, permut, n/2);
        temp1[n/2] = '\0';  // Ensure null termination
        strncpy(temp2, permut + n/2, n/2);
        temp2[n/2] = '\0';  // Ensure null termination
        
        if (temp1[0] == '0' || temp2[0] == '0') return; //conditions for vampire number
        if (temp1[n/2 - 1] == '0' && temp2[n/2 - 1] == '0') return; //conditions for vampire number

        //converting temp1 and temp2 into long long
        long long a = atoll(temp1);
        long long b = atoll(temp2);

        //checking if a*b=num and num is vampire or not
        *isVampire = (a * b == num);
        if(*isVampire) {
            printf("%lld = %lld * %lld\n", num, a, b);
        }
        return;
    } else {
        // Generate all permutations
        for (int j = 0; j < n; j++) {
            if (chosen[j]) continue;
            chosen[j] = 1;
            permut[i] = str[j]; // Assign character at position i
            search(str, i + 1, isVampire, permut, n, chosen, num);
            chosen[j] = 0;
        }
    }
}

// Function to check if a number is a Vampire number
bool Is_Vampire_Num(long long num) {
    
    // Counting number of digits
    int digit_count = 0;
    long long temp = num;
    while (temp) {
        digit_count++;
        temp /= 10;
    }

    if (!num || digit_count % 2) return false; //condition for vampire number
    if (digit_count == 2) return false; //condition for vampire number

    char *str = (char *)malloc(sizeof(char) * (digit_count + 1)); // +1 for null terminator
    char *permut = (char *)malloc(sizeof(char) * (digit_count + 1)); // +1 for null terminator
    int *chosen = (int *)malloc(sizeof(int) * digit_count);
    memset(chosen, 0, sizeof(int) * digit_count); // Initialize chosen array
    sprintf(str, "%lld", num);  // Convert num to string

    bool isVampire = false; // Initialize isVampire to false

    search(str, 0, &isVampire, permut, digit_count, chosen, num);

    free(str);
    free(permut);
    free(chosen);

    return isVampire;
}

//Struct for arguments in thread function
typedef struct ComputeArgs {
    FILE *fp;      // File pointer to print the vampire number to the output file
    long long i;   // Index of the thread 
    long long n;   // Range of numbers to be checked
    long long k;   // Number of threads or step size for each thread
} ComputeArgs;

//Function for each thread
void *Compute(void *arg) {
    ComputeArgs *args = (ComputeArgs *)arg; // Cast arg to ComputeArgs
    long long num = args->i; // Initial number to be checked which will be incremented

    struct timeval start, end;
    gettimeofday(&start,NULL);

    //increment the number and check if it is a Vampire number till n
    while (num <= args->n) {
        
        if (checked[num]) { // Check if the number has already been checked
            num += args->k;
            continue;
        }

        // Check if the number is a Vampire number and print out if it is
        checked[num] = true;
        if (Is_Vampire_Num(num)) {
            printf("%lld: Found by thread %lld\n", num, args->i + 1);
            fprintf(args->fp, "%lld: Found by thread %lld\n", num, args->i + 1);
            count++;
        }

        num += args->k; // Increment the number by step size
    
    }
    
    // Calculate the execution time of the thread
    gettimeofday(&end,NULL);
    long exec_time = (end.tv_sec - start.tv_sec) * 1000 + (end.tv_usec - start.tv_usec) / 1000;
    execution_times[args->i] = exec_time;

    free(args);
    pthread_exit(NULL); // Exit the thread
}


int main() {
    
    long long n,k;
    //scan range(n) and number of threads(k) for input file
    FILE *input = fopen("../Input Files/input.txt", "r");
    if (input == NULL) {
        printf("Input file not found.\n");
        return 1;
    }
    fscanf(input, "%lld %lld", &n, &k);
    fclose(input);

    FILE *output = fopen("../Output Files/output.txt", "w");
    if (output == NULL) {
        printf("Output file not created.\n");
        return 1;
    }
    fprintf(output, "Range of numbers to be checked: 1 to %lld\n", n);
    fprintf(output, "Number of threads: %lld\n\n", k);

    checked = (bool *)calloc(n + 1, sizeof(bool));  // Initialize checked array
    execution_times = malloc(k * sizeof(long)); // Initialize execution_times array

    struct timeval start, end; // Initialize start and end times
    gettimeofday(&start,NULL); // Get the current time

    pthread_t *threads = malloc(k * sizeof(pthread_t)); // Initialize threads array
    
    // Create k threads and call Compute function for each thread with intitial num
    for (long long i = 0; i < k; i++) {
        
        //Arguments for each thread in the form of struct
        ComputeArgs *args = malloc(sizeof(*args));
        args->i = i;
        args->n = n;
        args->k = k;
        args->fp = output;

        pthread_create(&threads[i], NULL, Compute, args); // Create a new thread

    }

    // Wait for all threads to finish
    for (long long i = 0; i < k; i++) {
        pthread_join(threads[i], NULL);
    }

    gettimeofday(&end,NULL);   //Get the current time
    // Calculate the total execution time by subtracting the start time from the end time
    long time_milli = (end.tv_sec - start.tv_sec) * 1000 + (end.tv_usec - start.tv_usec) / 1000;

    printf("\nVampire numbers found: %lld\n", count);
    printf("Multithreaded Execution Time: %ld milliseconds\n", time_milli);

    fprintf(output, "\nVampire numbers found: %lld\n", count);
    fprintf(output, "Multithreaded Execution Time: %ld milliseconds\n\n", time_milli);

    //printing execution time of each thread
    for (long long i = 0; i < k; i++) {
        fprintf(output, "Thread %lld execution time: %ld milliseconds\n", i + 1, execution_times[i]);
        printf("Thread %lld execution time: %ld milliseconds\n", i + 1, execution_times[i]);
    }

    fclose(output);
    free(checked);
    free(threads);

    return 0;
}



