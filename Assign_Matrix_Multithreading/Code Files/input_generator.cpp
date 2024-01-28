//generates input.txt file with matrix a of size n by n of random numbers

#include<bits/stdc++.h>
using namespace std;

#define n 10// size of matrix
#define k 4 // number of threads

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

    fprintf(out, "%d %d\n", n, k);

    //generate matrix of size n by n with random numbers
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            fprintf(out, "%d ", input_arr[rand()% 100]);
        }
        fprintf(out, "\n");
    }
}