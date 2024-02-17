//generates input.txt file with matrix a of size n by n of random numbers

#include<bits/stdc++.h>
using namespace std;

#define n 1024// size of matrix
#define k 32 // number of threads
#define c 12 // number of logical cores on my pc
#define bt 8 // number of bounded threads

int main() {
     
    //for sparse matrix
    vector<int> input_arr(100,0);
    for(int i = 0; i < 25; i++) {
        input_arr[rand()%100] = rand()%100;
    }

    FILE* out = fopen("../Input Files/input.txt", "w");
    if (out == NULL) {
        cout << "Output file not found" << endl;
        return 1;
    }

    fprintf(out, "%d %d %d %d\n", n, k, c, bt);

    //generate matrix of size n by n with random numbers
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            fprintf(out, "%d ", input_arr[rand()% 100]);
        }
        fprintf(out, "\n");
    }
}